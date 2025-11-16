#include <Arduino.h>
#include "wifi/WiFiManager.h"
#include "state/StateManager.h"
#include "nfc/NFCReader.h"
#include "sensors/MetalSensor.h"
#include "servo/ServoMotor.h"
#include "display/DisplayOLED.h"

// Configuración WiFi
const char* WIFI_SSID = "Ray";
const char* WIFI_PASSWORD = "10021976@";

// Configuración de pines
#define SENSOR_PIN 26
#define SERVO_PIN 18
#define SDA_PIN 21
#define SCL_PIN 22

// Configuración del sistema
#define POINTS_PER_METAL 10
#define IDLE_TIMEOUT 15000

// Módulos del sistema
MetalSensor metal(SENSOR_PIN);
ServoMotor servo(SERVO_PIN, 0, 180);
NFCReader nfc(SDA_PIN, SCL_PIN);
DisplayOLED display(128, 64);

// Gestores
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
StateManager stateManager(&metal, &servo, &nfc, &display, POINTS_PER_METAL, IDLE_TIMEOUT);

void setup() {
  Serial.begin(115200);
  delay(200);

  // Conectar WiFi
  wifiManager.begin();

  // Inicializar módulos de hardware
  metal.begin();
  servo.begin();
  nfc.begin();
  
  if (!display.begin()) {
    Serial.println("❌ Error: Display no detectado");
    while (true) delay(1000);
  }

  // Iniciar máquina de estados
  stateManager.begin();
}

void loop() {
  stateManager.update();
  delay(100);
}