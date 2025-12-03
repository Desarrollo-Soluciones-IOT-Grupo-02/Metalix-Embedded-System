#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password, int ledPin, DisplayOLED* display) {
  this->ssid = ssid;
  this->password = password;
  this->ledPin = ledPin;
  this->display = display;
}

void WiFiManager::begin() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  Serial.println("🌐 Conectando a WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    if (display) {
      int dots = attempts % 4;
      display->showWiFiConnecting(dots);
    }
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(ledPin, HIGH);
    if (display) {
      String ip = WiFi.localIP().toString();
      display->showWiFiConnected(ip);
    }
  } else {
    Serial.println("\n❌ No se pudo conectar a WiFi");
    digitalWrite(ledPin, LOW);
  }
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() {
  return WiFi.localIP().toString();
}

void WiFiManager::reconnect() {
  if (!isConnected()) {
    Serial.println("🔄 Reconectando WiFi...");
    digitalWrite(ledPin, LOW);
    WiFi.disconnect();
    WiFi.reconnect();
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (isConnected()) {
      Serial.println("\n✅ Reconectado!");
      digitalWrite(ledPin, HIGH);
    } else {
      Serial.println("\n❌ Fallo al reconectar");
    }
  }
}
