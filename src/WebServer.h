// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_WEBSERVER_H_
#define SRC_WEBSERVER_H_

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


extern ESP8266WebServer server;
extern WiFiClient serverClient;
extern PubSubClient client;

const char *html = "<html>...</html>";

String getWifiOptions() {
  int n = WiFi.scanNetworks();
  String options = "";

  String ssids[n];

  for (int i = 0; i < n; i++) {
    ssids[i] = WiFi.SSID(i);
  }

  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (ssids[j] < ssids[i]) {
        String temp = ssids[i];
        ssids[i] = ssids[j];
        ssids[j] = temp;
      }
    }
  }

  for (int i = 0; i < n; i++) {
    options += "<option value='" + ssids[i] + "'>" + ssids[i] + "</option>";
  }

  return options;
}

void handleRoot() {
  String options = getWifiOptions();
  String html = "<html>\
<head>\
  <title>ESP8266 Config</title>\
  <style>\
    body {\
      font-family: Arial, sans-serif;\
      margin: 0;\
      padding: 0;\
      background-color: #f0f0f0;\
    }\
    .container {\
      max-width: 600px;\
      margin: 0 auto;\
      padding: 20px;\
    }\
    h1 {\
      color: #333;\
      font-size: 2em; /* Increase the size of the title */\
    }\
    label {\
      display: block;\
      margin-bottom: 10px;\
      font-size: 1.5em; /* Increase the size of the labels */\
    }\
    select, input[type='password'] {\
      width: 100%;\
      padding: 10px;\
      margin-bottom: 20px;\
      border-radius: 5px;\
      border: 1px solid #ccc;\
      font-size: 1.5em; /* Increase the size of the input fields */\
    }\
    input[type='submit'] {\
      padding: 10px 20px;\
      border: none;\
      border-radius: 5px;\
      background-color: #007BFF;\
      color: white;\
      cursor: pointer;\
      font-size: 1.5em; /* Increase the size of the button */\
    }\
    input[type='submit']:hover {\
      background-color: #0056b3;\
    }\
  </style>\
</head>\
<body>\
  <div class='container'>\
    <h1>ESP8266 Configuration</h1>\
    <form action=\"/submit\" method=\"POST\">\
      <label for=\"ssid\">SSID:</label><br>\
      <select id=\"ssid\" name=\"ssid\">" +
                options + "</select><br>\
      <label for=\"password\">Password:</label><br>\
      <input type=\"password\" id=\"password\" name=\"password\"><br>\
      <input type=\"submit\" value=\"Connect!\">\
    </form>\
  </div>\
</body>\
</html>";
  server.send(200, "text/html", html);
}

void webpage_status() {
  String html = "<html>\
  <body>\
    <h1>ESP8266 Web Server</h1>\
    <p>MQTT Connectivity: ";
  html += (client.connected()) ? "Connected" : "Disconnected";
  html += "</p>\
  </body>\
  </html>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    writeEEPROM(EEPROM_SSID_START, EEPROM_SSID_END, ssid);
    writeEEPROM(EEPROM_PASSWORD_START, EEPROM_PASSWORD_END, password);

    server.send(200, "text/plain", "Data received. Restarting...");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing SSID or password");
  }
}

#endif  // SRC_WEBSERVER_H_