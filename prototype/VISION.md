# Product vision and research

Research checked 2026-09-23. This is a plan, not a claim that the system has been deployed.

## Goal

A small sensor for a single occupancy bathroom reports **available**, **occupied**, or **unknown**. Residents check one publicly hosted web app from their dorm rooms before walking to the bathroom. The page should work on campus Wi-Fi, cellular data, or another network; it does not connect to the ESP32 directly. These bathrooms already have physical occupied/vacant indicators, so the product does not need another indicator light. Start with one University of Illinois residence hall floor, expand to other floors if residents find it useful, and eventually offer an installable product to universities and schools with similar bathrooms. Low cost, easy setup, and a minimal physical presence are proposed advantages to test, not proven advantages. This distance-sensing arrangement is a prototype; a compact door-mounted product is one possible later design. The product should never use a camera or microphone.

## Recommended first architecture

```text
Sensor → ESP32 → campus Wi-Fi → outbound HTTPS update → hosted backend
Resident's browser (any network) → public web app → hosted backend
```

The current ESP32 prototype broadcasts VL53L0X distance measurements over BLE, and the Mac receiver has decoded them. It aims from the side of the door at the lock hardware. With the lock engaged, the hardware should obstruct the beam at a short distance. With the lock disengaged, the hardware should move aside and expose the far wall at a much longer distance. That is the occupancy mechanism to validate; the current logs do not yet show repeated locked/unlocked classification.

The preferred path uses the ESP32 as an Internet **client**. It joins an approved Wi-Fi network and sends authenticated status updates by HTTPS to a publicly hosted backend. The backend stores each bathroom's latest state and update time; the web app reads that backend. The ESP32 does not host the public website, need a public IP address, or accept connections from residents. [Espressif documents ESP32 Wi-Fi client mode](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html) and [outbound HTTPS requests](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/api-reference/protocols/esp_http_client.html), so the hardware can do this. The Wi-Fi reporter is now flashed and has associated with `campus guest Wi-Fi`; the Vercel page and API are live. The status store still needs Upstash terms acceptance, and the ESP32 needs university device registration before its Internet path can be verified. Radio coverage, power use, and reliable reconnection remain untested.

The hosted backend would publish **unknown** when updates stop arriving, rather than leave a stale **available** state visible. This design needs Internet hosting for the backend and page but no Raspberry Pi server. If the sensor cannot use campus Wi-Fi reliably, a second ESP32 could receive its BLE broadcast and send updates to the same backend; [Espressif documents BLE-to-Wi-Fi bridging](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/bt/ble.html). Residents would still use the same public URL. A native mobile app can wait until the web app has been tested with residents.

## Stages and evidence needed

| Stage | Work | Move forward when… |
| --- | --- | --- |
| 1. Prove occupancy | Aim the prototype from the side of a test door across the lock path. Record distance with the lock engaged and disengaged over repeated cycles and normal door movement. Set a threshold only after seeing the two distance ranges and ambiguous cases. | The short lock-obstructed and long far-wall readings separate reliably; ambiguous or failed reads become **unknown**. |
| 2. Connect and display | Test ESP32 outbound HTTPS updates on an approved campus network. Build a public hosted backend and mobile-friendly web app that anyone can access from any network. Test a second ESP32 BLE-to-Wi-Fi relay only if the sensor cannot use campus Wi-Fi reliably. | Residents can open one URL on campus Wi-Fi or cellular data and see timely status; loss of updates becomes **unknown** rather than a false available reading. |
| 3. One-floor pilot | Present the device, mounting method, power plan, privacy behavior, and removal plan to University Housing Facilities and Residential Technology. After approval, install on a few bathrooms on one floor. | Residents use it, the status is trustworthy, and Housing accepts installation and upkeep. |
| 4. Other floors | Repeat with multiple doors and network coverage zones. Record failures, maintenance time, and resident feedback. | The same setup works without hand tuning each door or constant maintenance. |
| 5. Optimize | Improve battery life, enclosure, sensor choice, mounting, resilience, and operations based on pilot data. | Operating cost and upkeep are reasonable for a building operator. |
| 6. Custom PCB | Once the sensor and network design settle, design a compact board and enclosure. | A small batch passes the same field checks as the prototype. |
| 7. Commercial product | Package installation, support, security, privacy, and pricing for universities, other colleges, and schools with similar bathrooms. | A buyer validates the problem and a paid pilot or purchasing path exists. |

The immediate next experiment is **lock-state validation**. The VL53L0X is a [time-of-flight distance sensor](https://www.st.com/en/imaging-and-photonics-solutions/vl53l0x.html). Its distance difference should be measured across many lock cycles, door positions, and bathroom layouts before a threshold is chosen. The prototype reads distance to the lock hardware; it does not infer occupancy merely because a door is closed.

## Campus constraints to design around

- [Illinois Housing Hallmarks H-602](https://housing.illinois.edu/MyHousing/Hallmarks) prohibits personal devices that record audio or video and personal wireless routers in residence halls. A proposal with no imaging or audio, no added indicator, and a removable mount should make this explicit; Housing must determine whether the installation is permitted.
- [Illinois Technology Services says IoT devices can be registered for campus network access](https://answers.illinois.edu/illinois/90275), but residence hall cases go through Housing Technology. [Housing Facilities Operations](https://iservice.housing.illinois.edu/f_bot_main.html) is a practical first contact for mounting and maintenance questions.
- The existing Raspberry Pi uses an `campus Wi-Fi` EAP-TLS profile installed by SecureW2 on Linux. The ESP32 cannot run that setup script. For this first connection test, [Illinois's documented device route](https://answers.illinois.edu/90286) is to register the ESP32's Wi-Fi MAC, then join `campus guest Wi-Fi`. This registration and the actual ESP32 connection are pending.
- The public output should be room availability only. Avoid person identifiers and detailed visit history; show **unknown** when the sensor or network connection is stale. These are proposed privacy and reliability requirements, not features in the present firmware.

## Competitors and adjacent products

| Product | What its own site describes | Implication for this project |
| --- | --- | --- |
| [VERTECO LooLights](https://www.verteco.com/what-we-do/smart-washroom-technologies/loolights-wc-cubicle-occupancy-system) | Ceiling-mounted cubicle occupancy lights, outside-room displays, and desktop/mobile occupancy data for high-traffic washrooms. | The dorm pilot needs remote availability, not another light. Test whether using the existing door indicator with a small sensor is simpler and cheaper to install. |
| [MetaE Smart Restroom](https://www.virsical.com/en/metae-restroom.html) | Wireless door-lock sensors, stall status screens, and facility-management features. | It also reads lock state, but offers a broader building operations system. A minimal single-room retrofit and resident web view may be a different fit. |
| [TOTO CONNECT PUBLIC](https://jp.toto.com/en/support/use/iotp/ip_help/) | A phone-accessible service for installed facilities shows restroom occupancy information through a browser, without requiring a login. TOTO notes that displayed availability can differ from actual conditions. | A web availability view already exists. Status accuracy and a simple retrofit for single occupancy rooms are more useful pilot questions than whether an app can display a status. |
| [Waitz / Occuspace](https://waitz.io/faq.html) | Campus occupancy estimates from nearby Wi-Fi and Bluetooth signals, exposed to students in an app. | Adjacent university buyer and user model, but it estimates area crowding rather than a particular bathroom door's occupied state. |

This scan establishes competition, not product novelty or a defensible patent position. Before product claims or pricing, interview residents and Housing, compare install and support costs, and expand the competitor search by region and buyer type.
