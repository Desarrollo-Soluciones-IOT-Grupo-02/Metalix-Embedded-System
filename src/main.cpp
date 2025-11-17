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

// Configuración MQTT (con TLS sin certificados)
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 8883
#define MQTT_USERNAME nullptr
#define MQTT_PASSWORD nullptr

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
                    POINTS_PER_METAL, IDLE_TIMEOUT);

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
  mqtt.connect();

  Serial.print("Client ID MQTT: ");
  Serial.println(mqtt.getClientId());

  // Iniciar estado
  state.begin();
}

void loop() {
  state.update();
  mqtt.loop();
  delay(30);
}
