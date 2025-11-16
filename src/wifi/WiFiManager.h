#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "display/DisplayOLED.h"

class WiFiManager {
private:
  const char* ssid;
  const char* password;
  int ledPin;
  DisplayOLED* display;
  
public:
  WiFiManager(const char* ssid, const char* password, int ledPin = LED_BUILTIN, DisplayOLED* display = nullptr);
  void begin();
  bool isConnected();
  String getIP();
  void reconnect();
};

#endif
