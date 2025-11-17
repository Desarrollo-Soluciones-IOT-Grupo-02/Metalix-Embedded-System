#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class DisplayOLED {
public:
    DisplayOLED(int width, int height, TwoWire* bus);

    bool begin();
    void showLogo();
    void showMessage(const String& msg);
    void showWelcome();
    void showPoints(int points);
    void showRedeemMessage();
    void showRedeemSuccess(int points, String uid);
    void showWiFiConnecting(int dots);
    void showWiFiConnected(String ip);
    void showMQTTConnecting(int dots);
    void showMQTTConnected();
    void clear();

private:
    int width, height;
    TwoWire* i2cBus;
    Adafruit_SSD1306* oled;
};