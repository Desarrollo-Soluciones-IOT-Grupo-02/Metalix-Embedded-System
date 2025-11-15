#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayOLED {
public:
    DisplayOLED(int width, int height);

    bool begin();
    void showLogo();
    void showMessage(const String& msg);
    void showWelcome();
    void showPoints(int points);
    void showRedeemMessage();
    void showRedeemSuccess(int points);
    void clear();

private:
    Adafruit_SSD1306 oled;
};