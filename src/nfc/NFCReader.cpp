#include "NFCReader.h"

NFCReader::NFCReader(int sda, int scl) : nfc(sda, scl) {}

void NFCReader::begin() {
    Wire.begin();
    nfc.begin();

    uint32_t v = nfc.getFirmwareVersion();
    if (!v) {
        Serial.println("No se detecto el PN532");
        while (1) delay(10);
    }

    nfc.SAMConfig();
}

bool NFCReader::readCard(String &uidOut) {
    uint8_t uid[7];
    uint8_t uidLen = 0;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) {
        uidOut = "";
        for (uint8_t i = 0; i < uidLen; i++) {
            if (uid[i] < 0x10) uidOut += "0";
            uidOut += String(uid[i], HEX);
            if (i < uidLen - 1) uidOut += " ";
        }
        uidOut.toUpperCase();
        return true;
    }

    return false;
}