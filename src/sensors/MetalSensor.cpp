#include "MetalSensor.h"

MetalSensor::MetalSensor(int sensorPin) {
    pin = sensorPin;
}

void MetalSensor::begin() {
    pinMode(pin, INPUT);
}

bool MetalSensor::isMetalDetected() {
    return digitalRead(pin) == LOW;
}