#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>

// ESP32-WROOM-32E wiring. These are GPIO numbers, not board header positions.
constexpr uint8_t kSdaPin = 32;
constexpr uint8_t kSclPin = 33;
constexpr uint8_t kSensorAddress = 0x29;

constexpr uint32_t kSampleIntervalMs = 100;
constexpr uint32_t kPayloadRefreshMs = 500;
constexpr uint16_t kInvalidDistanceMm = 0xFFFF;

// Private UUID for this project's BLE service-data format.
const BLEUUID kDistanceServiceUuid("e63f0c82-7f89-4c1e-a4dd-9d87c45b1401");

// Service data is four bytes: version, status, distance low byte, distance high byte.
// Status: 0 = valid, 1 = invalid/out of range, 2 = sensor unavailable.
struct Reading {
  uint8_t status = 2;
  uint16_t distanceMm = kInvalidDistanceMm;
};

Adafruit_VL53L0X sensor;
BLEAdvertising* advertising = nullptr;
Reading latest;
bool sensorReady = false;
uint32_t lastSampleAt = 0;
uint32_t lastPayloadRefreshAt = 0;

void publishReading(const Reading& reading) {
  const uint8_t payload[] = {
      1,
      reading.status,
      static_cast<uint8_t>(reading.distanceMm & 0xFF),
      static_cast<uint8_t>(reading.distanceMm >> 8),
  };

  BLEAdvertisementData data;
  data.setFlags(0x06);  // General discoverable; BLE only.
  data.setServiceData(kDistanceServiceUuid,
                      String(reinterpret_cast<const char*>(payload), sizeof(payload)));

  advertising->stop();
  if (!advertising->setAdvertisementData(data) || !advertising->start()) {
    Serial.println("BLE advertising update failed");
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(kSdaPin, kSclPin);
  Wire.setClock(400000);
  sensorReady = sensor.begin(kSensorAddress, false, &Wire,
                             Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
  if (!sensorReady) {
    Serial.println("VL53L0X not found at I2C address 0x29");
  }

  BLEDevice::init("Bathroom Sensor");
  advertising = BLEDevice::getAdvertising();
  advertising->setMinInterval(160);  // BLE units of 0.625 ms = 100 ms.
  advertising->setMaxInterval(160);

  BLEAdvertisementData scanResponse;
  scanResponse.setName("Bathroom Sensor");
  advertising->setScanResponseData(scanResponse);

  publishReading(latest);
  lastSampleAt = millis() - kSampleIntervalMs;
  lastPayloadRefreshAt = millis();
}

void loop() {
  const uint32_t now = millis();

  if (now - lastSampleAt >= kSampleIntervalMs) {
    lastSampleAt = now;
    latest = Reading{};

    if (sensorReady) {
      VL53L0X_RangingMeasurementData_t measurement;
      const VL53L0X_Error result = sensor.rangingTest(&measurement, false);
      if (result == VL53L0X_ERROR_NONE) {
        latest.status = measurement.RangeStatus == 0 ? 0 : 1;
        if (latest.status == 0) {
          latest.distanceMm = measurement.RangeMilliMeter;
        }
      }
    }

    if (latest.status == 0) {
      Serial.printf("Distance: %u mm\n", latest.distanceMm);
    } else {
      Serial.printf("Distance unavailable (status %u)\n", latest.status);
    }
  }

  if (now - lastPayloadRefreshAt >= kPayloadRefreshMs) {
    lastPayloadRefreshAt = now;
    publishReading(latest);
  }

  delay(5);
}
