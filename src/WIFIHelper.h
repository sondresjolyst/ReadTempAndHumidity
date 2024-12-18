// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_WIFIHELPER_H_
#define SRC_WIFIHELPER_H_

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "EEPROMHelper.h"

extern String WIFI_NAME;
extern const int DNS_PORT;
extern const int EEPROM_PASSWORD_END;
extern const int EEPROM_PASSWORD_START;
extern const int EEPROM_SSID_END;
extern const int EEPROM_SSID_START;
extern const int WIFI_DELAY;
extern const int WIFI_TRIES;

const size_t kBufferSize = 256;

DNSServer dnsServer;
WiFiServer telnetServer(23);
WiFiClient serverClient;

bool connectWifi(String ssid, String password) {
  WiFi.begin(ssid.c_str(), password.c_str());
  int tries = 0;
  while (tries < WIFI_TRIES) {
    if (WiFi.status() == WL_CONNECTED) {
      telnetServer.begin();
      telnetServer.setNoDelay(true);
      return true;
    }
    Serial.print(".");
    delay(WIFI_DELAY);
    tries++;
  }
  Serial.println(" could not connect to wifi!");
  return false;
}

void handleNotFound() {
  server.sendHeader("Location", "http://192.168.4.1", true);
  server.send(302, "text/plain", "");
}

void setupAP() {
  Serial.println("Starting AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NAME);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Access Point is up and running!");
}

void handleTelnet() {
  if (telnetServer.hasClient()) {
    if (!serverClient || !serverClient.connected()) {
      if (serverClient) serverClient.stop();
      serverClient = telnetServer.accept();
    }
  }

  while (serverClient.available()) {
    Serial.write(serverClient.read());
  }

  if (Serial.available()) {
    size_t len = Serial.available();
    len = (len > kBufferSize) ? kBufferSize : len;
    uint8_t sbuf[kBufferSize];
    Serial.readBytes(sbuf, len);
    serverClient.write(sbuf, len);
    serverClient.flush();
    delay(1);
  }
}

class ResetWiFi {
 public:
  ResetWiFi(int pin, uint32_t duration)
      : buttonPin(pin), buttonPressTime(0), pressDuration(duration) {
    pinMode(buttonPin, INPUT_PULLUP);
  }

  void update() {
    int buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH) {
      buttonPressTime = 0;
      return;
    }

    if (buttonPressTime == 0) {
      Serial.print("Pressed!");
      buttonPressTime = millis();
      return;
    }
    Serial.println("");
    Serial.println(millis() - buttonPressTime);
    if ((millis() - buttonPressTime) <= pressDuration) {
      return;
    }

    clearWifiCredentials();
    buttonPressTime = 0;
  }

 private:
  int buttonPin;
  int32_t buttonPressTime;
  int32_t pressDuration;
};

#endif  // SRC_WIFIHELPER_H_
