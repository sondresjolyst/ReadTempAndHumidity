// Copyright (c) 2023-2024 Sondre Sjølyst

#ifndef SRC_EEPROMHELPER_H_
#define SRC_EEPROMHELPER_H_

#include <EEPROM.h>

extern const int EEPROM_SSID_START;
extern const int EEPROM_SSID_END;
extern const int EEPROM_PASSWORD_START;
extern const int EEPROM_PASSWORD_END;

void writeEEPROM(unsigned int start, int end, String data) {
  unsigned int size = end - start;
  if (data.length() > size) {
    data = data.substring(0, size);
  }

  for (unsigned int i = 0; i < size; i++) {
    if (i < data.length()) {
      EEPROM.write(start + i, data[i]);
    } else {
      EEPROM.write(start + i, 0);
    }
  }

  EEPROM.commit();
}

String readEEPROM(int start, int end) {
  String res = "";
  for (int i = start; i < end; i++) {
    res += static_cast<char>(EEPROM.read(i));
  }
  return res;
}

void clearWifiCredentials() {
  for (int i = EEPROM_SSID_START; i < EEPROM_PASSWORD_END; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  Serial.println("Restarting ESP...");
  ESP.restart();
}

#endif  // SRC_EEPROMHELPER_H_
