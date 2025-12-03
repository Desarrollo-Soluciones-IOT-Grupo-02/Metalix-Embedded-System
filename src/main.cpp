#include <Arduino.h>
#include <Wire.h>
#include "wifi/WiFiManager.h"
#include "state/StateManager.h"
#include "nfc/NFCReader.h"
#include "sensors/MetalSensor.h"
#include "servo/ServoMotor.h"
#include "display/DisplayOLED.h"
#include "mqtt/MQTTManager.h"

// Pines
#define SENSOR_PIN 26
#define SERVO_PIN 18

#define PN532_SDA 21
#define PN532_SCL 22

#define OLED_SDA 4
#define OLED_SCL 5

// Configuración MQTT - HiveMQ Cloud
#define MQTT_SERVER "e7801d833ee0419bbad9bceac294aa93.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
// TODO: Reemplazar con tus credenciales de Access Management
#define MQTT_USERNAME "Metalix"
#define MQTT_PASSWORD "Metalix@2025"

// Parámetros
#define POINTS_PER_METAL 10
#define IDLE_TIMEOUT 15000

MetalSensor metal(SENSOR_PIN);
ServoMotor servo(SERVO_PIN, 0, 180);
NFCReader nfc(PN532_SDA, PN532_SCL);
DisplayOLED display(128, 64, &Wire1);

WiFiManager wifi("Ray", "10021976@", LED_BUILTIN, &display);
MQTTManager mqtt(MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, &display, LED_BUILTIN);

StateManager state(&metal, &servo, &nfc, &display,
                    POINTS_PER_METAL, IDLE_TIMEOUT, &mqtt);

// Callback para mensajes MQTT recibidos
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 Mensaje recibido en [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Convertir payload a String
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  // Procesar comandos si es el tópico correcto
  if (String(topic) == mqtt.getTopicCommands()) {
    // Parsear comando (formato simple: {"action":"block_by_weight"})
    if (message.indexOf("unblock_by_weight") > 0) {
      Serial.println("✅ Comando recibido: Desbloquear");
      state.unblockByWeight();
    }
    else if (message.indexOf("block_by_weight") > 0) {
      Serial.println("🚫 Comando recibido: Bloquear por peso");
      state.blockByWeight();
    }
    else {
      Serial.println("⚠️  Comando no reconocido");
    }
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(PN532_SDA, PN532_SCL);
  Wire1.begin(OLED_SDA, OLED_SCL, 400000);

  // Inicializar display y mostrar logo
  display.begin();
  display.showLogo();
  delay(2000);

  // Inicializar módulos
  metal.begin();
  servo.begin();
  nfc.begin();

  // Conectar WiFi con animación en OLED
  wifi.begin();

  // Inicializar MQTT
  mqtt.begin();
  mqtt.setMessageCallback(onMqttMessage);
  mqtt.setStateManager(&state); // Pasar referencia del StateManager
  mqtt.enableAutoStatus(30000); // Publicar estado cada 30 segundos
  
  if (mqtt.connect()) {
    Serial.print("✅ Client ID MQTT: ");
    Serial.println(mqtt.getClientId());
  } else {
    Serial.println("⚠️  MQTT no conectado, se reintentará automáticamente...");
  }

  // Iniciar estado
  state.begin();
}

void loop() {
  state.update();
  mqtt.loop();
  delay(30);
}
