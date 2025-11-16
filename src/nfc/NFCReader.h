#ifndef NFCREADER_H
#define NFCREADER_H

#include <Arduino.h>
#include <Adafruit_PN532.h>

class NFCReader {
public:
    NFCReader(int sda, int scl);
    void begin();
    bool readCard(String &uidOut);

private:
    Adafruit_PN532 nfc;

    unsigned long lastRead = 0;
    const int interval = 80; 
    unsigned long lastSuccessfulRead = 0;
    unsigned long lastLogTime = 0;
};

#endif