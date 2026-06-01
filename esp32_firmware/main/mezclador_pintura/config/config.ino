/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║         SISTEMA DE MEZCLADO AUTOMÁTICO DE PINTURA                       ║
 * ║                  ESP32-S3  ·  mezclador_pintura.ino                     ║
 * ╠══════════════════════════════════════════════════════════════════════════╣
 * ║  Protocolo serial (115200 bps) — comandos JSON:                         ║
 * ║                                                                          ║
 * ║  Mezclar color:                                                          ║
 * ║    {"cmd":"mix","r":30,"g":20,"b":10,"w":40}   (valores en mL)          ║
 * ║                                                                          ║
 * ║  Limpiar:                                                                ║
 * ║    {"cmd":"clean","volume":50}                  (mL de agua)             ║
 * ║                                                                          ║
 * ║  Consultar estado:                                                       ║
 * ║    {"cmd":"status"}                                                      ║
 * ║                                                                          ║
 * ║  Parada de emergencia:                                                   ║
 * ║    {"cmd":"stop"}   ó pulsar botón GPIO 11                               ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include <ArduinoJson.h>        // Instalar: ArduinoJson by Benoit Blanchon v7.x
#include "config.h"

// ══════════════════════════════════════════════════════════════════════════════
//  TABLAS INDEXADAS POR PUMP_x  (generadas desde las macros de config.h)
// ══════════════════════════════════════════════════════════════════════════════
const uint8_t PIN_IN1[PUMP_COUNT]  = PUMP_IN1_PINS;
const uint8_t PIN_IN2[PUMP_COUNT]  = PUMP_IN2_PINS;
const uint8_t PIN_EN [PUMP_COUNT]  = PUMP_EN_PINS;
const uint8_t PWM_CH [PUMP_COUNT]  = PWM_CHANNELS;

const char*   PUMP_NAME[PUMP_COUNT] = { "RED", "GREEN", "BLUE", "WHITE", "CLEAN" };

// ══════════════════════════════════════════════════════════════════════════════
//  ESTADO GLOBAL
// ══════════════════════════════════════════════════════════════════════════════
SystemState_t sysState           = STATE_IDLE;
float         tankLevel[PUMP_COUNT] = TANK_LEVELS_INIT;  // %
float         flowRate[PUMP_COUNT]  = FLOW_RATES_ML_S;   // mL/s

uint32_t      lastStateReport    = 0;
bool          emergencyLatched   = false;

// ══════════════════════════════════════════════════════════════════════════════
//  PROTOTIPOS
// ══════════════════════════════════════════════════════════════════════════════
void     initPumps();
void     stopAllPumps();
void     pumpOn (uint8_t idx, uint8_t duty = PWM_DUTY_MAX);
void     pumpOff(uint8_t idx);
bool     dispensar(uint8_t idx, float volumen_mL, uint8_t duty = PWM_DUTY_MAX);
void     cmdMix(JsonDocument& doc);
void     cmdClean(JsonDocument& doc);
void     cmdStatus();
void     processSerial();
void     checkEmergency();
void     sendStatus();
void     sendError(const char* msg);

// ══════════════════════════════════════════════════════════════════════════════
//  SETUP
// ══════════════════════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);

    DBGLN("[ MEZCLADOR ] Iniciando sistema...");

    // Parada de emergencia
    pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);

    // Inicializar bombas
    initPumps();

    // Asegurar todo apagado
    stopAllPumps();

    DBGLN("[ MEZCLADOR ] Listo. Esperando comandos JSON...");
    sendStatus();
}

// ══════════════════════════════════════════════════════════════════════════════
//  LOOP
// ══════════════════════════════════════════════════════════════════════════════
void loop() {
    // 1. Verificar parada de emergencia (botón físico)
    checkEmergency();

    // 2. Procesar comandos seriales
    if (sysState != STATE_EMERGENCY_STOP) {
        processSerial();
    }

    // 3. Enviar estado periódico
    if (millis() - lastStateReport >= INTERVAL_STATE_UPDATE_MS) {
        lastStateReport = millis();
        sendStatus();
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  INICIALIZACIÓN DE BOMBAS
// ══════════════════════════════════════════════════════════════════════════════
void initPumps() {
    for (uint8_t i = 0; i < PUMP_COUNT; i++) {
        pinMode(PIN_IN1[i], OUTPUT);
        pinMode(PIN_IN2[i], OUTPUT);
        digitalWrite(PIN_IN1[i], LOW);
        digitalWrite(PIN_IN2[i], LOW);

        ledcAttach(PIN_EN[i], PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
        ledcWrite(PIN_EN[i], PWM_DUTY_OFF);

        DBGF("  Bomba %s — IN1:%d IN2:%d EN:%d\n",
             PUMP_NAME[i], PIN_IN1[i], PIN_IN2[i], PIN_EN[i]);
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  CONTROL DE BOMBAS
// ══════════════════════════════════════════════════════════════════════════════
void stopAllPumps() {
    for (uint8_t i = 0; i < PUMP_COUNT; i++) pumpOff(i);
}

void pumpOn(uint8_t idx, uint8_t duty) {
    if (idx >= PUMP_COUNT) return;
    digitalWrite(PIN_IN1[idx], HIGH);
    digitalWrite(PIN_IN2[idx], LOW);
    ledcWrite(PIN_EN[idx], duty);
}

void pumpOff(uint8_t idx) {
    if (idx >= PUMP_COUNT) return;
    ledcWrite(PIN_EN[idx], PWM_DUTY_OFF);
    digitalWrite(PIN_IN1[idx], LOW);
    digitalWrite(PIN_IN2[idx], LOW);
}

/**
 * Dispensar un volumen específico de una bomba.
 * Retorna false si el tanque está bajo o hay emergencia.
 */
bool dispensar(uint8_t idx, float volumen_mL, uint8_t duty) {
    if (idx >= PUMP_COUNT)                              return false;
    if (tankLevel[idx] <= SAFETY_TANK_MIN_PERCENT)      return false;
    if (emergencyLatched)                               return false;
    if (volumen_mL <= 0.0f)                             return true;  // nada que hacer

    uint32_t duracion_ms = MS_FOR_ML(volumen_mL, flowRate[idx]);
    if (duracion_ms < PUMP_MIN_DURATION_MS) duracion_ms = PUMP_MIN_DURATION_MS;

    DBGF("  [%s] %.1f mL → %lu ms (duty %d)\n",
         PUMP_NAME[idx], volumen_mL, duracion_ms, duty);

    pumpOn(idx, duty);
    uint32_t inicio = millis();

    // Espera activa con vigilancia de emergencia
    while (millis() - inicio < duracion_ms) {
        checkEmergency();
        if (emergencyLatched) { pumpOff(idx); return false; }
        delay(10);
    }

    pumpOff(idx);

    // Actualizar nivel estimado del tanque (simplificado)
    float consumo_pct = (volumen_mL / 1000.0f) * 100.0f;  // asume tanque de 1 L
    tankLevel[idx] -= consumo_pct;
    if (tankLevel[idx] < 0.0f) tankLevel[idx] = 0.0f;

    delay(DELAY_PUMP_BETWEEN_MS);
    return true;
}

// ══════════════════════════════════════════════════════════════════════════════
//  COMANDOS
// ══════════════════════════════════════════════════════════════════════════════

/** {"cmd":"mix","r":30,"g":20,"b":10,"w":40} */
void cmdMix(JsonDocument& doc) {
    float r = doc["r"] | 0.0f;
    float g = doc["g"] | 0.0f;
    float b = doc["b"] | 0.0f;
    float w = doc["w"] | 0.0f;

    if (r + g + b + w <= 0.0f) { sendError("Volúmenes inválidos"); return; }

    sysState = STATE_MIXING;
    DBGLN("[ MIX ] Iniciando mezcla...");

    bool ok = true;
    if (r > 0) ok &= dispensar(PUMP_RED,   r);
    if (g > 0) ok &= dispensar(PUMP_GREEN, g);
    if (b > 0) ok &= dispensar(PUMP_BLUE,  b);
    if (w > 0) ok &= dispensar(PUMP_WHITE, w);

    sysState = ok ? STATE_IDLE : STATE_ERROR;
    DBGLN(ok ? "[ MIX ] Completado." : "[ MIX ] Error durante mezcla.");
    sendStatus();
}

/** {"cmd":"clean","volume":50} */
void cmdClean(JsonDocument& doc) {
    float vol = doc["volume"] | 50.0f;

    sysState = STATE_CLEANING;
    DBGLN("[ CLEAN ] Iniciando limpieza...");

    bool ok = dispensar(PUMP_CLEAN, vol);

    sysState = ok ? STATE_IDLE : STATE_ERROR;
    DBGLN(ok ? "[ CLEAN ] Completado." : "[ CLEAN ] Error durante limpieza.");
    sendStatus();
}

/** {"cmd":"status"} */
void cmdStatus() {
    sendStatus();
}

// ══════════════════════════════════════════════════════════════════════════════
//  LECTURA SERIAL
// ══════════════════════════════════════════════════════════════════════════════
void processSerial() {
    if (!Serial.available()) return;

    String raw = Serial.readStringUntil('\n');
    raw.trim();
    if (raw.isEmpty()) return;

    DBGF("[ RX ] %s\n", raw.c_str());

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, raw);
    if (err) {
        DBGF("[ JSON ] Error: %s\n", err.c_str());
        sendError("JSON inválido");
        return;
    }

    const char* cmd = doc["cmd"] | "";

    if      (strcmp(cmd, "mix")    == 0) cmdMix(doc);
    else if (strcmp(cmd, "clean")  == 0) cmdClean(doc);
    else if (strcmp(cmd, "status") == 0) cmdStatus();
    else if (strcmp(cmd, "stop")   == 0) {
        emergencyLatched = true;
        stopAllPumps();
        sysState = STATE_EMERGENCY_STOP;
        sendStatus();
    }
    else {
        sendError("Comando desconocido");
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  EMERGENCIA
// ══════════════════════════════════════════════════════════════════════════════
void checkEmergency() {
    if (digitalRead(EMERGENCY_STOP_PIN) == LOW) {   // activo LOW (INPUT_PULLUP)
        if (!emergencyLatched) {
            emergencyLatched = true;
            stopAllPumps();
            sysState = STATE_EMERGENCY_STOP;
            DBGLN("[ EMERGENCIA ] Parada activada por botón físico.");
            sendStatus();
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  RESPUESTAS SERIALES (JSON)
// ══════════════════════════════════════════════════════════════════════════════
void sendStatus() {
    JsonDocument doc;
    doc["state"]        = (uint8_t)sysState;
    doc["emergency"]    = emergencyLatched;

    JsonObject tanks    = doc["tanks"].to<JsonObject>();
    tanks["r"]          = (int)tankLevel[PUMP_RED];
    tanks["g"]          = (int)tankLevel[PUMP_GREEN];
    tanks["b"]          = (int)tankLevel[PUMP_BLUE];
    tanks["w"]          = (int)tankLevel[PUMP_WHITE];
    tanks["clean"]      = (int)tankLevel[PUMP_CLEAN];

    serializeJson(doc, Serial);
    Serial.println();
}

void sendError(const char* msg) {
    JsonDocument doc;
    doc["error"] = msg;
    serializeJson(doc, Serial);
    Serial.println();
}
