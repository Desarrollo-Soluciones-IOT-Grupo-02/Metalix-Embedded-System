#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "display/DisplayOLED.h"

class MQTTManager {
private:
  WiFiClientSecure wifiClient;
  PubSubClient client;
  const char* server;
  int port;
  String clientId;
  String deviceId; 
  const char* username;
  const char* password;
  DisplayOLED* display;
  int ledPin;
  unsigned long lastBlinkTime;
  bool ledState;
  unsigned long lastReconnectAttempt;
  unsigned long lastStatusPublish;
  
  // Referencia al StateManager para obtener datos
  void* stateManager;
  
  // Tópicos dinámicos
  String topicEvents;
  String topicCommands;
  String topicStatus;
  
  // Callback personalizado
  void (*messageCallback)(char*, byte*, unsigned int);
  
  // Método interno para callback de PubSubClient
  static void internalCallback(char* topic, byte* payload, unsigned int length);
  static MQTTManager* instance;

  void generateClientId();
  void generateTopics();
  void publishStatus();

public:
  MQTTManager(const char* server, int port = 8883, const char* username = nullptr, const char* password = nullptr, DisplayOLED* display = nullptr, int ledPin = LED_BUILTIN);
  void begin();
  bool connect();
  bool reconnect();
  bool isConnected();
  bool publish(const char* topic, const char* payload);
  bool publishEvent(const char* eventType, const char* data);
  bool subscribe(const char* topic);
  void loop();
  void setMessageCallback(void (*callback)(char*, byte*, unsigned int));
  void enableAutoStatus(unsigned long intervalMs = 30000);
  void setStateManager(void* stateManager);
  String getClientId();
  String getTopicEvents();
  String getTopicCommands();
  String getTopicStatus();
  PubSubClient& getClient();
};

#endif