#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "display/DisplayOLED.h"

class MQTTManager {
private:
  WiFiClientSecure wifiClient;
  PubSubClient client;
  const char* server;
  int port;
  String clientId;
  const char* username;
  const char* password;
  DisplayOLED* display;
  int ledPin;
  unsigned long lastBlinkTime;
  bool ledState;

  void generateClientId();

public:
  MQTTManager(const char* server, int port = 8883, const char* username = nullptr, const char* password = nullptr, DisplayOLED* display = nullptr, int ledPin = LED_BUILTIN);
  void begin();
  bool connect();
  bool isConnected();
  bool publish(const char* topic, const char* payload);
  bool subscribe(const char* topic);
  void loop();
  String getClientId();
};

#endif