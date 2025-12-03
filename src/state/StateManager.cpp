#include "StateManager.h"
#include "mqtt/MQTTManager.h"

StateManager::StateManager(MetalSensor* metal, ServoMotor* servo, NFCReader* nfc,
                           DisplayOLED* display, int ppm, unsigned long idle,
                           MQTTManager* mqtt)
{
  this->metal = metal;
  this->servo = servo;
  this->nfc = nfc;
  this->display = display;
  this->mqtt = mqtt;

  this->pointsPerMetal = ppm;
  this->idleTimeout = idle;

  currentState = STATE_LOGO;

  points = 0;
  lastActivity = 0;
  
  // Inicializar sistema de peso
  kgSession = 0.0;
  kgTotal = 0.0;
  kgPerMetal = random(80, 150) / 1000.0; // Entre 0.080 y 0.150 kg por metal
  isBlockedByWeight = false; // Inicialmente desbloqueado

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
    if (isBlockedByWeight) {
      display->showMessage("LIMITE DE PESO", "Dispositivo bloqueado");
    } else {
      display->showWelcome();
    }
    welcomeShown = true;
  }

  // Solo detectar metal si NO está bloqueado y el servo NO está abierto
  if (metal && !metalLast && !servoOpen && !isBlockedByWeight) {
    points = pointsPerMetal;
    
    // Simular peso del metal detectado
    float metalWeight = random(80, 150) / 1000.0; // 0.080 - 0.150 kg
    kgSession += metalWeight;
    kgTotal += metalWeight;

    openServoAsync();
    display->showPoints(points);
    
    // Publicar evento de detección de metal con peso
    if (mqtt) {
      String eventData = "{\"points\":" + String(points) + ",\"total\":" + String(points) + ",\"weight_kg\":" + String(metalWeight, 3) + ",\"session_kg\":" + String(kgSession, 3) + ",\"total_kg\":" + String(kgTotal, 3) + "}";
      mqtt->publishEvent("metal_detected", eventData.c_str());
      Serial.println("🔩 Evento: metal_detected publicado");
      Serial.print("⚖️  Peso: ");
      Serial.print(metalWeight, 3);
      Serial.print(" kg | Sesión: ");
      Serial.print(kgSession, 3);
      Serial.print(" kg | Total: ");
      Serial.print(kgTotal, 3);
      Serial.println(" kg");
    }

    collectingShown = false;
    welcomeShown = false;

    lastActivity = millis();
    currentState = STATE_COLLECTING;
  }
}

void StateManager::handleCollecting(bool metal, bool card, String uid) {

  if (!collectingShown) {
    if (isBlockedByWeight) {
      display->showMessage("LIMITE DE PESO", "Dispositivo bloqueado");
    } else {
      display->showPoints(points);
    }
    collectingShown = true;
  }

  // Solo detectar metal si NO está bloqueado y el servo NO está abierto
  if (metal && !metalLast && !servoOpen && !isBlockedByWeight) {
    points += pointsPerMetal;
    
    // Simular peso del metal detectado
    float metalWeight = random(80, 150) / 1000.0; // 0.080 - 0.150 kg
    kgSession += metalWeight;
    kgTotal += metalWeight;

    openServoAsync();
    display->showPoints(points);
    
    // Publicar evento de detección de metal con peso
    if (mqtt) {
      String eventData = "{\"points\":" + String(pointsPerMetal) + ",\"total\":" + String(points) + ",\"weight_kg\":" + String(metalWeight, 3) + ",\"session_kg\":" + String(kgSession, 3) + ",\"total_kg\":" + String(kgTotal, 3) + "}";
      mqtt->publishEvent("metal_detected", eventData.c_str());
      Serial.println("🔩 Evento: metal_detected publicado");
      Serial.print("⚖️  Peso: ");
      Serial.print(metalWeight, 3);
      Serial.print(" kg | Sesión: ");
      Serial.print(kgSession, 3);
      Serial.print(" kg | Total: ");
      Serial.print(kgTotal, 3);
      Serial.println(" kg");
    }

    lastActivity = millis();
  }

  if (points > 0 && millis() - lastActivity > idleTimeout) {
    // Publicar evento de timeout
    if (mqtt) {
      String eventData = "{\"points\":" + String(points) + ",\"reason\":\"timeout\"}";
      mqtt->publishEvent("redeem_requested", eventData.c_str());
      Serial.println("⏰ Evento: redeem_requested (timeout) publicado");
    }
    
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
    if (isBlockedByWeight) {
      display->showMessage("LIMITE DE PESO", "Dispositivo bloqueado");
    } else {
      display->showRedeemMessage();
    }
    redeemShown = true;
  }

  if (card && !cardLast && !redeemCooldown) {
    redeemPoints(uid);
  }

  // Solo detectar metal si NO está bloqueado y el servo NO está abierto
  if (metal && !metalLast && !servoOpen && !isBlockedByWeight) {
    points += pointsPerMetal;
    
    // Simular peso del metal detectado
    float metalWeight = random(80, 150) / 1000.0; // 0.080 - 0.150 kg
    kgSession += metalWeight;
    kgTotal += metalWeight;
    
    openServoAsync();
    display->showPoints(points);
    
    // Publicar evento de detección de metal con peso
    if (mqtt) {
      String eventData = "{\"points\":" + String(pointsPerMetal) + ",\"total\":" + String(points) + ",\"weight_kg\":" + String(metalWeight, 3) + ",\"session_kg\":" + String(kgSession, 3) + ",\"total_kg\":" + String(kgTotal, 3) + "}";
      mqtt->publishEvent("metal_detected", eventData.c_str());
      Serial.println("🔩 Evento: metal_detected publicado");
      Serial.print("⚖️  Peso: ");
      Serial.print(metalWeight, 3);
      Serial.print(" kg | Sesión: ");
      Serial.print(kgSession, 3);
      Serial.print(" kg | Total: ");
      Serial.print(kgTotal, 3);
      Serial.println(" kg");
    }

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
  Serial.print(" puntos | ");
  Serial.print(kgSession, 3);
  Serial.println(" kg");

  lastRedeemedUID = uid;
  
  // Publicar evento de canje exitoso con peso
  if (mqtt) {
    String eventData = "{\"nfc_uid\":\"" + uid + "\",\"points_redeemed\":" + String(points) + ",\"kg_redeemed\":" + String(kgSession, 3) + ",\"device_total_kg\":" + String(kgTotal, 3) + ",\"timestamp\":" + String(millis()) + "}";
    mqtt->publishEvent("points_redeemed", eventData.c_str());
    Serial.println("✅ Evento: points_redeemed publicado");
    Serial.print("⚖️  Kg canjeados: ");
    Serial.print(kgSession, 3);
    Serial.print(" kg | Kg total del dispositivo: ");
    Serial.print(kgTotal, 3);
    Serial.println(" kg");
  }

  display->showRedeemSuccess(points, uid);

  // Activar cooldown para evitar canjes múltiples
  redeemCooldown = true;
  redeemCooldownTimer = millis();

  redeemWait = true;
  redeemTimer = millis();
}

void StateManager::resetToWelcome() {
  points = 0;
  kgSession = 0.0; // Resetear solo kg de la sesión
  // kgTotal NO se resetea, se mantiene acumulado

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

int StateManager::getPoints() {
  return points;
}

float StateManager::getKgSession() {
  return kgSession;
}

float StateManager::getKgTotal() {
  return kgTotal;
}

void StateManager::blockByWeight() {
  isBlockedByWeight = true;
  Serial.println("🚫 Dispositivo bloqueado por peso");
  
  // Actualizar display
  display->showMessage("LIMITE DE PESO", "Dispositivo bloqueado");
  
  // Publicar evento de bloqueo
  if (mqtt) {
    String eventData = "{\"total_kg\":" + String(kgTotal, 3) + ",\"reason\":\"weight_limit\"}";
    mqtt->publishEvent("device_blocked", eventData.c_str());
  }
}

void StateManager::unblockByWeight() {
  isBlockedByWeight = false;
  Serial.println("✅ Dispositivo desbloqueado");
  
  // Volver al estado inicial (Welcome)
  resetToWelcome();
  
  // Publicar evento de desbloqueo
  if (mqtt) {
    String eventData = "{\"total_kg\":" + String(kgTotal, 3) + "}";
    mqtt->publishEvent("device_unblocked", eventData.c_str());
  }
}

bool StateManager::isBlocked() {
  return isBlockedByWeight;
}

String StateManager::getLastDetectedUID() {
  return lastUID;
}

unsigned long StateManager::getLastActivity() {
  return lastActivity;
}