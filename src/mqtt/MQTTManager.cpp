#include <WiFiClientSecure.h>
#include "MQTTManager.h"
#include "state/StateManager.h"

// Variable estática para el callback
MQTTManager* MQTTManager::instance = nullptr;

MQTTManager::MQTTManager(const char* server, int port, const char* username, const char* password, DisplayOLED* display, int ledPin)
  : client(wifiClient), server(server), port(port), username(username), password(password), display(display), ledPin(ledPin), 
    lastBlinkTime(0), ledState(false), lastReconnectAttempt(0), lastStatusPublish(0), messageCallback(nullptr), stateManager(nullptr) {
  generateClientId();
  generateTopics();
  instance = this;
}

void MQTTManager::generateClientId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  clientId = "Metalix-" + String(macStr);
}

void MQTTManager::generateTopics() {
  // Usar MAC address como Device ID (sin los :)
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char deviceIdBuf[13];
  sprintf(deviceIdBuf, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  deviceId = String(deviceIdBuf);
  
  // Crear tópicos dinámicos
  topicEvents = "metalix/devices/" + deviceId + "/events";
  topicCommands = "metalix/devices/" + deviceId + "/commands";
  topicStatus = "metalix/devices/" + deviceId + "/status";
  
  Serial.println("📡 Tópicos configurados:");
  Serial.println("   Events: " + topicEvents);
  Serial.println("   Commands: " + topicCommands);
  Serial.println("   Status: " + topicStatus);
}

void MQTTManager::begin() {
  wifiClient.setInsecure();
  client.setServer(server, port);
  client.setCallback(MQTTManager::internalCallback);
}

bool MQTTManager::connect() {
  if (!client.connected()) {
    Serial.println("🔗 Conectando a MQTT...");
    Serial.print("   Server: ");
    Serial.print(server);
    Serial.print(":");
    Serial.println(port);
    Serial.print("   Client ID: ");
    Serial.println(clientId);
    
    if (username && password) {
      Serial.print("   Usuario: ");
      Serial.println(username);
      Serial.println("   [Conectando CON credenciales]");
    } else {
      Serial.println("   [Conectando SIN credenciales]");
    }
    
    if (display) {
      display->showMQTTConnecting(0); // Mostrar inicial
    }

    int attempts = 0;
    bool connected = false;
    while (!connected && attempts < 10) {
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
        
        // Suscribirse al tópico de comandos
        if (client.subscribe(topicCommands.c_str())) {
          Serial.println("✅ Suscrito a: " + topicCommands);
        }
        
        // Publicar evento de conexión
        String connectMsg = "{\"event\":\"connected\",\"client\":\"" + clientId + "\",\"timestamp\":" + String(millis()) + "}";
        if (client.publish(topicEvents.c_str(), connectMsg.c_str())) {
          Serial.println("📤 Evento 'connected' publicado");
        }
      } else {
        attempts++;
        Serial.print("\n   Intento ");
        Serial.print(attempts);
        Serial.print(" falló, rc=");
        Serial.print(client.state());
        Serial.println(" (intentando de nuevo...)");
        delay(1000); // Esperar 1s entre intentos
      }
    }
    if (!connected) {
      Serial.print("\n❌ Fallo MQTT después de ");
      Serial.print(attempts);
      Serial.print(" intentos, rc=");
      Serial.println(client.state());
      Serial.println("   Códigos de error:");
      Serial.println("   -4 = Timeout");
      Serial.println("   -3 = Connection lost");
      Serial.println("   -2 = Connect failed");
      Serial.println("   -1 = Disconnected");
      Serial.println("    0 = Connected");
      Serial.println("    1 = Bad protocol");
      Serial.println("    2 = Bad client ID");
      Serial.println("    3 = Unavailable");
      Serial.println("    4 = Bad credentials");
      Serial.println("    5 = Unauthorized");
    }
    return connected;
  }
  return true;
}

bool MQTTManager::reconnect() {
  unsigned long now = millis();
  if (now - lastReconnectAttempt > 5000) { // Reintentar cada 5 segundos
    lastReconnectAttempt = now;
    Serial.println("🔄 Reintentando conexión MQTT...");
    if (connect()) {
      Serial.println("✅ MQTT reconectado!");
      return true;
    }
  }
  return false;
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
  // Verificar conexión y reconectar si es necesario
  if (!isConnected()) {
    reconnect();
  }
  
  if (isConnected()) {
    client.loop();
    unsigned long currentTime = millis();
    
    // Parpadear LED
    if (currentTime - lastBlinkTime >= 500) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
      lastBlinkTime = currentTime;
    }
    
    // Publicar estado periódicamente si está habilitado
    if (lastStatusPublish > 0 && currentTime - lastStatusPublish >= 30000) {
      publishStatus();
      lastStatusPublish = currentTime;
    }
  }
}

void MQTTManager::enableAutoStatus(unsigned long intervalMs) {
  lastStatusPublish = millis();
}

void MQTTManager::setStateManager(void* sm) {
  stateManager = sm;
}

void MQTTManager::publishStatus() {
  if (isConnected()) {
    unsigned long uptime = millis() / 1000;
    String statusMsg = "{\"uptime\":" + String(uptime) + ",\"heap\":" + String(ESP.getFreeHeap()) + ",\"device_id\":\"" + deviceId + "\"";
    
    // Incluir datos del StateManager si está disponible
    if (stateManager) {
      StateManager* sm = (StateManager*)stateManager;
      statusMsg += ",\"total_kg\":" + String(sm->getKgTotal(), 3);
      statusMsg += ",\"blocked\":" + String(sm->isBlocked() ? "true" : "false");
      statusMsg += ",\"points\":" + String(sm->getPoints());
    }
    
    statusMsg += ",\"timestamp\":" + String(millis()) + "}";
    
    if (client.publish(topicStatus.c_str(), statusMsg.c_str())) {
      Serial.println("📤 Estado publicado: " + statusMsg);
    }
  }
}

bool MQTTManager::publishEvent(const char* eventType, const char* data) {
  if (isConnected()) {
    String eventMsg = "{\"event\":\"" + String(eventType) + "\",\"data\":" + String(data) + ",\"timestamp\":" + String(millis()) + "}";
    return client.publish(topicEvents.c_str(), eventMsg.c_str());
  }
  return false;
}

void MQTTManager::setMessageCallback(void (*callback)(char*, byte*, unsigned int)) {
  messageCallback = callback;
}

void MQTTManager::internalCallback(char* topic, byte* payload, unsigned int length) {
  if (instance && instance->messageCallback) {
    instance->messageCallback(topic, payload, length);
  }
}

String MQTTManager::getClientId() {
  return clientId;
}

String MQTTManager::getTopicEvents() {
  return topicEvents;
}

String MQTTManager::getTopicCommands() {
  return topicCommands;
}

String MQTTManager::getTopicStatus() {
  return topicStatus;
}

PubSubClient& MQTTManager::getClient() {
  return client;
}