# Bathroom door occupancy sensor

The goal is a small, low-cost sensor for single occupancy bathrooms that already have physical occupied/vacant indicators. Residents could open one publicly hosted web app from their dorm rooms, on campus Wi-Fi or any other network, to check availability. The ESP32 would send status outward to the web app's hosted backend; residents would never connect to the ESP32 directly. This distance-sensing setup is a prototype, and the final enclosure and mounting method may change. The first intended pilot is one University of Illinois residence hall floor. See [VISION.md](VISION.md) for the staged plan, research, and competitors.

## Built and verified

- `bathroom_ble/bathroom_ble.ino` reads a VL53L0X distance sensor and advertises the reading over Bluetooth Low Energy (BLE).
- `mac_ble_monitor.swift` and `run_mac_ble_monitor.sh` receive and print that BLE reading on a Mac.
- `serial_distance/serial_distance.ino` is a separate USB serial diagnostic sketch.
- On 2026-09-23, the BLE sketch was flashed to an ESP32-D0WD-V3 with 4 MB flash. USB serial and the Mac BLE receiver both showed live distance readings. See [learnings.md](learnings.md) and the CSVs in `tests/`.
- `wifi_reporter/` is the 15-second Wi-Fi/HTTPS sketch now flashed to the USB-connected ESP32. The university device portal shows its MAC as Active. Repeated live distance reports reached the Vercel API with HTTP 200, and [bathroom-sensor.vercel.app](https://bathroom-sensor.vercel.app) displayed the latest reading from Upstash. See [learnings.md](learnings.md) for the test data.

The intended measurement is from the side of the door toward the next visible surface. When the lock is engaged, its hardware should block the sensor at a short distance; when unlocked, that hardware moves out of the way and the sensor should see the far wall. The large distance change is the proposed occupied/vacant signal. The sensor-to-webpage connection is verified, but lock-state thresholds, repeated locked/unlocked trials, an installed door test, battery runtime, and a campus pilot remain unverified.

## Prototype wiring

For the proposed two-AA supply and low-power approach, see [POWER.md](POWER.md). Battery operation has not been tested yet.

These are GPIO labels, not physical header positions.

| VL53L0X breakout | ESP32 board |
| --- | --- |
| SDA | GPIO32 |
| SCL | GPIO33 |
| GND | GND |
| VCC | Board 5V pin |

Confirm the exact breakout accepts 5 V and keeps SDA/SCL at ESP32-safe logic levels before powering it. The [Adafruit](https://learn.adafruit.com/adafruit-vl53l0x-micro-lidar-distance-sensor-breakout/pinouts) and [Pololu](https://www.pololu.com/product/2490) carriers accept 5 V; the [bare sensor](https://www.st.com/resource/en/datasheet/vl53l0x.pdf) does not. Confirm the development board's 5 V power path before connecting external power and USB together; [Espressif's DevKitC guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html) treats the two as mutually exclusive.

## Build and inspect

Install the `esp32` board package and `Adafruit_VL53L0X` Arduino library. Select **ESP32 Dev Module** for this prototype board and compile `bathroom_ble/bathroom_ble.ino`:

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 bathroom_ble
```

The sketch prints readings at 115200 baud. To read its BLE advertisements on this Mac, run `sh run_mac_ble_monitor.sh` and allow Terminal Bluetooth access if prompted. The BLE receiver scans advertisements; it does not pair with the ESP32.

## Internet connection MVP

The [web setup guide](web/README.md) describes the new connection test. Its page shows whether the ESP32 has reported recently and the latest distance. It does **not** label a bathroom occupied or available yet; lock-state calibration is still needed. The ESP32 sends one authenticated HTTPS update every 15 seconds. A 60-second gap displays it as disconnected; the backend discards the last reading after 90 seconds.

The Pi's university setup used SecureW2 to install an EAP-TLS profile on Linux. That script cannot simply run on an ESP32. University [device registration guidance](https://answers.illinois.edu/illinois/90275) instead describes registering a device's Wi-Fi MAC for `campus guest Wi-Fi`. The connected ESP32's MAC is `[redacted MAC]`; its registration and outbound HTTPS reporting are now verified. Do not copy Pi certificates or account credentials into the sketch.

BLE service-data UUID: `e63f0c82-7f89-4c1e-a4dd-9d87c45b1401`. Each four-byte payload is `version, status, distance-low, distance-high`. Version is `1`; status `0` means valid, `1` invalid/out of range, and `2` sensor unavailable. Distance is unsigned little-endian millimetres. A nonzero status uses `0xFFFF` for distance. The sketch samples about every 100 ms and refreshes advertised data every 500 ms. Treat missing advertisements as **unknown**, not vacant. The broadcast is unencrypted and presently contains a raw distance, so a later deployment should send only the minimum needed state.
