#include "StateManager.h"

StateManager::StateManager(MetalSensor* metal, ServoMotor* servo, NFCReader* nfc, 
                           DisplayOLED* display, int pointsPerMetal, unsigned long idleTimeout) {
  this->metal = metal;
  this->servo = servo;
  this->nfc = nfc;
  this->display = display;
  this->pointsPerMetal = pointsPerMetal;
  this->idleTimeout = idleTimeout;
  this->currentState = STATE_LOGO;
  this->points = 0;
  this->lastActivity = 0;
  this->metalWasDetected = false;
}

void StateManager::begin() {
  display->showLogo();
  Serial.println("✅ Sistema iniciado - Mostrando logo");
  delay(3000);
  
  currentState = STATE_WELCOME;
  display->showWelcome();
  Serial.println("Estado: BIENVENIDA");
  lastActivity = millis();
}

void StateManager::update() {
  bool metalDetected = metal->isMetalDetected();
  String uid;
  bool cardDetected = nfc->readCard(uid);

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
      
    case STATE_LOGO:
      break;
  }
}

void StateManager::handleWelcome(bool metalDetected) {
  if (metalDetected && !metalWasDetected) {
    points = pointsPerMetal;
    Serial.println("🧲 Primer metal detectado - Iniciando recolección");
    
    openAndCloseServo();
    
    currentState = STATE_COLLECTING;
    display->showPoints(points);
    Serial.print("Puntos: ");
    Serial.println(points);
    lastActivity = millis();
  }
  metalWasDetected = metalDetected;
}

void StateManager::handleCollecting(bool metalDetected, bool cardDetected, String uid) {
  // Acumulando puntos por cada metal
  if (metalDetected && !metalWasDetected) {
    points += pointsPerMetal;
    Serial.print("🧲 Metal detectado - Puntos: ");
    Serial.println(points);
    
    openAndCloseServo();
    
    display->showPoints(points);
    lastActivity = millis();
  }
  metalWasDetected = metalDetected;
  
  // Timeout sin actividad: mostrar mensaje de canje
  if (points > 0 && (millis() - lastActivity > idleTimeout)) {
    currentState = STATE_REDEEM;
    display->showRedeemMessage();
    Serial.println("Estado: ESPERANDO TARJETA PARA CANJE");
  }
  
  // Si pasa tarjeta mientras recolecta
  if (cardDetected && points > 0) {
    redeemPoints(uid);
  }
}

void StateManager::handleRedeem(bool metalDetected, bool cardDetected, String uid) {
  if (cardDetected) {
    redeemPoints(uid);
  }
  
  // Si siguen insertando metal, volver a recolección
  if (metalDetected && !metalWasDetected) {
    points += pointsPerMetal;
    Serial.print("🧲 Metal detectado - Puntos: ");
    Serial.println(points);
    
    openAndCloseServo();
    
    currentState = STATE_COLLECTING;
    display->showPoints(points);
    lastActivity = millis();
  }
  metalWasDetected = metalDetected;
}

void StateManager::openAndCloseServo() {
  servo->open();
  delay(2000);
  servo->close();
}

void StateManager::redeemPoints(String uid) {
  Serial.print("📱 Tarjeta detectada: ");
  Serial.println(uid);
  Serial.print("💰 Canjeando ");
  Serial.print(points);
  Serial.println(" puntos");
  
  display->showRedeemSuccess(points);
  delay(3000);
  
  resetToWelcome();
}

void StateManager::resetToWelcome() {
  points = 0;
  currentState = STATE_WELCOME;
  display->showWelcome();
  Serial.println("Estado: BIENVENIDA");
  lastActivity = millis();
}

State StateManager::getCurrentState() {
  return currentState;
}

int StateManager::getPoints() {
  return points;
}
