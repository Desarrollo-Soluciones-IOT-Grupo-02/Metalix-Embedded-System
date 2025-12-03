#include "NFCReader.h"

NFCReader::NFCReader(int sda, int scl)
: nfc(sda, scl) {}

void NFCReader::begin() {
    nfc.begin();

    uint32_t ver = nfc.getFirmwareVersion();
    if (!ver) {
        Serial.println("❌ PN532 no encontrado");
        while (1) delay(1000);
    }

    nfc.SAMConfig();
}

bool NFCReader::readCard(String &uidOut) {

    if (millis() - lastRead < interval) return false;
    lastRead = millis();

    uint8_t uid[8];
    uint8_t uidLen = 0;

    // Log solo cada 5 segundos para no spam
    if (millis() - lastLogTime > 5000) {
        Serial.println("🔍 Esperando tarjeta NFC...");
        lastLogTime = millis();
    }

    bool ok = nfc.readPassiveTargetID(
      PN532_MIFARE_ISO14443A,
      uid,
      &uidLen,
      150
    );

    if (!ok || uidLen == 0) {
        // Si no se detecta y ha pasado más de 10 segundos desde la última detección exitosa, reconfigurar PN532
        if (millis() - lastSuccessfulRead > 10000) {
            Serial.println("🔄 Reconfigurando PN532 por inactividad...");
            nfc.SAMConfig();
            lastSuccessfulRead = millis();
        }
        return false;
    }

    // Actualizar tiempo de última detección exitosa
    lastSuccessfulRead = millis();

    uidOut = "";

    for (int i = 0; i < uidLen; i++) {
        if (uid[i] < 0x10) uidOut += "0";
        uidOut += String(uid[i], HEX);
        if (i < uidLen - 1) uidOut += " ";
    }

    uidOut.toUpperCase();
    Serial.print("🎫 Tarjeta detectada: ");
    Serial.println(uidOut);
    return true;
}