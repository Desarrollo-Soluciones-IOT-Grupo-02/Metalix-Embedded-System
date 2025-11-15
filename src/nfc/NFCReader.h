#ifndef NFC_READER_H
#define NFC_READER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

class NFCReader {
private:
    Adafruit_PN532 nfc;

public:
    NFCReader(int sda, int scl);
    void begin();
    bool readCard(String &uidString);
};

#endif