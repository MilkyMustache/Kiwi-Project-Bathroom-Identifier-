# Connection test web app

This is a plain HTML page with two Vercel Functions: `POST /api/report` receives the ESP32 reading and `GET /api/status` gives the page the latest reading. Upstash Redis stores one value with a 90-second expiry. The page polls every 15 seconds and marks updates older than 60 seconds as disconnected. No occupancy threshold or visit history is included.

The page pauses polling while hidden. At one report every 15 seconds, the device alone uses about 172,800 Redis commands in a 30-day month. [Upstash's free plan currently allows 500,000 commands per month](https://upstash.com/pricing/redis), so check usage before adding devices or frequent viewers; automatic paid upgrades should remain disabled for this test.

## Local checks

Run `npm test` in this folder. It checks token rejection, input validation, a fresh reading, and a stale reading. The test mocks Redis; it does not prove a live deployment.

## Current deployment and reflash

The private [GitHub repository](https://github.com/jweivy/bathroom-sensor) deploys this `web/` directory to [bathroom-sensor.vercel.app](https://bathroom-sensor.vercel.app). An Upstash Redis resource is connected on the free plan and supplies `KV_REST_API_URL` and `KV_REST_API_TOKEN` as server-side environment variables. The API also accepts `UPSTASH_REDIS_REST_URL` and `UPSTASH_REDIS_REST_TOKEN` if configured directly. `DEVICE_TOKEN` is a separate sensitive Production variable. Do not put any of these values in browser code or commit them.

ESP32 Wi-Fi MAC `[redacted MAC]` is registered in the university's [Device Management portal](https://go.illinois.edu/campus Wi-FiDevices) and uses `campus guest Wi-Fi`. The current USB-powered board has posted real sensor data to `https://bathroom-sensor.vercel.app/api/report`; the public page displayed it. For another board or a reflash, copy `../wifi_reporter/config.example.h` to `../wifi_reporter/config.h`, fill in the URL and token, then compile and flash `wifi_reporter/` at 115200 baud. The config file is ignored by Git. The sketch prints Wi-Fi, clock, sensor, and HTTP results at 115200 baud.

The sketch verifies the current `*.vercel.app` Google Trust Services certificate chain. It tries [Illinois's `ntp.illinois.edu`](https://answers.illinois.edu/illinois/page.php?id=47806) first, then public NTP servers. If NTP is unavailable, it uses the firmware build time as an approximate clock while still checking the server certificate and hostname. This fallback may fail after the server certificate rotates; rebuild or provide reliable time for longer deployments. If the deployment uses another hostname or Vercel changes its issuing chain, update `wifi_reporter/vercel_ca.h` with the appropriate public CA certificate before flashing. Do not disable TLS verification.

This first sketch keeps Wi-Fi active and reports every 15 seconds. Battery runtime has not been measured; expect to optimize sleep and reporting later. Mounting on a university bathroom door is a separate approval and reliability step.
