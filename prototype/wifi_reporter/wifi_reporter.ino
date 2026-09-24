#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_VL53L0X.h>
#include <time.h>
#include <sys/time.h>
#include "vercel_ca.h"

#if __has_include("config.h")
#include "config.h"
#else
#include "config.example.h"
#endif

constexpr uint8_t kSdaPin = 32;
constexpr uint8_t kSclPin = 33;
constexpr uint8_t kSensorAddress = 0x29;
constexpr uint32_t kReportIntervalMs = 15000;

Adafruit_VL53L0X sensor;
bool sensorReady = false;
uint32_t lastReportAt = 0;

bool connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.printf("Connecting to %s...\n", kWifiSsid);
  WiFi.begin(kWifiSsid, strlen(kWifiPassword) ? kWifiPassword : nullptr);
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 12000) {
    delay(250);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Wi-Fi unavailable (status %d)\n", WiFi.status());
    return false;
  }
  Serial.printf("Wi-Fi connected; IP %s\n", WiFi.localIP().toString().c_str());
  return true;
}

bool clockReady() {
  if (time(nullptr) > 1700000000) return true;
  // Try the campus server first; guest-network NTP has not worked in this test.
  configTime(0, 0, "ntp.illinois.edu", "time.google.com", "pool.ntp.org");
  const uint32_t started = millis();
  while (time(nullptr) <= 1700000000 && millis() - started < 10000) {
    delay(250);
  }
  if (time(nullptr) <= 1700000000) {
    // A recent firmware build gives TLS an approximate clock when UDP NTP fails.
    // CA and hostname checks still run; rebuild if the server certificate rotates.
    char monthName[4] = {};
    int day = 0, year = 0, hour = 0, minute = 0, second = 0;
    if (sscanf(__DATE__, "%3s %d %d", monthName, &day, &year) != 3 ||
        sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second) != 3) {
      Serial.println("Cannot parse firmware build time; HTTPS paused");
      return false;
    }
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int month = 0;
    while (month < 12 && strcmp(monthName, months[month]) != 0) month++;
    if (month == 12) {
      Serial.println("Cannot parse firmware build month; HTTPS paused");
      return false;
    }
    tm built = {};
    built.tm_year = year - 1900;
    built.tm_mon = month;
    built.tm_mday = day;
    built.tm_hour = hour;
    built.tm_min = minute;
    built.tm_sec = second;
    setenv("TZ", "UTC0", 1);
    tzset();
    const time_t approximate = mktime(&built);
    if (approximate <= 1700000000) return false;
    timeval now = {};
    now.tv_sec = approximate;
    settimeofday(&now, nullptr);
    Serial.println("NTP unavailable; using firmware build time for verified HTTPS");
  }
  return true;
}

void reportReading() {
  if (!connectWifi()) return;
  if (!strlen(kReportUrl) || !strlen(kDeviceToken)) {
    Serial.println("Set kReportUrl and kDeviceToken in config.h before reporting");
    return;
  }
  if (!clockReady()) return;

  bool valid = false;
  uint16_t distanceMm = 0;
  if (sensorReady) {
    VL53L0X_RangingMeasurementData_t measurement;
    const VL53L0X_Error result = sensor.rangingTest(&measurement, false);
    valid = result == VL53L0X_ERROR_NONE && measurement.RangeStatus == 0;
    if (valid) distanceMm = measurement.RangeMilliMeter;
  }

  String body = String("{\"sensorOk\":") + (valid ? "true" : "false");
  if (valid) body += String(",\"distanceMm\":") + distanceMm;
  body += "}";

  WiFiClientSecure client;
  client.setCACert(kVercelCaPem);
  client.setTimeout(5000);
  HTTPClient http;
  http.setConnectTimeout(5000);
  if (!http.begin(client, kReportUrl)) {
    Serial.println("Invalid HTTPS report URL");
    return;
  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-device-token", kDeviceToken);
  const int code = http.POST(body);
  Serial.printf("Sensor %s", valid ? "OK" : "unavailable");
  if (valid) Serial.printf(" (%u mm)", distanceMm);
  Serial.printf("; report HTTP %d\n", code);
  if (code < 0) Serial.println(http.errorToString(code));
  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(true);
  Serial.printf("ESP32 Wi-Fi MAC: %s\n", WiFi.macAddress().c_str());

  Wire.begin(kSdaPin, kSclPin);
  Wire.setClock(400000);
  sensorReady = sensor.begin(kSensorAddress, false, &Wire,
                             Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
  Serial.println(sensorReady ? "VL53L0X ready" : "VL53L0X unavailable");
  lastReportAt = millis() - kReportIntervalMs;
}

void loop() {
  const uint32_t now = millis();
  if (now - lastReportAt >= kReportIntervalMs) {
    lastReportAt = now;
    reportReading();
  }
  delay(50);
}
