# BeaconPulse - ESP32 BLE Live-Status Beacon (PRD v1.0)

## Objective
Firmware for an ESP32 DevKit that, after boot, advertises a BLE beacon whose name identifies the board and whose payload carries live status (uptime and button-press count), with a heartbeat LED and UART debug logs.

## Target
- Board: ESP32 DevKit (ESP32 classic), ESP-IDF v5.5, BLE (NimBLE host)
- Onboard parts only: BOOT button (GPIO0, active low), LED (GPIO2). No wiring.

## Scope
In scope: non-connectable BLE advertising, status payload, button counter, heartbeat LED, UART logs.
Out of scope (do not add): BLE connections, pairing, GATT services, Wi-Fi, OTA, deep sleep.

## Functional Requirements
1. FR-1: The device shall initialize NVS and the BLE stack automatically after boot.
2. FR-2: The device shall read the Bluetooth MAC address at runtime (esp_read_mac, ESP_MAC_BT).
3. FR-3: The device shall build the BLE name as `FirmGen_XX`, where XX is the last byte of the Bluetooth MAC in uppercase hex (e.g. MAC A4:CF:12:34:56:9B gives FirmGen_9B).
4. FR-4: The device shall advertise as non-connectable, general discoverable, with the complete name in the advertising data.
5. FR-5: The advertising data shall include manufacturer-specific data of exactly 6 bytes: company ID 0xFFFF (2 bytes, little-endian), version 0x01 (1 byte), uptime in seconds (2 bytes, big-endian, wraps at 65535), button-press count (1 byte, wraps at 255).
6. FR-6: The device shall refresh the manufacturer data every 10 seconds, and advertising shall be active again within 100 ms of each refresh.
7. FR-7: The device shall count BOOT button presses (GPIO0, falling edge) with 50 ms debounce.
8. FR-8: The device shall blink the LED (GPIO2) at 1 Hz (500 ms on, 500 ms off) as a heartbeat, independent of BLE activity.
9. FR-9: The device shall never block the heartbeat or button handling on BLE operations.

## UART Output (115200 baud)
- UR-1: Print the detected Bluetooth MAC address.
- UR-2: Print the generated BLE device name.
- UR-3: Print a message when BLE advertising starts.
- UR-4: Print one line per payload refresh with uptime and press count.
- UR-5: Print one line per debounced button press.

## Acceptance Criteria
- AC-1: The ESP32 boots without reboot loops and prints the advertising-started message.
- AC-2: The advertised name equals FirmGen_ plus the last byte of the MAC printed on UART.
- AC-3: A phone BLE scanner shows the device continuously for at least 2 minutes.
- AC-4: Pressing BOOT N times makes the press count in the scanner's manufacturer data equal N after the next refresh (within 10 s), and UART shows each press.
- AC-5: The uptime field in the scanner increases between refreshes, and the LED blinks at about 1 Hz throughout.
