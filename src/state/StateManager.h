#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <Arduino.h>
#include "nfc/NFCReader.h"
#include "sensors/MetalSensor.h"
#include "servo/ServoMotor.h"
#include "display/DisplayOLED.h"

enum State {
  STATE_LOGO,
  STATE_WELCOME,
  STATE_COLLECTING,
  STATE_REDEEM
};

class StateManager {
private:
  State currentState;
  int points;
  unsigned long lastActivity;
  bool metalWasDetected;
  int pointsPerMetal;
  unsigned long idleTimeout;
  
  MetalSensor* metal;
  ServoMotor* servo;
  NFCReader* nfc;
  DisplayOLED* display;
  
  void handleWelcome(bool metalDetected);
  void handleCollecting(bool metalDetected, bool cardDetected, String uid);
  void handleRedeem(bool metalDetected, bool cardDetected, String uid);
  void openAndCloseServo();
  void redeemPoints(String uid);
  void resetToWelcome();
  
public:
  StateManager(MetalSensor* metal, ServoMotor* servo, NFCReader* nfc, 
               DisplayOLED* display, int pointsPerMetal, unsigned long idleTimeout);
  void begin();
  void update();
  State getCurrentState();
  int getPoints();
};

#endif
