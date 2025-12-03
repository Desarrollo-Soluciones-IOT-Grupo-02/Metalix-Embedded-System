#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoMotor {
private:
    Servo servo;
    int pin;
    int openAngle;
    int closeAngle;

public:
    ServoMotor(int servoPin, int openDeg = 0, int closeDeg = 90);

    void begin();
    void open();
    void close();
    void moveTo(int angle);
};

#endif