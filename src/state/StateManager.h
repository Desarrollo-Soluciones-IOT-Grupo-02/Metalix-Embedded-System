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
public:
  StateManager(MetalSensor* metal, ServoMotor* servo, NFCReader* nfc,
               DisplayOLED* display, int pointsPerMetal, unsigned long idleTimeout);

  void begin();
  void update();
  State getCurrentState();
  int getPoints();
  String getLastRedeemedUID();
  String getLastDetectedUID();
  unsigned long getLastActivity();

private:
  // Hardware
  MetalSensor* metal;
  ServoMotor* servo;
  NFCReader* nfc;
  DisplayOLED* display;

  // Estado general
  State currentState;
  int points;
  unsigned long lastActivity;

  // Flags internos
  bool metalLast;
  bool cardLast;

  bool welcomeShown;
  bool collectingShown;
  bool redeemShown;

  String lastUID;

  // UID del último canje
  String lastRedeemedUID;

  // Config
  int pointsPerMetal;
  unsigned long idleTimeout;

  // Servo no bloqueante
  bool servoOpen;
  unsigned long servoTimer;

  // Pantalla final no bloqueante
  bool redeemWait;
  unsigned long redeemTimer;

  bool redeemCooldown;
  unsigned long redeemCooldownTimer;

  // Métodos privados
  void handleWelcome(bool metal);
  void handleCollecting(bool metal, bool card, String uid);
  void handleRedeem(bool metal, bool card, String uid);

  void openServoAsync();
  void redeemPoints(String uid);
  void resetToWelcome();
};

#endif