#include "StateManager.h"

StateManager::StateManager(MetalSensor* metal, ServoMotor* servo, NFCReader* nfc,
                           DisplayOLED* display, int ppm, unsigned long idle)
{
  this->metal = metal;
  this->servo = servo;
  this->nfc = nfc;
  this->display = display;

  this->pointsPerMetal = ppm;
  this->idleTimeout = idle;

  currentState = STATE_LOGO;

  points = 0;
  lastActivity = 0;

  metalLast = false;
  cardLast = false;

  welcomeShown = false;
  collectingShown = false;
  redeemShown = false;

  servoOpen = false;
  servoTimer = 0;

  redeemWait = false;
  redeemTimer = 0;

  redeemCooldown = false;
  redeemCooldownTimer = 0;

  lastRedeemedUID = "";
}

void StateManager::begin() {
  display->showLogo();
  delay(2000);

  currentState = STATE_WELCOME;
  display->showWelcome();
  welcomeShown = true;

  lastActivity = millis();
}

void StateManager::update() {

  // ------------------------------
  // Servo no bloqueante
  // ------------------------------
  if (servoOpen && millis() - servoTimer >= 2000) {
    servo->close();
    servoOpen = false;
  }

  // ------------------------------
  // Pantalla final no bloqueante
  // ------------------------------
  if (redeemWait && millis() - redeemTimer >= 2500) {
    redeemWait = false;
    resetToWelcome();
  }

  // ------------------------------
  // Cooldown para evitar canjes múltiples
  // ------------------------------
  if (redeemCooldown && millis() - redeemCooldownTimer >= 3000) {
    redeemCooldown = false;
  }

  // ------------------------------
  // Lectura de sensores
  // ------------------------------
  bool metalDetected = metal->isMetalDetected();

  String uid;
  bool cardDetected = nfc->readCard(uid);

  // ------------------------------
  // Máquina de estados
  // ------------------------------
  switch (currentState) {

    case STATE_WELCOME:
      handleWelcome(metalDetected);
      break;

    case STATE_COLLECTING:
      handleCollecting(metalDetected, cardDetected, uid);
      break;

    case STATE_REDEEM:
      handleRedeem(metalDetected, cardDetected, uid);
      break;

    default:
      break;
  }

  metalLast = metalDetected;
  cardLast = cardDetected;
}

void StateManager::handleWelcome(bool metal) {

  if (!welcomeShown) {
    display->showWelcome();
    welcomeShown = true;
  }

  if (metal && !metalLast) {
    points = pointsPerMetal;

    openServoAsync();
    display->showPoints(points);

    collectingShown = false;
    welcomeShown = false;

    lastActivity = millis();
    currentState = STATE_COLLECTING;
  }
}

void StateManager::handleCollecting(bool metal, bool card, String uid) {

  if (!collectingShown) {
    display->showPoints(points);
    collectingShown = true;
  }

  if (metal && !metalLast) {
    points += pointsPerMetal;

    openServoAsync();
    display->showPoints(points);

    lastActivity = millis();
  }

  if (points > 0 && millis() - lastActivity > idleTimeout) {
    redeemShown = false;
    currentState = STATE_REDEEM;
    return;
  }

  if (card && !cardLast) {
    redeemPoints(uid);
  }
}

void StateManager::handleRedeem(bool metal, bool card, String uid) {

  if (!redeemShown) {
    display->showRedeemMessage();
    redeemShown = true;
  }

  if (card && !cardLast && !redeemCooldown) {
    redeemPoints(uid);
  }

  if (metal && !metalLast) {
    points += pointsPerMetal;
    openServoAsync();
    display->showPoints(points);

    collectingShown = false;
    redeemShown = false;

    currentState = STATE_COLLECTING;
    lastActivity = millis();
  }
}

void StateManager::openServoAsync() {
  servo->open();
  servoOpen = true;
  servoTimer = millis();
}

void StateManager::redeemPoints(String uid) {
  Serial.print("📱 Tarjeta detectada: ");
  Serial.println(uid);
  Serial.print("💰 Canjeando ");
  Serial.print(points);
  Serial.println(" puntos");

  lastRedeemedUID = uid;

  display->showRedeemSuccess(points, uid);

  // Activar cooldown para evitar canjes múltiples
  redeemCooldown = true;
  redeemCooldownTimer = millis();

  redeemWait = true;
  redeemTimer = millis();
}

void StateManager::resetToWelcome() {
  points = 0;

  metalLast = false;
  cardLast = false;

  welcomeShown = true;
  collectingShown = false;
  redeemShown = false;

  currentState = STATE_WELCOME;
  display->showWelcome();
}

String StateManager::getLastRedeemedUID() {
  return lastRedeemedUID;
}

String StateManager::getLastDetectedUID() {
  return lastUID;
}

unsigned long StateManager::getLastActivity() {
  return lastActivity;
}