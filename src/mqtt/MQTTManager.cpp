#include <WiFiClientSecure.h>
#include "MQTTManager.h"

MQTTManager::MQTTManager(const char* server, int port, const char* username, const char* password, DisplayOLED* display, int ledPin)
  : client(wifiClient), server(server), port(port), username(username), password(password), display(display), ledPin(ledPin), lastBlinkTime(0), ledState(false) {
  generateClientId();
}

void MQTTManager::generateClientId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  clientId = "Metalix-" + String(macStr);
}

void MQTTManager::begin() {
  wifiClient.setInsecure();
  client.setServer(server, port);
}

bool MQTTManager::connect() {
  if (!client.connected()) {
    Serial.println("🔗 Conectando a MQTT...");
    if (display) {
      display->showMQTTConnecting(0); // Mostrar inicial
    }

    int attempts = 0;
    bool connected = false;
    while (!connected && attempts < 10) { // Reintentar hasta 10 veces
      if (display) {
        int dots = attempts % 4;
        display->showMQTTConnecting(dots);
      }
      Serial.print(".");
      bool result = false;
      if (username && password) {
        result = client.connect(clientId.c_str(), username, password);
      } else {
        result = client.connect(clientId.c_str());
      }
      if (result) {
        connected = true;
        Serial.println("\n✅ MQTT Conectado!");
        if (display) {
          display->showMQTTConnected();
        }
      } else {
        attempts++;
        delay(1000); // Esperar 1s entre intentos
      }
    }
    if (!connected) {
      Serial.print("\n❌ Fallo MQTT, rc=");
      Serial.println(client.state());
    }
    return connected;
  }
  return true;
}

bool MQTTManager::isConnected() {
  return client.connected();
}

bool MQTTManager::publish(const char* topic, const char* payload) {
  if (isConnected()) {
    return client.publish(topic, payload);
  }
  return false;
}

bool MQTTManager::subscribe(const char* topic) {
  if (isConnected()) {
    return client.subscribe(topic);
  }
  return false;
}

void MQTTManager::loop() {
  client.loop();
  if (isConnected()) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= 500) { // Parpadear cada 500ms
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
      lastBlinkTime = currentTime;
    }
  }
}

String MQTTManager::getClientId() {
  return clientId;
}