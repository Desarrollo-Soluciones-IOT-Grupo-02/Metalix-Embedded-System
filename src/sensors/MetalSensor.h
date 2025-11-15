#ifndef METAL_SENSOR_H
#define METAL_SENSOR_H

#include <Arduino.h>

class MetalSensor {
private:
    int pin;

public:
    MetalSensor(int sensorPin);
    void begin();
    bool isMetalDetected();
};

#endif