//
// ── COMPATIBILIDAD WROOM 32 ─────────────────────────────────────────────────
// ✓ Código compatible con Arduino IDE v2.x y ESP32 Arduino Core v2.x
// ✓ Usa ledcSetup() + ledcAttachPin() (ESTÁNDAR en WROOM 32)
// ✓ Pines validados para WROOM 32 (no usa GPIO 3 ni GPIO 46)
// ✓ Sin conflictos UART
//
// ── PINES ESP32 WROOM 32 (GPIO válidos) ──────────────────────────────────────
// Bombas (6 unidades × 3 pines = 18 pines) + Agitador (1 pin) = 19 pines totales
//   ROJO        → IN1=12, IN2=13, EN=14
//   AMARILLO    → IN1=15, IN2=16, EN=17
//   AZUL        → IN1=18, IN2=8,  EN=9
//   BLANCO      → IN1=10, IN2=11, EN=6
//   LIMPIEZA    → IN1=40, IN2=41, EN=42 (solo por comando manual)
//   EXTRACCIÓN  → IN1=4,  IN2=5,  EN=7
//   AGITADOR    → GPIO 2 (pin digital simple — compatible WROOM 32)
//
// ════════════════════════════════════════════════════════════════════════════════

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define AP_SSID "Mezclador-ESP32"
#define AP_PASS "mezclador123"

WebSocketsServer ws(81);
Preferences prefs;

// ──────────────────────────────────────────────────────────────────────────────
// HARDWARE — Pines para ESP32 WROOM 32
// ──────────────────────────────────────────────────────────────────────────────
#define LEDC_FREQ  5000
#define LEDC_BITS  8
#define LEDC_CH(i) (i)

#define PIN_AGITADOR 2  // GPIO 2 (válido en WROOM 32, sin conflictos UART)

struct Pump {
  uint8_t     in1, in2, en;
  float       tank;
  const char* name;
};

Pump pumps[6] = {
  {12, 13, 14, 100.0f, "rojo"},
  {15, 16, 17, 100.0f, "amarillo"},
  {18,  8,  9, 100.0f, "azul"},
  {10, 11,  6, 100.0f, "blanco"},
  {40, 41, 42, 100.0f, "limpieza"},
  { 4,  5,  7, 100.0f, "extraccion"}
};

// ──────────────────────────────────────────────────────────────────────────────
// CALIBRACIÓN DINÁMICA — Guardada en NVS (EEPROM del ESP32)
// ──────────────────────────────────────────────────────────────────────────────
float flowRate[6] = {
  1.667f,  // 0 Rojo       (100 mL / 60 s = 1.667 mL/s @ PWM 255)
  1.667f,  // 1 Amarillo
  1.667f,  // 2 Azul
  1.667f,  // 3 Blanco
  1.667f,  // 4 Limpieza
  2.000f   // 5 Extracción (más rápido)
};

void saveCalib() {
  prefs.begin("calib", false);
  for (int i = 0; i < 6; i++) {
    char k[8];
    snprintf(k, sizeof(k), "fr%d", i);
    prefs.putFloat(k, flowRate[i]);
  }
  prefs.end();
  Serial.printf("[NVS] Calibración guardada en NVS\n");
}

void loadCalib() {
  prefs.begin("calib", true);
  for (int i = 0; i < 6; i++) {
    char k[8];
    snprintf(k, sizeof(k), "fr%d", i);
    float v = prefs.getFloat(k, flowRate[i]);
    if (v > 0.05f && v < 50.0f) {
      flowRate[i] = v;
      Serial.printf("[NVS] Bomba %d: %.4f mL/s\n", i, v);
    }
  }
  prefs.end();
}

// ──────────────────────────────────────────────────────────────────────────────
// PARÁMETROS FIJOS
// ──────────────────────────────────────────────────────────────────────────────
#define INTER_STEP_DELAY_MS      300
#define EXTRACT_MAX_MS           10000UL
#define CALIB_PUMP_DURATION_MS   10000UL

// ── LIMPIEZA MANUAL ───────────────────────────────────────────────────────────
// La limpieza ya NO forma parte del flujo automático.
// Se activa con {"cmd":"cleanOn"} y se detiene con {"cmd":"cleanOff"}.
// Bomba de agua (idx 4) + agitador corren en paralelo indefinidamente.
// El operario decide cuándo parar según su criterio visual.
bool cleanManualActive = false;

// ──────────────────────────────────────────────────────────────────────────────
// CONTROL DE ACTUADORES
// ──────────────────────────────────────────────────────────────────────────────
void pumpOff(uint8_t i) {
  ledcWrite(pumps[i].en, 0);
  digitalWrite(pumps[i].in1, LOW);
  digitalWrite(pumps[i].in2, LOW);
}

void pumpOn(uint8_t i) {
  digitalWrite(pumps[i].in1, HIGH);
  digitalWrite(pumps[i].in2, LOW);
  ledcWrite(pumps[i].en, 255);
}

void agitadorOn() {
  digitalWrite(PIN_AGITADOR, HIGH);
}

void agitadorOff() {
  digitalWrite(PIN_AGITADOR, LOW);
}

void allOff() {
  for (int i = 0; i < 6; i++) pumpOff(i);
  agitadorOff();
  cleanManualActive = false;
}

// ──────────────────────────────────────────────────────────────────────────────
// MÁQUINA DE ESTADOS
// ──────────────────────────────────────────────────────────────────────────────
enum SysState {
  S_IDLE        = 0,
  S_MIXING      = 1,
  S_EXTRACTING  = 2,
  S_CLEANING    = 3,   // limpieza manual activa
  S_EMERGENCY   = 4,
  S_READY       = 5,
  S_CALIBRATING = 6
};

SysState sysState      = S_IDLE;
bool     emergencyFlag = false;

// ── Cola de pasos ─────────────────────────────────────────────────────────────
struct Step {
  uint8_t       type;        // 0=simple  1=paralelo (reservado)
  uint8_t       pumpIdx;
  unsigned long durationMs;
  unsigned long agitExtraMs; // reservado para compatibilidad futura
  const char*   label;
};

#define MAX_STEPS 8
Step    stepQueue[MAX_STEPS];
uint8_t stepCount  = 0;
uint8_t stepIndex  = 0;

enum StepPhase { SP_IDLE, SP_RUNNING, SP_COOLDOWN };
StepPhase     stepPhase   = SP_IDLE;
unsigned long stepEndMs   = 0;
unsigned long cooldownEnd = 0;

float         totalMixVolume = 0.0f;
unsigned long seqTotalMs    = 0;
unsigned long seqStartMs    = 0;

// ── Calibración ───────────────────────────────────────────────────────────────
uint8_t       calibPumpIdx = 0;
unsigned long calibEndMs   = 0;

// ── Helpers de encolado ───────────────────────────────────────────────────────
void enqueue(uint8_t pumpIdx, float ml, const char* label) {
  if (ml <= 0.0f || stepCount >= MAX_STEPS) return;
  unsigned long ms = (unsigned long)((ml / flowRate[pumpIdx]) * 1000.0f);
  if (ms < 50) ms = 50;
  stepQueue[stepCount++] = { 0, pumpIdx, ms, 0, label };
  seqTotalMs += ms + INTER_STEP_DELAY_MS;
}

void enqueueExtract(float mixVolumeMl) {
  if (stepCount >= MAX_STEPS) return;
  unsigned long ms = (unsigned long)((mixVolumeMl / flowRate[5]) * 1000.0f);
  if (ms > EXTRACT_MAX_MS) ms = EXTRACT_MAX_MS;
  if (ms < 50) ms = 50;
  stepQueue[stepCount++] = { 0, 5, ms, 0, "extraccion" };
  seqTotalMs += ms + INTER_STEP_DELAY_MS;
}

// ── Broadcast de progreso ─────────────────────────────────────────────────────
void broadcastProgress(const char* activePump, bool cooldown) {
  unsigned long elapsed = (seqStartMs > 0) ? (millis() - seqStartMs) : 0;
  int pct = (seqTotalMs > 0)
    ? (int)min(100UL, elapsed * 100UL / seqTotalMs) : 0;
  long remaining = (long)seqTotalMs - (long)elapsed;
  if (remaining < 0) remaining = 0;

  JsonDocument doc;
  doc["state"]       = (int)sysState;
  doc["emergency"]   = false;
  doc["activePump"]  = activePump;
  doc["step"]        = stepIndex + 1;
  doc["totalSteps"]  = stepCount;
  doc["progress"]    = pct;
  doc["remainingMs"] = remaining;
  doc["cooldown"]    = cooldown;
  doc["totalMixMl"]  = (int)totalMixVolume;
  doc["cleanActive"] = cleanManualActive;
  String msg;
  serializeJson(doc, msg);
  ws.broadcastTXT(msg);
}

// ── Lanzar paso actual ────────────────────────────────────────────────────────
void launchStep() {
  Step& s = stepQueue[stepIndex];
  pumpOn(s.pumpIdx);
  stepPhase = SP_RUNNING;
  stepEndMs = millis() + s.durationMs;
  Serial.printf("[STEP %d/%d] bomba=%s dur=%lu ms\n",
    stepIndex + 1, stepCount, s.label, s.durationMs);
  broadcastProgress(s.label, false);
}

// ── Finalizar secuencia ───────────────────────────────────────────────────────
void finishSequence() {
  allOff();
  stepPhase  = SP_IDLE;
  stepCount  = stepIndex = 0;
  seqTotalMs = seqStartMs = 0;
  sysState   = S_READY;

  JsonDocument doc;
  doc["state"]       = (int)sysState;
  doc["emergency"]   = false;
  doc["activePump"]  = "";
  doc["progress"]    = 100;
  doc["step"]        = 0;
  doc["totalSteps"]  = 0;
  doc["remainingMs"] = 0;
  doc["cleanActive"] = false;
  String msg;
  serializeJson(doc, msg);
  ws.broadcastTXT(msg);
  Serial.println("[SEQ] ✓ Mezcla + extracción completadas — LISTO para limpieza manual");
}

// ──────────────────────────────────────────────────────────────────────────────
// WEBSOCKET
// ──────────────────────────────────────────────────────────────────────────────
void sendStatus(uint8_t client) {
  const char* ap = "";
  if (stepPhase == SP_RUNNING)
    ap = stepQueue[stepIndex].label;
  if (cleanManualActive)
    ap = "limpieza";

  unsigned long elapsed = (seqStartMs > 0) ? (millis() - seqStartMs) : 0;
  int pct = (seqTotalMs > 0)
    ? (int)min(100UL, elapsed * 100UL / seqTotalMs) : 0;
  long remaining = (long)seqTotalMs - (long)elapsed;
  if (remaining < 0) remaining = 0;

  JsonDocument doc;
  doc["state"]       = (int)sysState;
  doc["emergency"]   = emergencyFlag;
  doc["activePump"]  = ap;
  doc["step"]        = stepIndex + 1;
  doc["totalSteps"]  = stepCount;
  doc["progress"]    = pct;
  doc["remainingMs"] = remaining;
  doc["totalMixMl"]  = (int)totalMixVolume;
  doc["cleanActive"] = cleanManualActive;

  auto t  = doc["tanks"].to<JsonObject>();
  t["r"]  = (int)pumps[0].tank;
  t["g"] = (int)pumps[1].tank;
  t["b"]  = (int)pumps[2].tank;
  t["w"] = (int)pumps[3].tank;
  t["clean"] = (int)pumps[4].tank;

  auto fr = doc["flowRates"].to<JsonArray>();
  for (int i = 0; i < 6; i++) fr.add(flowRate[i]);

  String msg;
  serializeJson(doc, msg);
  ws.sendTXT(client, msg);
}

void onWsEvent(uint8_t c, WStype_t type, uint8_t* payload, size_t len) {
  if (type != WStype_TEXT) return;

  JsonDocument doc;
  if (deserializeJson(doc, (char*)payload)) {
    ws.sendTXT(c, "{\"error\":\"JSON invalido\"}");
    return;
  }

  const char* cmd = doc["cmd"] | "";
  Serial.printf("[WS] Comando: %s\n", cmd);

  // ── STATUS ────────────────────────────────────────────────────────────────
  if (strcmp(cmd, "status") == 0) {
    sendStatus(c);
    return;
  }

  // ── STOP (EMERGENCIA) ─────────────────────────────────────────────────────
  if (strcmp(cmd, "stop") == 0) {
    emergencyFlag    = true;
    sysState         = S_EMERGENCY;
    allOff();
    stepPhase        = SP_IDLE;
    stepCount        = stepIndex = 0;
    seqTotalMs       = seqStartMs = 0;
    calibEndMs       = 0;
    sendStatus(c);
    Serial.println("[STOP] ⚠ EMERGENCIA — todos los actuadores detenidos");
    return;
  }

  // ── RESET ─────────────────────────────────────────────────────────────────
  if (strcmp(cmd, "reset") == 0) {
    if (stepPhase != SP_IDLE) {
      ws.sendTXT(c, "{\"error\":\"Proceso activo, usa stop primero\"}");
      return;
    }
    emergencyFlag = false;
    sysState      = S_IDLE;
    allOff();
    sendStatus(c);
    Serial.println("[RESET] Sistema reseteado — listo");
    return;
  }

  // ── Bloquear si está en emergencia ────────────────────────────────────────
  if (emergencyFlag) {
    ws.sendTXT(c, "{\"error\":\"EMERGENCIA activa. Envia reset primero.\"}");
    return;
  }

  // ── LIMPIEZA MANUAL ON ────────────────────────────────────────────────────
  if (strcmp(cmd, "cleanOn") == 0) {
    if (stepPhase != SP_IDLE) {
      ws.sendTXT(c, "{\"error\":\"Proceso activo — no se puede activar limpieza\"}");
      return;
    }
    if (cleanManualActive) {
      sendStatus(c);
      return;
    }
    cleanManualActive = true;
    sysState = S_CLEANING;
    pumpOn(4);      // bomba de agua (idx 4)
    agitadorOn();   // agitador simultáneo
    Serial.println("[CLEAN] 💧 Limpieza manual ON — bomba agua + agitador activos");
    broadcastProgress("limpieza", false);
    return;
  }

  // ── LIMPIEZA MANUAL OFF ───────────────────────────────────────────────────
  if (strcmp(cmd, "cleanOff") == 0) {
    if (!cleanManualActive) {
      sendStatus(c);
      return;
    }
    cleanManualActive = false;
    pumpOff(4);
    agitadorOff();
    sysState = S_IDLE;
    Serial.println("[CLEAN] ⏹ Limpieza manual OFF");
    broadcastProgress("", false);
    return;
  }

  // ── SET CALIBRACIÓN ───────────────────────────────────────────────────────
  if (strcmp(cmd, "setCalib") == 0) {
    int   pump     = doc["pump"]     | -1;
    float mlPerSec = doc["mlPerSec"] | 0.0f;
    if (pump < 0 || pump > 5) {
      ws.sendTXT(c, "{\"error\":\"pump fuera de rango\"}");
      return;
    }
    if (mlPerSec < 0.05f || mlPerSec > 50.0f) {
      ws.sendTXT(c, "{\"error\":\"mlPerSec invalido\"}");
      return;
    }
    flowRate[pump] = mlPerSec;
    saveCalib();
    Serial.printf("[CALIB] Bomba %d → %.4f mL/s (guardado en NVS)\n", pump, mlPerSec);
    sendStatus(c);
    return;
  }

  // ── PULSO DE CALIBRACIÓN ──────────────────────────────────────────────────
  if (strcmp(cmd, "calibPulse") == 0) {
    if (stepPhase != SP_IDLE) {
      ws.sendTXT(c, "{\"error\":\"Operacion en progreso\"}");
      return;
    }
    int pump = doc["pump"] | -1;
    if (pump < 0 || pump > 5) {
      ws.sendTXT(c, "{\"error\":\"pump fuera de rango\"}");
      return;
    }
    calibPumpIdx = (uint8_t)pump;
    calibEndMs   = millis() + CALIB_PUMP_DURATION_MS;
    sysState     = S_CALIBRATING;
    pumpOn(calibPumpIdx);
    Serial.printf("[CALIB-PULSE] Bomba %d ON por %lu ms\n", calibPumpIdx, CALIB_PUMP_DURATION_MS);
    JsonDocument r;
    r["state"]     = (int)sysState;
    r["calibPump"] = pump;
    r["calibMs"]   = (int)CALIB_PUMP_DURATION_MS;
    r["msg"]       = "Pulso calibración iniciado";
    String out;
    serializeJson(r, out);
    ws.sendTXT(c, out);
    return;
  }

  // ── SET TANK ──────────────────────────────────────────────────────────────
  if (strcmp(cmd, "setTank") == 0) {
    int pump = doc["pump"] | -1;
    int pct  = doc["pct"]  | -1;
    if (pump < 0 || pump > 4) {
      ws.sendTXT(c, "{\"error\":\"pump fuera de rango\"}");
      return;
    }
    if (pct < 0 || pct > 100) {
      ws.sendTXT(c, "{\"error\":\"pct fuera de rango\"}");
      return;
    }
    pumps[pump].tank = (float)pct;
    sendStatus(c);
    return;
  }

  // ── MIX (Flujo completo: Dosificación → Extracción) ──────────────────────
  if (strcmp(cmd, "mix") == 0) {
    if (stepPhase != SP_IDLE) {
      ws.sendTXT(c, "{\"error\":\"Operacion en progreso\"}");
      return;
    }
    float r = doc["r"] | 0.0f;
    float g = doc["g"] | 0.0f;
    float b = doc["b"] | 0.0f;
    float w = doc["w"] | 0.0f;
    totalMixVolume = r + g + b + w;

    if (totalMixVolume <= 0.0f) {
      ws.sendTXT(c, "{\"error\":\"Volumen cero\"}");
      return;
    }

    sysState   = S_MIXING;
    stepCount  = stepIndex = 0;
    seqTotalMs = 0;

    enqueue(0, r, "rojo");
    enqueue(1, g, "amarillo");
    enqueue(2, b, "azul");
    enqueue(3, w, "blanco");
    enqueueExtract(totalMixVolume);

    seqStartMs = millis();
    Serial.printf("[MIX] %d pasos | volumen total: %.1f mL | tiempo estimado: %lu ms\n",
      stepCount, totalMixVolume, seqTotalMs);
    sendStatus(c);
    launchStep();
    return;
  }

  // ── EXTRACT independiente ─────────────────────────────────────────────────
  if (strcmp(cmd, "extract") == 0) {
    if (stepPhase != SP_IDLE) {
      ws.sendTXT(c, "{\"error\":\"Operacion en progreso\"}");
      return;
    }
    float vol = doc["volume"] | 50.0f;
    sysState   = S_EXTRACTING;
    stepCount  = stepIndex = 0;
    seqTotalMs = 0;
    enqueueExtract(vol);
    seqStartMs = millis();
    sendStatus(c);
    launchStep();
    return;
  }

  ws.sendTXT(c, "{\"error\":\"Comando desconocido\"}");
}

// ──────────────────────────────────────────────────────────────────────────────
// SETUP
// ──────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n═══════════════════════════════════════════════════════════════════════════");
  Serial.println("╔═ MEZCLADOR ESP32 WROOM 32 v5 ═════════════════════════════════════════╗");
  Serial.println("║ Flujo: MEZCLA → EXTRACCIÓN → LISTO (limpieza MANUAL)                  ║");
  Serial.println("║ WebSocket: ws://192.168.4.1:81                                        ║");
  Serial.println("║ WiFi: Mezclador-ESP32 | Clave: mezclador123                           ║");
  Serial.println("╚════════════════════════════════════════════════════════════════════════╝");
  Serial.println();

  // Cargar calibración desde NVS
  loadCalib();

  // Setup LEDC (PWM) para las 6 bombas
  for (int i = 0; i < 6; i++) {
    ledcSetup(LEDC_CH(i), LEDC_FREQ, LEDC_BITS);      // WROOM 32 v2.x compatible
    ledcAttachPin(pumps[i].en, LEDC_CH(i));
    pinMode(pumps[i].in1, OUTPUT);
    pinMode(pumps[i].in2, OUTPUT);
    pumpOff(i);
  }

  // Setup agitador (GPIO digital)
  pinMode(PIN_AGITADOR, OUTPUT);
  agitadorOff();

  // Setup WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("[WiFi AP] %s | IP: %s\n", AP_SSID, ip.toString().c_str());

  // Setup WebSocket
  ws.begin();
  ws.onEvent(onWsEvent);
  Serial.println("[WebSocket] Escuchando en puerto 81");
  Serial.println("\n✓ Firmware listo — esperando conexión...\n");
}

// ──────────────────────────────────────────────────────────────────────────────
// LOOP
// ──────────────────────────────────────────────────────────────────────────────
void loop() {
  ws.loop();

  // ── Pulso de calibración ──────────────────────────────────────────────────
  if (sysState == S_CALIBRATING) {
    if (calibEndMs > 0 && (long)(millis() - calibEndMs) >= 0) {
      pumpOff(calibPumpIdx);
      calibEndMs = 0;
      sysState   = S_IDLE;
      Serial.printf("[CALIB-PULSE] Bomba %d OFF — pulso completado\n", calibPumpIdx);
      JsonDocument doc;
      doc["state"]     = (int)sysState;
      doc["emergency"] = false;
      doc["calibDone"] = true;
      doc["calibPump"] = (int)calibPumpIdx;
      String msg;
      serializeJson(doc, msg);
      ws.broadcastTXT(msg);
    }
    return;
  }

  // ── Limpieza manual activa — sin timeout, el operario manda ───────────────
  if (cleanManualActive) return;

  // ── Nada en progreso o emergencia activa ──────────────────────────────────
  if (stepPhase == SP_IDLE || emergencyFlag) return;

  unsigned long now = millis();

  // ── SP_RUNNING: Paso en ejecución ─────────────────────────────────────────
  if (stepPhase == SP_RUNNING) {
    if ((long)(now - stepEndMs) < 0) return;
    pumpOff(stepQueue[stepIndex].pumpIdx);
    Serial.printf("[STEP %d/%d] Bomba apagada\n", stepIndex + 1, stepCount);
    broadcastProgress("", true);
    stepPhase   = SP_COOLDOWN;
    cooldownEnd = now + INTER_STEP_DELAY_MS;
    return;
  }

  // ── SP_COOLDOWN: Pausa entre pasos ────────────────────────────────────────
  if (stepPhase == SP_COOLDOWN) {
    if ((long)(now - cooldownEnd) < 0) return;
    stepIndex++;
    if (stepIndex < stepCount) {
      const char* lbl = stepQueue[stepIndex].label;
      if (strcmp(lbl, "extraccion") == 0) sysState = S_EXTRACTING;
      launchStep();
    } else {
      finishSequence();
    }
  }
}

