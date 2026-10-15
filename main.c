/* BeaconPulse reference implementation (ESP-IDF v5.5, NimBLE).
 * Fallback/reference only: FirmGen generates the submitted firmware.
 * Not compiled by the author; verify on your toolchain. */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#define TAG "BEACON"
#define LED_GPIO 2
#define BTN_GPIO 0
#define REFRESH_MS 10000

static char ble_name[16];
static uint8_t own_addr_type;
static volatile uint8_t press_count;
static volatile bool ble_synced;

/* FR-4, FR-5: (re)start non-connectable advertising with live payload */
static void adv_start(void)
{
    uint16_t up = (uint16_t)((esp_timer_get_time() / 1000000ULL) & 0xFFFF);
    uint8_t mfg[6] = {0xFF, 0xFF, 0x01, (uint8_t)(up >> 8), (uint8_t)up, press_count};

    struct ble_hs_adv_fields f;
    memset(&f, 0, sizeof f);
    f.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    f.name = (uint8_t *)ble_name;
    f.name_len = strlen(ble_name);
    f.name_is_complete = 1;
    f.mfg_data = mfg;
    f.mfg_data_len = sizeof mfg;

    int rc = ble_gap_adv_set_fields(&f);
    if (rc) { ESP_LOGE(TAG, "set_fields rc=%d", rc); return; }

    struct ble_gap_adv_params p;
    memset(&p, 0, sizeof p);
    p.conn_mode = BLE_GAP_CONN_MODE_NON;
    p.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &p, NULL, NULL);
    if (rc) { ESP_LOGE(TAG, "adv_start rc=%d", rc); return; }
    ESP_LOGI(TAG, "Payload refresh: uptime=%us presses=%u", up, press_count);
}

static void on_sync(void)
{
    ble_hs_id_infer_auto(0, &own_addr_type);
    ble_synced = true;
    adv_start();
    ESP_LOGI(TAG, "BLE advertising started as %s", ble_name); /* UR-3 */
}

static void host_task(void *arg)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/* FR-6: refresh every 10 s; FR-9: separate task, never blocks LED/button */
static void refresh_task(void *arg)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(REFRESH_MS));
        if (!ble_synced) continue;
        ble_gap_adv_stop();
        adv_start();
    }
}

/* FR-8: 1 Hz heartbeat */
static void led_task(void *arg)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    for (;;) {
        gpio_set_level(LED_GPIO, 1); vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 0); vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* FR-7: BOOT button, falling edge, 50 ms debounce */
static void button_task(void *arg)
{
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_GPIO, GPIO_PULLUP_ONLY);
    int last = 1;
    for (;;) {
        int now = gpio_get_level(BTN_GPIO);
        if (last == 1 && now == 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (gpio_get_level(BTN_GPIO) == 0) {
                press_count++;
                ESP_LOGI(TAG, "Button press #%u", press_count); /* UR-5 */
                while (gpio_get_level(BTN_GPIO) == 0) vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    /* FR-1 */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* FR-2, FR-3 */
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    ESP_LOGI(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); /* UR-1 */
    snprintf(ble_name, sizeof ble_name, "FirmGen_%02X", mac[5]);
    ESP_LOGI(TAG, "BLE name: %s", ble_name); /* UR-2 */

    xTaskCreate(led_task, "led", 2048, NULL, 3, NULL);
    xTaskCreate(button_task, "btn", 2048, NULL, 4, NULL);

    nimble_port_init();
    ble_hs_cfg.sync_cb = on_sync;
    ble_svc_gap_device_name_set(ble_name);
    nimble_port_freertos_init(host_task);
    xTaskCreate(refresh_task, "refresh", 3072, NULL, 3, NULL);
}
