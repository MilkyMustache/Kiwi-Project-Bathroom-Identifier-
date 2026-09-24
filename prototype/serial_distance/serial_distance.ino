#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

constexpr uint8_t kSdaPin = 32;
constexpr uint8_t kSclPin = 33;
constexpr uint8_t kSensorAddress = 0x29;
constexpr uint32_t kReadIntervalMs = 100;

Adafruit_VL53L0X sensor;
uint32_t lastReadAt = 0;

void setup() {
  Serial.begin(115200);
  delay(500);  // Give the USB serial monitor time to open after reset.
  Serial.println("Starting VL53L0X distance monitor");

  Wire.begin(kSdaPin, kSclPin);
  Wire.setClock(400000);

  if (!sensor.begin(kSensorAddress, false, &Wire,
                    Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE)) {
    Serial.println("VL53L0X not found at I2C address 0x29; check power and wiring");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Sensor ready. Distance is in millimetres.");
  lastReadAt = millis() - kReadIntervalMs;
}

void loop() {
  const uint32_t now = millis();
  if (now - lastReadAt < kReadIntervalMs) {
    return;
  }
  lastReadAt = now;

  VL53L0X_RangingMeasurementData_t measurement;
  const VL53L0X_Error result = sensor.rangingTest(&measurement, false);

  if (result != VL53L0X_ERROR_NONE) {
    Serial.printf("Sensor read error: %d\n", static_cast<int>(result));
  } else if (measurement.RangeStatus != 0) {
    Serial.printf("Invalid/out-of-range reading: status %u\n",
                  measurement.RangeStatus);
  } else {
    Serial.printf("Distance: %u mm\n", measurement.RangeMilliMeter);
  }
}
