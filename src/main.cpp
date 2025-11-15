#include <Arduino.h>
#include "nfc/NFCReader.h"
#include "sensors/MetalSensor.h"
#include "servo/ServoMotor.h"

#define SENSOR_PIN 26
#define SERVO_PIN 18
#define SDA_PIN 21
#define SCL_PIN 22

MetalSensor metal(SENSOR_PIN);
ServoMotor servo(SERVO_PIN, 0, 180);
NFCReader nfc(SDA_PIN, SCL_PIN);

void setup() {
  Serial.begin(115200);

  metal.begin();
  servo.begin();
  nfc.begin();

  Serial.println("Sistema listo.");
}

void loop() {
  
  // ---- Sensor inductivo ----
  if (metal.isMetalDetected()) {
    Serial.println("🧲 Metal detectado → Abriendo servo");
    servo.open();
    delay(2500);       // tiempo que queda abierto
    Serial.println("Cerrando...");
    servo.close();
  }

  // ---- Lector NFC ----
  String uid;
  if (nfc.readCard(uid)) {
    Serial.print("📱 Tarjeta detectada: ");
    Serial.println(uid);
    
    delay(1000);
  }

  delay(100);
}