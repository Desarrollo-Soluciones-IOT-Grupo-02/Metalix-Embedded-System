#include "ServoMotor.h"

ServoMotor::ServoMotor(int servoPin, int openDeg, int closeDeg) {
    pin = servoPin;
    openAngle = openDeg;
    closeAngle = closeDeg;
}

void ServoMotor::begin() {
    servo.attach(pin);
}

void ServoMotor::moveTo(int angle) {
    servo.write(angle);
    delay(400);
}

void ServoMotor::open() {
    moveTo(openAngle);
}

void ServoMotor::close() {
    moveTo(closeAngle);
}