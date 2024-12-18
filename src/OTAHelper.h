// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_OTAHELPER_H_
#define SRC_OTAHELPER_H_

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

void onStart() { Serial.println("Start"); }

void onEnd() { Serial.println("\nEnd"); }

void onProgress(unsigned int progress, unsigned int total) {
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
}

void onError(ota_error_t error) {
  Serial.printf("Error [%u]: ", error);
  if (error == OTA_AUTH_ERROR)
    Serial.println("Auth Failed");
  else if (error == OTA_BEGIN_ERROR)
    Serial.println("Begin Failed");
  else if (error == OTA_CONNECT_ERROR)
    Serial.println("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR)
    Serial.println("Receive Failed");
  else if (error == OTA_END_ERROR)
    Serial.println("End Failed");
}

class OTAHelper {
 public:
  OTAHelper() {}

  void setup() {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Error: No WiFi connection when setting up OTAHelper");
      return;
    }

    ArduinoOTA.onStart(onStart);
    ArduinoOTA.onEnd(onEnd);
    ArduinoOTA.onProgress(onProgress);
    ArduinoOTA.onError(onError);

    ArduinoOTA.begin();
    Serial.println("OTA is ready");
  }

  void loop() { ArduinoOTA.handle(); }
};

#endif  // SRC_OTAHELPER_H_
