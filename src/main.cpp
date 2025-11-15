#include <Arduino.h>
#include "nfc/NFCReader.h"
#include "sensors/MetalSensor.h"

#define SENSOR_PIN 26
#define SDA_PIN 21
#define SCL_PIN 22

MetalSensor metal(SENSOR_PIN);
NFCReader nfc(SDA_PIN, SCL_PIN);

void setup() {
  Serial.begin(115200);

  metal.begin();
  nfc.begin();

  Serial.println("Sistema listo.");
}

void loop() {
  
  // ---- Sensor inductivo ----
  if (metal.isMetalDetected()) {
    Serial.println("🧲 Metal detectado");
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