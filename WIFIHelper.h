#ifndef WIFIHELPER_H
#define WIFIHELPER_H

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "EEPROMHelper.h"

extern const char* WIFI_NAME;
extern const int DNS_PORT;
extern const int EEPROM_PASSWORD_END;
extern const int EEPROM_PASSWORD_START;
extern const int EEPROM_SSID_END;
extern const int EEPROM_SSID_START;
extern const int WIFI_DELAY;
extern const int WIFI_TRIES;

DNSServer dnsServer;

bool connectWifi(String ssid, String password) {
  WiFi.begin(ssid.c_str(), password.c_str());
  int tries = 0;
  while (tries < WIFI_TRIES) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi");
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

class ResetWiFi {
public:
  ResetWiFi(int pin, unsigned long duration)
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
    buttonPressTime = 0;  // Reset the timer
  }

private:
  int buttonPin;
  unsigned long buttonPressTime;
  unsigned long pressDuration;
};

#endif
