// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_VOLTMETERCONTROLLER_H_
#define SRC_VOLTMETERCONTROLLER_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "PRINTHelper.h"

extern PubSubClient client;
extern String MQTT_STATETOPIC;
extern PRINTHelper printHelper;

const int ANALOG_IN_PIN = A0;
const int ANALOG_RESOLUTION = 1024;     // 10-bit resolution
const float ANALOG_VOLTAGE = 3.32;      // Reference voltage for ESP8266 ADC
const float R1 = 10000.0;               // 10kΩ
const float R2 = 4700.0;                // 4.7kΩ
const float CORRECTION_FACTOR = 1.218;  // multimeter voltage / voltageMeasured

// Corrected constants for exponential correction
// Constants a and b are calculated based on curve fitting using known points:
// Point 1: (Measured Voltage: 5.25, Actual Voltage: 5.09)
// Point 2: (Measured Voltage: 10.38, Actual Voltage: 12.65)
// Steps to calculate a and b:
// 1. Convert the known points to natural logarithms.
// 2. Solve the system of linear equations to find b.
// 3. Use b to solve for a.
//
// Using points (5.25, 5.09) and (10.38, 12.65):
// ln(5.09) = ln(a) + b * ln(5.25)
// ln(12.65) = ln(a) + b * ln(10.38)
//
// Solve for b:
// b = (ln(12.65) - ln(5.09)) / (ln(10.38) - ln(5.25))
// b ≈ 1.33555
//
// Solve for a:
// a = exp(ln(5.09) - b * ln(5.25))
// a ≈ 0.555788
const float a = 0.555788;
const float b = 1.33555;

const int READ_VOLTAGE_DELAY = 60000;
const int READING_VOLTAGE_BUFFER = 5;
float averageVoltage = 0;
float voltageReadings[READING_VOLTAGE_BUFFER];
float totalVoltage = 0;
int readVoltageIndex = 0;

float currentVoltageReadings = 0;
int failedVoltageReadings = 0;

float readVoltage() {
  int sensorValue = analogRead(ANALOG_IN_PIN);
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  // Calculate the measured voltage at the divider output
  float voltageMeasured = (ANALOG_VOLTAGE / ANALOG_RESOLUTION) * sensorValue;
  Serial.print("Measured Voltage: ");
  Serial.print(voltageMeasured);
  Serial.println(" V");

  printHelper.print("Measured Voltage: ");
  printHelper.print(String(voltageMeasured));
  printHelper.println(" V");

  float vinTest = voltageMeasured * ((R1 + R2) / R2);
  Serial.print("Input Voltage: ");
  Serial.print(vinTest);
  Serial.println(" V");

  printHelper.print("Input Voltage: ");
  printHelper.print(String(vinTest));
  printHelper.println(" V");

  // calculated correction
  float vinTestCorrected =
      voltageMeasured * ((R1 + R2) / R2 * CORRECTION_FACTOR);
  Serial.print("Input Voltage Corrected: ");
  Serial.print(vinTestCorrected);
  Serial.println(" V");

  printHelper.print("Input Voltage Corrected: ");
  printHelper.print(String(vinTestCorrected));
  printHelper.println(" V");

  // exponential correction
  float vinTestCorrectedExponential = a * pow(vinTest, b);
  Serial.print("Input Voltage exponential Corrected: ");
  Serial.print(vinTestCorrectedExponential);
  Serial.println(" V");

  printHelper.print("Input Voltage exponential Corrected: ");
  printHelper.print(String(vinTestCorrectedExponential));
  printHelper.println(" V");

  return vinTestCorrectedExponential;
}

void voltageSensorSetup() {
  pinMode(ANALOG_IN_PIN, INPUT);

  for (int i = 0; i < READING_VOLTAGE_BUFFER; i++) {
    voltageReadings[i] = readVoltage();
    totalVoltage += voltageReadings[i];
  }
}

void voltageCheckAndRestartIfFailed(float *reading, int32_t *failedReadings) {
  printHelper.println("Checking if reading failed");
  if (reading == nullptr || std::isnan(*reading)) {
    printHelper.printf("Reading: %s", String(*reading));
    failedReadings += 1;
    printHelper.printf("Failed count: %s", String(*failedReadings));
    if (*failedReadings >= 10) {
      ESP.restart();
    }
  } else {
    printHelper.println("Reading OK");
    printHelper.printf("Reading: %s", String(*reading));
    failedReadings = 0;
  }
}

void readAndWriteVoltageSensor() {
  static uint32_t lastToggleTime = 0;

  if (millis() - lastToggleTime >= READ_VOLTAGE_DELAY) {
    int arrayLength = sizeof(voltageReadings) / sizeof(voltageReadings[0]);

    lastToggleTime = millis();

    totalVoltage -= voltageReadings[readVoltageIndex];
    voltageReadings[readVoltageIndex] = readVoltage();
    totalVoltage += voltageReadings[readVoltageIndex];

    voltageCheckAndRestartIfFailed(&totalVoltage, &failedVoltageReadings);

    readVoltageIndex = (readVoltageIndex + 1) % arrayLength;
    averageVoltage = totalVoltage / arrayLength;

    DynamicJsonDocument doc(1024);
    char buffer[256];

    doc["voltage"] = averageVoltage;

    size_t n = serializeJson(doc, buffer);

    bool published = client.publish(MQTT_STATETOPIC.c_str(), buffer, n);

    Serial.print("Published: ");
    Serial.println(published);
    // Debugging
    printHelper.print("Published: ");
    printHelper.println(String(published));

    Serial.print("Battery Voltage: ");
    Serial.print(averageVoltage);
    Serial.println(" V");
    // Debugging
    printHelper.print("Battery Voltage: ");
    printHelper.println(String(averageVoltage));
  }
}

#endif  // SRC_VOLTMETERCONTROLLER_H_
