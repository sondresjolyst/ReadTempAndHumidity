// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_PRINTHELPER_H_
#define SRC_PRINTHELPER_H_

#include <Arduino.h>
#include <WiFiClient.h>

#include <cstdio>

extern WiFiClient serverClient;

class PRINTHelper {
 public:
  explicit PRINTHelper(WiFiClient &client) : _client(client) {}

  void print(const String &message) {
    Serial.print(message);
    _client.print(message);
  }

  void println(const String &message) {
    Serial.println(message);
    _client.println(message);
  }

  void printf(const char *format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    Serial.println(buf);
    _client.println(buf);
  }

 private:
  WiFiClient &_client;
};

#endif  // SRC_PRINTHELPER_H_
