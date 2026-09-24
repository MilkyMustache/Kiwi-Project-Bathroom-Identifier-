# Two-AA power plan for the current prototype

This plan uses the ESP32-D0WD-V3 development board already running the Wi-Fi reporter and the VL53L0X V2 breakout. The chip family is known, but the development board's exact model and the breakout's manufacturer are not recorded. Check the labels or schematics of both boards before applying battery power.

## Wiring

```text
2 × AA alkaline cells in series
  → on/off switch
  → 3.3 V step-up regulator, sized for Wi-Fi current bursts
  → regulated 3.3 V rail ─┬─→ ESP32 board 3V3
                          └─→ VL53L0X V2 breakout VCC/VIN*

Battery −, regulator GND, ESP32 GND, and sensor GND are common.
Sensor SDA → ESP32 GPIO32; sensor SCL → ESP32 GPIO33.
```

*Power the breakout from 3.3 V only after confirming its VCC/VIN input accepts 3.3 V and its I²C pins are safe for 3.3 V ESP32 GPIOs. Common Adafruit and Pololu VL53L0X carriers meet this requirement, but “V2” alone does not identify the carrier circuit. The bare VL53L0X chip has different supply and I/O requirements. Until the actual board is identified, keep the present USB-powered sensor wiring.*

The AA pair is not a 3.3 V supply: two alkalines are about 3 V nominal and their voltage falls in use. The regulator must hold 3.3 V during ESP32 Wi-Fi transmit peaks. Espressif recommends a 3.3 V supply capable of at least 500 mA for the ESP32; allow additional margin for the sensor (up to 40 mA while ranging), development board, and startup. Select a regulator that can **actually deliver at least 600 mA at a 2.4 V input**, has low no-load current, and starts at a voltage the AA pack can provide. The usable battery cutoff must be established by measurement; a regulator's input-current rating is not its 3.3 V output-current rating. [Espressif supply guidance](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/schematic-checklist.html); [ST sensor current](https://www.st.com/resource/en/datasheet/vl53l0x.pdf).

A candidate module for a bench trial is [Pololu U1V11F3](https://www.pololu.com/product/2561), a 3.3 V boost regulator that starts below the voltage of a two-AA pack. Its available output current falls as the batteries discharge; its 1.2 A input-current limit is **not** a promise of 1.2 A at 3.3 V output. Confirm the regulator's 3.3 V output stays steady during association and HTTPS posts before selecting it for an unattended installation. Add at least 10 µF across 3V3 and GND near the ESP32 supply entrance, following the regulator module's capacitor guidance. [Espressif decoupling guidance](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/schematic-checklist.html).

The board's 5V/VIN pin will **not** become 5 V when supplying its 3V3 pin. The existing 9 V-to-5 V converter is a step-down supply and cannot run from two AAs. Supplying the 3V3 pin bypasses the development board's normal input regulator, so use only a regulated supply, confirm that this board supports power through that pin, and disconnect USB while the battery supply is connected. The exact board power path still needs identification.

## Operating mode

The current Wi-Fi sketch sends an HTTPS reading about every 15 seconds and already calls `WiFi.setSleep(true)`. That enables Wi-Fi modem sleep while retaining the access-point connection. For lower idle current without a reconnect on each report, the next firmware option is **automatic light sleep combined with Wi-Fi modem sleep**. Simply calling manual light sleep for 15 seconds does not preserve the Wi-Fi connection. No external wake timer or sensor power switch is needed for the first battery trial. [Espressif Wi-Fi power modes](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/low-power-mode/low-power-mode-wifi.html); [ESP32 sleep behavior](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/sleep_modes.html).

## Before a door installation

1. Identify the exact ESP32 development board and VL53L0X V2 carrier from their markings or schematics; confirm the 3V3 input and sensor VCC/VIN ranges.
2. With USB unplugged, measure the regulator output at the ESP32 during boot, Wi-Fi association, and several HTTPS reports. Any drop that causes resets means the regulator or cells need changing.
3. Measure battery-side average current over many 15-second cycles and record the pack voltage. Use those measurements to estimate replacement interval; chip deep-sleep figures do not include the development board or regulator.
4. Keep the backend's stale-reading behavior: it displays disconnected after a 60-second gap and discards a report after 90 seconds.

This is a proposed wiring plan, not a battery-powered result. No battery runtime, brownout margin, or automatic light-sleep behavior has been measured on this board yet.
