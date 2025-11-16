#include <Arduino.h>
#include <Wire.h>
#include "wifi/WiFiManager.h"
#include "state/StateManager.h"
#include "nfc/NFCReader.h"
#include "sensors/MetalSensor.h"
#include "servo/ServoMotor.h"
#include "display/DisplayOLED.h"

// Pines
#define SENSOR_PIN 26
#define SERVO_PIN 18

#define PN532_SDA 21
#define PN532_SCL 22

#define OLED_SDA 4
#define OLED_SCL 5

// Parámetros
#define POINTS_PER_METAL 10
#define IDLE_TIMEOUT 15000

MetalSensor metal(SENSOR_PIN);
ServoMotor servo(SERVO_PIN, 0, 180);
NFCReader nfc(PN532_SDA, PN532_SCL);
DisplayOLED display(128, 64, &Wire1);

WiFiManager wifi("Ray", "10021976@", LED_BUILTIN, &display);

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

  // Iniciar estado
  state.begin();
}

void loop() {
  state.update();
  delay(30);
}
