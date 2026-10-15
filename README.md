# BeaconPulse - FirmGen Track 2 Submission Package

Project: ESP32 BLE live-status beacon, built PRD-first with CraftifAI FirmGen v0.4.1.

## Why this project
It extends the lab's BLE name beacon with a live payload (uptime, button count) and a heartbeat LED, so the demo shows real physical behavior (button, LED, scanner) while staying small enough for Auto Mode to finish. No extra wiring.

## Runbook (follow the lab steps)
1. Create project: platform ESP32, verify your board, toolchain ESP-IDF v5.5, coding guideline BARR-C (optional).
2. Draft PRD > I have a document > upload BeaconPulse_PRD.md > Generate requirements.
3. Review and freeze. Explicit must include FR-1 to FR-9, UR-1 to UR-5, 115200 baud. Reject any Inferred item about connections, pairing, GATT, Wi-Fi, OTA or sleep. Keep inferred items that only add startup/error handling.
4. Task lists, then Criteria. Note which criteria are host-testable (name format, payload packing) vs on-target (scanner, LED, button).
5. Start implementation in Auto Mode. Do not press Build/Flash/Monitor yourself. Stop Monitor if flashing fails.
6. Set up Git while it runs (remote repo you own, short branch name).
7. Validate on board (below). Then Artifacts: Unit Tests and Design Doc.
8. Open the traceability report in sources/, export/keep it, commit, push.

If the build fails, prompt: "Fix only the first root cause build error, preserve the current architecture, and redeploy."

## Acceptance evidence checklist (capture each)
| Criterion | Evidence to capture |
|---|---|
| AC-1 boots + advertises | Monitor screenshot: MAC, name, "advertising started", no reboots |
| AC-2 name matches MAC | Same screenshot: MAC last byte equals name suffix |
| AC-3 visible 2 min | Phone scanner screenshot/video, device staying listed |
| AC-4 button counter | Video: press BOOT 3 times, UART lines, scanner payload shows 03 within 10 s |
| AC-5 uptime + LED | Two scanner screenshots 10+ s apart with increasing uptime; video of LED blinking |

Scanner tip: use nRF Connect (shows raw manufacturer data). Payload FFFF 01 UUUU PP: UUUU = uptime seconds, PP = presses. If the scanner address differs from UART MAC, check you are comparing the Bluetooth MAC (the firmware reads ESP_MAC_BT).

## Traceability (what the FirmGen report should show)
Each of FR-1..FR-9 and UR-1..UR-5 maps to criteria, firmware files/symbols and a test. Expected host tests: name formatting, payload packing/endianness, counter wrap, debounce logic. On-target only: scanner visibility, LED, real button. Remember: traceability complete does not mean on-board evidence is complete.

## Files
- BeaconPulse_PRD.md: upload this to FirmGen.
- firmware/: hand-written reference implementation (ESP-IDF v5.5, NimBLE), for comparison or emergency fallback. Submit the FirmGen-generated firmware.

## 2-minute demo script
1. PRD and frozen requirements (10 s, show Explicit/Inferred and a rejected item).
2. Req. tab with criteria and Auto Mode progress (15 s).
3. Board on camera: LED blinking, UART log, phone scanner shows FirmGen_XX (40 s).
4. Press BOOT 3 times, show counter 03 in scanner (25 s).
5. Traceability report: follow FR-3 to code and test; Git push (30 s).

## Submission checklist
- [ ] PRD uploaded (not skip-specification)  - [ ] requirements frozen
- [ ] real ESP32 flashed  - [ ] UART/Monitor + physical evidence
- [ ] traceability report  - [ ] Git branch pushed
