// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_WIZHELPER_H_
#define SRC_WIZHELPER_H_

#include <ArduinoJson.h>
#include <WiFiUdp.h>

#include <vector>

#include "MQTTHelper.h"
#include "PRINTHelper.h"

// extern WiFiClient serverClient;
extern PRINTHelper printHelper;

// const int WIZDISCOVER_READ_DELAY = 5000;
// const int WIZSTATE_READ_DELAY = 5000;
// const int WIZDEVICE_READ_DELAY = 100;
unsigned int localUdpPort = 38899;

WiFiUDP Udp;

// #define MAX_DEVICES 50
// std::vector<std::pair<String, String>> discoveredDevices;
// std::pair<String, String> deviceArray[MAX_DEVICES];
// int deviceCount = 0;
// int deviceIndex = 0;

// void wizDiscover() {
//   static unsigned long lastToggleTime = 0;

//   if (millis() - lastToggleTime >= WIZDISCOVER_READ_DELAY) {
//     lastToggleTime = millis();

//     Udp.beginPacketMulticast(IPAddress(255, 255, 255, 255), localUdpPort,
//     WiFi.localIP()); Udp.write("{\"method\":\"getDevInfo\"}");
//     Udp.endPacket();

//     int packetSize = Udp.parsePacket();
//     if (packetSize) {
//       char incomingPacket[255];
//       int len = Udp.read(incomingPacket, 255);

//       if (len > 0) {
//         incomingPacket[len] = 0;
//       }

//       String deviceIP = Udp.remoteIP().toString();
//       DynamicJsonDocument doc(1024);
//       deserializeJson(doc, incomingPacket);
//       String deviceMac = doc["result"]["devMac"];

//       bool deviceDiscovered = false;
//       for (const auto& device : discoveredDevices) {
//         if (device.first == deviceIP) {
//           deviceDiscovered = true;
//           break;
//         }
//       }

//       if (!deviceDiscovered) {
//         discoveredDevices.push_back(std::make_pair(deviceIP, deviceMac));
//         printHelper.printf("Discovered new device at IP: %s\n",
//         deviceIP.c_str()); String deviceName = "wiz_light_" + deviceMac;
//         sendMQTTWizDiscoveryMsg(deviceIP, deviceName);

//         String stateTopic = "home/storage/" + deviceName + "/state";
//         printHelper.println(stateTopic);
//         client.subscribe(stateTopic.c_str());
//       }
//     }
//   }
// }

// void wizGetState() {
//   static unsigned long lastToggleTime = 0;

//   // If the array is empty, add all devices to the array
//   if (deviceCount == 0) {
//     for (const auto& device : discoveredDevices) {
//       if (deviceCount < MAX_DEVICES) {
//         deviceArray[deviceCount++] = device;
//       } else {
//         // Handle the situation where there are more devices than the array
//         can hold
//         // For example, you could ignore the extra devices or replace an
//         existing device
//       }
//     }
//   }

//   if (millis() - lastToggleTime >= WIZSTATE_READ_DELAY && deviceCount > 0) {
//     lastToggleTime = millis();

//     // Get the next device from the array
//     const auto& device = deviceArray[deviceIndex++];
//     if (deviceIndex >= deviceCount) {
//       deviceIndex = 0;  // Wrap around to the start of the array
//     }

//     String deviceName = "wiz_light_" + device.second;

//     String command = "{\"method\":\"getPilot\"}";
//     Udp.beginPacket(device.first.c_str(), 38899);
//     Udp.write(command.c_str());
//     Udp.endPacket();

//     printHelper.println("Sent " + command + " to: " + device.first);

//     int packetSize = Udp.parsePacket();
//     if (packetSize) {
//       char incomingPacket[255];
//       int len = Udp.read(incomingPacket, 255);
//       if (len > 0) {
//         incomingPacket[len] = 0;
//       }

//       printHelper.println("Received response from: " +
//       Udp.remoteIP().toString());

//       DynamicJsonDocument doc(1024);
//       deserializeJson(doc, incomingPacket);

//       String result = doc["result"];
//       String state = doc["result"]["state"];
//       bool lightState = doc["result"]["state"];

//       printHelper.println("result: " + result);
//       printHelper.println("state: " + state);
//       printHelper.println("");

//       publishWizState(deviceName, lightState);
//     }
//   }
// }

void wizSetup() {
  Udp.begin(localUdpPort);
  printHelper.printf("Now listening at IP %s, UDP port %d\n",
                     WiFi.localIP().toString().c_str(), localUdpPort);
}

#endif  // SRC_WIZHELPER_H_
