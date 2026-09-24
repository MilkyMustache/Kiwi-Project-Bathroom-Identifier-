# Learnings

## 2026-09-22 — USB identification check

The connected board appeared at `[redacted serial port]`. Reading its serial output at 115200 baud for about ten seconds produced no text. A read-only bootloader query identified an ESP32-D0WD-V3 revision 3.1 with 4 MB of flash. This confirms the target family and flash size, but does not verify the sensor or BLE behavior. The serial-only sketch compiled successfully. No firmware was uploaded.

Data: [tests/2026-09-22-usb-check.csv](tests/2026-09-22-usb-check.csv). CSV column legend: `date_local` is the local test date; `check` names the observation; `result` records its outcome; `detail` gives the measured value or limitation.

## 2026-09-23 — BLE firmware flash and live measurement

Flashed `bathroom_ble.ino` over `[redacted serial port]` at 115200 baud to the ESP32-D0WD-V3 with 4 MB flash. The upload verified each written image by hash. At 115200 baud, the serial monitor reported live VL53L0X distances around 122–138 mm. The Mac CoreBluetooth listener then received the advertised distance wirelessly, including readings around 119–138 mm. This verifies sensor initialization, the current SDA/SCL wiring, BLE broadcasting, and Mac decoding in the present setup. These are observations at the current sensor position, not calibration values. The test listener was stopped after verification.

Data: [tests/2026-09-23-flash-ble.csv](tests/2026-09-23-flash-ble.csv). CSV column legend: `date_local` is the local test date; `check` names the step; `result` records its outcome; `detail` gives the observed value or evidence.

## 2026-09-23 — Wi-Fi connection preparation

A read-only bootloader query on `[redacted serial port]` reported the ESP32 Wi-Fi MAC `[redacted MAC]`. No firmware was uploaded. The new Wi-Fi reporter sketch compiled at 919,444 bytes (70% of the board's application partition); this does not prove that it can join the campus network or post to Vercel. The Raspberry Pi is connected to `campus Wi-Fi` using EAP-TLS after a SecureW2 Linux setup. The university documents MAC registration for devices that cannot run SecureW2, followed by `campus guest Wi-Fi`; that path is pending for this ESP32.

Data: [tests/2026-09-23-wifi-mac.csv](tests/2026-09-23-wifi-mac.csv). CSV column legend: `date_local` is the local test date; `check` names the observation; `result` records its outcome; `detail` gives the measured value or limitation.

## 2026-09-23 — First campus Wi-Fi firmware connection attempt

The first upload stopped after switching the USB serial link to 921600 baud, before the application image was written. Retrying at 115200 baud succeeded, and the bootloader, partition, and application image hashes verified. The sketch reached its clock synchronization step, which only follows `WiFi.status() == WL_CONNECTED`, so association to `campus guest Wi-Fi` is inferred. The initial public NTP servers never returned valid time during repeated 10-second waits; HTTPS was deliberately not attempted without a clock. This does not establish Internet access or an end-to-end sensor update. Illinois provides [ntp.illinois.edu](https://answers.illinois.edu/illinois/page.php?id=47806), so the next firmware build uses that campus server first. Device MAC registration was still pending during this attempt. A later review of the [campus port policy](https://answers.illinois.edu/illinois/47646) found that it permits outbound NTP; the earlier claim that it blocks outbound NTP was incorrect.

Data: [tests/2026-09-23-wifi-first-connection.csv](tests/2026-09-23-wifi-first-connection.csv). CSV column legend: `date_local` is the local test date; `check` names the step; `result` records the outcome or inference level; `detail` gives the observed value or limitation.

## 2026-09-23 — Campus time-server connection attempt

The reporter with `ntp.illinois.edu` first in its time-server list was flashed at 115200 baud; its application hash verified. Serial output confirmed `WiFi.status() == 3` (`WL_CONNECTED`) and private address `[redacted private IP].223` on `campus guest Wi-Fi`. Repeated time-sync attempts still failed before device MAC registration, and the sketch did not attempt HTTPS. The guest network association is working, but this test does not show Internet access. Registration through the university's documented portal is the next gate; do not infer that the campus NTP service itself is unavailable to registered devices.

Data: [tests/2026-09-23-wifi-campus-ntp.csv](tests/2026-09-23-wifi-campus-ntp.csv). CSV column legend: `date_local` is the local test date; `check` names the step; `result` records the outcome; `detail` gives the observed value or limitation.

## 2026-09-23 — Registered device connection check

The university device portal showed MAC `[redacted MAC]` as Active with IP `[redacted private IP].223` and an expiration date of 2027-09-23. The ESP32 still reported `WL_CONNECTED` at that IP, but repeated clock synchronization attempts failed, including after a USB serial reset pulse. The firmware therefore did not attempt HTTPS or measure the sensor for reporting. The portal's active state confirms registration; it does not prove that the device has outbound Internet access. Illinois's [device guidance](https://answers.illinois.edu/90286) says a newly registered device may need to disconnect or power off for a few minutes before the network change takes hold.

Data: [tests/2026-09-23-registered-connection.csv](tests/2026-09-23-registered-connection.csv). CSV column legend: `date_local` is the local test date; `check` names the observation; `result` records the outcome; `detail` gives the observed value or limitation.

## 2026-09-23 — Live ESP32 to Vercel connection

The clock-fallback reporter compiled at 1,081,752 bytes (82% of the application partition). The first upload stopped while switching to 921600 baud, before writing the application image. Retrying at 115200 baud wrote the application and verified its hash. The USB serial monitor then showed valid VL53L0X readings of 37, 43, 39, 35, and 39 mm, each with HTTP 200, at roughly 15-second intervals. The public `/api/status` endpoint returned an online sensor with the same 37 mm reading, and Chrome displayed “Sensor connected” with a later 35 mm reading. This verifies the live sensor → ESP32 → Wi-Fi → Vercel API → Upstash → webpage path in the current USB-powered setup. The startup clock path was not captured, so this test does not establish whether NTP or the firmware build-time fallback supplied the clock. Battery runtime and occupied/unoccupied lock calibration remain untested; the current distance values are not calibration values.

Data: [tests/2026-09-23-live-web-connection.csv](tests/2026-09-23-live-web-connection.csv). CSV column legend: `date_local` is the local test date; `time_local` is the observation time in America/Chicago; `check` names the observation; `result` records the outcome; `detail` gives the value or evidence.
