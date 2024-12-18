// Copyright (c) 2023-2024 Sondre Sjølyst

#include <Arduino.h>
#include <DHT.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include <cstdint>
#include <cstring>
#include <regex>
#include <string>

#include "./arduino_secrets.h"
#include "./liz.h"
#include "EEPROMHelper.h"
#include "MQTTHelper.h"
#include "OTAHelper.h"
#include "PRINTHelper.h"
#include "SensorController.h"
#include "VoltmeterController.h"
#include "WIZHelper.h"
#include "WebServer.h"
#include "WiFiHelper.h"

uint32_t chipId = ESP.getChipId();
String CHIP_ID_STRING = String(chipId, HEX);
String MQTT_HOSTNAME_STRING = "Wemos_D1_Mini_" + CHIP_ID_STRING;
const char *MQTT_HOSTNAME = MQTT_HOSTNAME_STRING.c_str();
String MQTT_STATETOPIC = "home/storage/" + String(MQTT_HOSTNAME) + "/state";
String WIFI_NAME = "Liz Sensor " + String(MQTT_HOSTNAME);
const uint8_t DHTTYPE = DHT11;
const char *MQTT_BROKER = SECRET_MQTTBROKER;
const char *MQTT_PASS = SECRET_MQTTPASS;
const char *MQTT_USER = SECRET_MQTTUSER;
const char *SENSOR_TYPE = "BME";  // "BME" or "DHT"
const char *LIZ_TYPE = "sensor";  // "voltmeter" or "sensor"
const float TEMP_HUMID_DIFF = 10.0;
const int DHT_SENSOR_PIN = 2;
const int DNS_PORT = 53;
const int EEPROM_PASSWORD_END = 96;
const int EEPROM_PASSWORD_START = 32;
const int EEPROM_SSID_END = 31;
const int EEPROM_SSID_START = 0;
const int LED_BLINK_COUNT = 10;
const int LED_BLINK_DELAY = 1000;
const int MQTT_PORT = SECRET_MQTTPORT;
const int RESET_BUTTON_GPO = 4;
const int RESET_PRESS_DURATION = 5000;
const int SERIAL_PORT = 9600;
const int WEBSITE_PORT = 80;
const int WIFI_DELAY = 1000;
const int WIFI_TRIES = 15;
// const char *ipaddress = "192.168.1.203";
const int port = 38899;
const int runGetPilot_BLINK_DELAY = 2000;

Adafruit_BME280 bme;
DHT dht(DHT_SENSOR_PIN, DHTTYPE, 11);
ESP8266WebServer server(WEBSITE_PORT);
ResetWiFi resetWiFi(RESET_BUTTON_GPO, RESET_PRESS_DURATION);
OTAHelper *otaHelper = nullptr;
PRINTHelper printHelper(serverClient);

void setup() {
  Serial.begin(SERIAL_PORT);
  delay(100);

  Serial.println("Disconnecting WiFi");
  WiFi.disconnect();

  EEPROM.begin(512);
  delay(10);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Starting...");
  Serial.println("Reading EEPROM...");

  String EEPROM_SSID = readEEPROM(EEPROM_SSID_START, EEPROM_SSID_END);
  String EEPROM_PASSWORD =
      readEEPROM(EEPROM_PASSWORD_START, EEPROM_PASSWORD_END);

  if (EEPROM_SSID.length() == 0) {
    Serial.println("SSID not found in EEPROM. Starting AP...");
    setupAP();
    return;
  }

  Serial.print("Attempting to connect to SSID: ");
  Serial.print(EEPROM_SSID);
  Serial.print(" with PASSWORD: ");
  Serial.print(EEPROM_PASSWORD);
  if (connectWifi(EEPROM_SSID, EEPROM_PASSWORD)) {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    connectToMQTT();
    if (strcmp(LIZ_TYPE, "sensor") == 0) {
      environmentalSensorSetup(SENSOR_TYPE);
    } else if (strcmp(LIZ_TYPE, "voltmeter") == 0) {
      voltageSensorSetup();
    } else {
      Serial.println("Please set LIZ_TYPE");
    }

    server.on("/", webpage_status);
    server.begin();

    otaHelper = new OTAHelper();
    otaHelper->setup();

    wizSetup();
  } else {
    setupAP();
  }

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();
}

void blinkLED(int count) {
  static uint32_t lastToggleTime = 0;
  static int blinkCount = 0;
  static bool ledState = LOW;

  if (millis() - lastToggleTime >= LED_BLINK_DELAY) {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);

    lastToggleTime = millis();

    if (ledState == LOW) {
      blinkCount++;
    }
    if (blinkCount >= count) {
      blinkCount = 0;
    }
  }
}

void discoverAndSubscribe() {
  // Get the current discoveredDevices
  auto oldDiscoveredDevices = liz::getDiscoveredDevices();

  // Discover devices
  auto discoveredDevices = liz::discover(port, 60000);

  // Only subscribe if a new device has been discovered
  for (const auto &device : discoveredDevices) {
    // Check if the device is not in oldDiscoveredDevices
    if (std::find(oldDiscoveredDevices.begin(), oldDiscoveredDevices.end(),
                  device) == oldDiscoveredDevices.end()) {
      std::string deviceIP = std::get<0>(device);
      std::string deviceMac = std::get<1>(device);
      std::string moduleName = std::get<2>(device);

      // Filter out "SOCKET" or "SHRGBC"
      std::smatch match;
      if (std::regex_search(moduleName, match, std::regex("(SOCKET|SHRGBC)"))) {
        moduleName = match.str();
      }

      std::string deviceName = "wiz_" + moduleName + "_" + deviceMac;
      sendMQTTWizDiscoveryMsg(deviceIP, deviceName);

      std::string stateTopic = "home/storage/" + deviceName + "/set";
      client.subscribe(stateTopic.c_str());
      // runGetPilot(deviceIP);
    }
  }
}

void loop() {
  if ((WiFi.status() != WL_CONNECTED)) {
    server.handleClient();
    return;
  }

  if ((mqttStatus() == false)) {
    Serial.print("Connecting to MQTT");
    connectToMQTT();
  }

  if (otaHelper != nullptr) {
    otaHelper->loop();
  }

  handleTelnet();
  server.handleClient();
  resetWiFi.update();
  blinkLED(LED_BLINK_COUNT);
  client.loop();
  if (strcmp(LIZ_TYPE, "sensor") == 0) {
    readAndWriteEnvironmentalSensors(SENSOR_TYPE);
  } else if (strcmp(LIZ_TYPE, "voltmeter") == 0) {
    readAndWriteVoltageSensor();
  }
  // runGetPilot();
  discoverAndSubscribe();
}
