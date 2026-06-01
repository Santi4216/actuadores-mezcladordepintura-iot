/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║         SISTEMA DE MEZCLADO AUTOMÁTICO DE PINTURA                       ║
 * ║              ESP32-S3-WROOM-1  ·  config.h  v3.0                        ║
 * ╠══════════════════════════════════════════════════════════════════════════╣
 * ║  Hardware : ESP32-S3-WROOM-1 (pinout verificado) + L298N x3             ║
 * ║                                                                          ║
 * ║  Pines DISPONIBLES en el módulo (lado izquierdo, físico 4-20):          ║
 * ║    GPIO4,5,6,7,15,16,17,18,8,3,46,9,10,11,12,13,14                     ║
 * ║  Pines DISPONIBLES en el módulo (lado derecho, físico 4-22):            ║
 * ║    GPIO1,2,42,41,40,39,38,37,36,35,0,45,48,47,21,19,20                 ║
 * ║                                                                          ║
 * ║  PROHIBIDOS (no usar como salida digital):                               ║
 * ║    GPIO0  → BOOT (strapping)                                             ║
 * ║    GPIO3  → JTAG                                                         ║
 * ║    GPIO19 → USB D+                                                       ║
 * ║    GPIO20 → USB D-                                                       ║
 * ║    GPIO45 → VSPI (strapping)                                             ║
 * ║    GPIO46 → LOG  (strapping)                                             ║
 * ║    GPIO43 → UART TX (Serial debug)                                       ║
 * ║    GPIO44 → UART RX (Serial debug)                                       ║
 * ║                                                                          ║
 * ║  ADC SEGURO (WiFi compatible): ADC1 = GPIO1-10                          ║
 * ║  ADC2 (GPIO11-20) INCOMPATIBLE con WiFi activo                          ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 *
 * Wiring L298N → ESP32-S3:
 *   IN1 = HIGH, IN2 = LOW  → bomba ON  (dirección adelante)
 *   IN1 = LOW,  IN2 = LOW  → bomba OFF (freno libre)
 *   EN  → PWM via ledcAttach (5 kHz, 8 bits)
 *
 * MAPA FÍSICO DE PINES USADOS:
 *  ┌─────────────┬──────┬──────────────────────────────┐
 *  │ Función     │ GPIO │ Notas                        │
 *  ├─────────────┼──────┼──────────────────────────────┤
 *  │ ROJA IN1    │  12  │ ADC2_1 / TOUCH12             │
 *  │ ROJA IN2    │  13  │ ADC2_2 / TOUCH13             │
 *  │ ROJA EN~    │  14  │ ADC2_3 / TOUCH14 / PWM       │
 *  ├─────────────┼──────┼──────────────────────────────┤
 *  │ VERDE IN1   │  15  │ ADC2_4 / XTAL_32K_P          │
 *  │ VERDE IN2   │  16  │ ADC2_5 / XTAL_32K_N          │
 *  │ VERDE EN~   │  17  │ ADC2_6 / U1TXD / PWM         │
 *  ├─────────────┼──────┼──────────────────────────────┤
 *  │ AZUL IN1    │  18  │ ADC2_7 / U1RXD               │
 *  │ AZUL IN2    │   8  │ ADC1_7 / TOUCH8              │
 *  │ AZUL EN~    │   9  │ ADC1_8 / TOUCH9 / PWM        │
 *  ├─────────────┼──────┼──────────────────────────────┤
 *  │ BLANCA IN1  │  10  │ ADC1_9 / TOUCH10             │
 *  │ BLANCA IN2  │  11  │ ADC2_0 / TOUCH11             │
 *  │ BLANCA EN~  │   6  │ TOUCH6 / ADC1_5 / PWM        │
 *  ├─────────────┼──────┼──────────────────────────────┤
 *  │ LIMPIEZA IN1│  40  │ GPIO digital / MTDO          │
 *  │ LIMPIEZA IN2│  41  │ GPIO digital / MTDI          │
 *  │ LIMPIEZA EN~│  42  │ GPIO digital / MTMS / PWM    │
 *  ├─────────────┼──────┼──────────────────────────────┤
 *  │ PRESIÓN ADC │   4  │ ADC1_3 / TOUCH4 ✓ WiFi safe │
 *  │ TEMP ADC    │   5  │ ADC1_4 / TOUCH5 ✓ WiFi safe │
 *  │ EMERGENCIA  │   7  │ ADC1_6 / TOUCH7 INPUT_PULLUP│
 *  └─────────────┴──────┴──────────────────────────────┘
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ══════════════════════════════════════════════════════════════════════════════
//  1. COMUNICACIÓN SERIAL
// ══════════════════════════════════════════════════════════════════════════════
#define SERIAL_BAUD             115200      // bps
#define SERIAL_TIMEOUT_MS         5000      // ms — timeout lectura JSON

// ══════════════════════════════════════════════════════════════════════════════
//  2. IDENTIFICADORES DE BOMBA  (índice en arrays)
// ══════════════════════════════════════════════════════════════════════════════
#define PUMP_RED                0
#define PUMP_GREEN              1
#define PUMP_BLUE               2
#define PUMP_WHITE              3
#define PUMP_CLEAN              4
#define PUMP_COUNT              5           // Total — actualizar si se añaden bombas

// ══════════════════════════════════════════════════════════════════════════════
//  3. PINES GPIO — ESP32-S3-WROOM-1  (pinout verificado)
// ══════════════════════════════════════════════════════════════════════════════

// ── Bomba ROJA ────────────────────────────────────────────────────────────────
#define PUMP_RED_IN1            12          // GPIO12 — digital output
#define PUMP_RED_IN2            13          // GPIO13 — digital output
#define PUMP_RED_EN             14          // GPIO14 — PWM LEDC canal 0

// ── Bomba VERDE ───────────────────────────────────────────────────────────────
#define PUMP_GREEN_IN1          15          // GPIO15 — digital output
#define PUMP_GREEN_IN2          16          // GPIO16 — digital output
#define PUMP_GREEN_EN           17          // GPIO17 — PWM LEDC canal 1

// ── Bomba AZUL ────────────────────────────────────────────────────────────────
#define PUMP_BLUE_IN1           18          // GPIO18 — digital output
#define PUMP_BLUE_IN2            8          // GPIO8  — digital output
#define PUMP_BLUE_EN             9          // GPIO9  — PWM LEDC canal 2

// ── Bomba BLANCA ──────────────────────────────────────────────────────────────
#define PUMP_WHITE_IN1          10          // GPIO10 — digital output
#define PUMP_WHITE_IN2          11          // GPIO11 — digital output
#define PUMP_WHITE_EN            6          // GPIO6  — PWM LEDC canal 3

// ── Bomba LIMPIEZA ────────────────────────────────────────────────────────────
#define PUMP_CLEAN_IN1          40          // GPIO40 — digital output
#define PUMP_CLEAN_IN2          41          // GPIO41 — digital output
#define PUMP_CLEAN_EN           42          // GPIO42 — PWM LEDC canal 4

// ── Arrays indexados por PUMP_x ───────────────────────────────────────────────
//  Usar en .ino así:
//    const uint8_t pumpIN1[PUMP_COUNT] = PUMP_IN1_PINS;
#define PUMP_IN1_PINS  { PUMP_RED_IN1, PUMP_GREEN_IN1, PUMP_BLUE_IN1, PUMP_WHITE_IN1, PUMP_CLEAN_IN1 }
#define PUMP_IN2_PINS  { PUMP_RED_IN2, PUMP_GREEN_IN2, PUMP_BLUE_IN2, PUMP_WHITE_IN2, PUMP_CLEAN_IN2 }
#define PUMP_EN_PINS   { PUMP_RED_EN,  PUMP_GREEN_EN,  PUMP_BLUE_EN,  PUMP_WHITE_EN,  PUMP_CLEAN_EN  }

// ══════════════════════════════════════════════════════════════════════════════
//  4. SENSORES  — solo ADC1 (GPIO1-10), compatible con WiFi
// ══════════════════════════════════════════════════════════════════════════════
#define SENSOR_PRESSURE_PIN      4          // ADC1_CH3 / TOUCH4
#define SENSOR_TEMP_PIN          5          // ADC1_CH4 / TOUCH5

#define ADC_VREF_MV           3300          // Tensión referencia (mV)
#define ADC_RESOLUTION_BITS     12          // 12 bits → 0-4095
#define PRESSURE_MV_PER_KPA   10.0f         // Ajustar según datasheet del sensor
#define TEMP_MV_PER_DEGC      10.0f         // Ajustar (ej. LM35 = 10 mV/°C)

// ══════════════════════════════════════════════════════════════════════════════
//  5. PARADA DE EMERGENCIA
// ══════════════════════════════════════════════════════════════════════════════
#define EMERGENCY_STOP_PIN       7          // GPIO7 — INPUT_PULLUP, activo LOW
                                            // ADC1_6 / TOUCH7 — no conflicto

// ══════════════════════════════════════════════════════════════════════════════
//  6. PWM — LEDC  (ESP32-S3: 8 canales high-speed, 0-7)
// ══════════════════════════════════════════════════════════════════════════════
#define PWM_FREQ_HZ           5000          // 5 kHz
#define PWM_RESOLUTION_BITS      8          // 8 bits → 0-255
#define PWM_DUTY_OFF             0
#define PWM_DUTY_MIN            30          // Duty mínimo para vencer inercia
#define PWM_DUTY_MAX           255

#define PWM_CHANNEL_RED          0
#define PWM_CHANNEL_GREEN        1
#define PWM_CHANNEL_BLUE         2
#define PWM_CHANNEL_WHITE        3
#define PWM_CHANNEL_CLEAN        4

#define PWM_CHANNELS  { PWM_CHANNEL_RED, PWM_CHANNEL_GREEN, PWM_CHANNEL_BLUE, \
                        PWM_CHANNEL_WHITE, PWM_CHANNEL_CLEAN }

// ══════════════════════════════════════════════════════════════════════════════
//  7. TIEMPOS DEL SISTEMA  (ms)
// ══════════════════════════════════════════════════════════════════════════════
#define INTERVAL_STATE_UPDATE_MS    1000
#define TIMEOUT_MIXING_MS          30000
#define TIMEOUT_CLEANING_MS        20000
#define DELAY_PUMP_START_MS          200
#define DELAY_PUMP_BETWEEN_MS        100

// ══════════════════════════════════════════════════════════════════════════════
//  8. LÍMITES DE SEGURIDAD
// ══════════════════════════════════════════════════════════════════════════════
#define SAFETY_PUMP_MAX_RUNTIME_MS  10000
#define SAFETY_PUMP_COOLDOWN_MS      3000
#define SAFETY_TANK_MIN_PERCENT          5
#define SAFETY_PRESSURE_MAX_KPA        200
#define SAFETY_TEMP_MAX_DEGC            60

// ══════════════════════════════════════════════════════════════════════════════
//  9. NIVELES INICIALES DE TANQUES  (%)
// ══════════════════════════════════════════════════════════════════════════════
#define TANK_LEVEL_INIT_RED      85
#define TANK_LEVEL_INIT_GREEN    75
#define TANK_LEVEL_INIT_BLUE     90
#define TANK_LEVEL_INIT_WHITE    60
#define TANK_LEVEL_INIT_CLEAN    45

#define TANK_LEVELS_INIT  { TANK_LEVEL_INIT_RED, TANK_LEVEL_INIT_GREEN, \
                            TANK_LEVEL_INIT_BLUE, TANK_LEVEL_INIT_WHITE, \
                            TANK_LEVEL_INIT_CLEAN }

// ══════════════════════════════════════════════════════════════════════════════
// 10. CALIBRACIÓN DE BOMBAS
//     Medir caudal real (mL/s) a PWM_DUTY_MAX antes de usar en producción
// ══════════════════════════════════════════════════════════════════════════════
#define PUMP_RESPONSE_TIME_MS    200
#define PUMP_MIN_DURATION_MS     100

#define FLOW_RATE_RED_ML_S       2.5f
#define FLOW_RATE_GREEN_ML_S     2.5f
#define FLOW_RATE_BLUE_ML_S      2.5f
#define FLOW_RATE_WHITE_ML_S     2.5f
#define FLOW_RATE_CLEAN_ML_S     3.0f

#define FLOW_RATES_ML_S  { FLOW_RATE_RED_ML_S,   FLOW_RATE_GREEN_ML_S, \
                           FLOW_RATE_BLUE_ML_S,   FLOW_RATE_WHITE_ML_S, \
                           FLOW_RATE_CLEAN_ML_S }

// Macro: ms necesarios para dispensar V mL con caudal F mL/s
#define MS_FOR_ML(volume_ml, flow_ml_s) \
    ((uint32_t)(((volume_ml) / (flow_ml_s)) * 1000.0f))

// ══════════════════════════════════════════════════════════════════════════════
// 11. COMUNICACIÓN / BUFFERS
// ══════════════════════════════════════════════════════════════════════════════
#define JSON_BUFFER_SIZE       512
#define JSON_KEY_CMD         "cmd"
#define JSON_KEY_COLOR       "color"
#define JSON_KEY_VOLUME      "volume"
#define JSON_KEY_STATE       "state"
#define JSON_KEY_TANKS       "tanks"

// ══════════════════════════════════════════════════════════════════════════════
// 12. ESTADOS DEL SISTEMA
// ══════════════════════════════════════════════════════════════════════════════
typedef enum : uint8_t {
    STATE_IDLE           = 0,
    STATE_MIXING         = 1,
    STATE_CLEANING       = 2,
    STATE_ERROR          = 3,
    STATE_EMERGENCY_STOP = 4
} SystemState_t;

// ══════════════════════════════════════════════════════════════════════════════
// 13. DEBUG
// ══════════════════════════════════════════════════════════════════════════════
#define DEBUG_MODE                          // Comentar para desactivar

#ifdef DEBUG_MODE
    #define DBG(x)      Serial.print(x)
    #define DBGLN(x)    Serial.println(x)
    #define DBGF(...)   Serial.printf(__VA_ARGS__)
#else
    #define DBG(x)
    #define DBGLN(x)
    #define DBGF(...)
#endif

#endif // CONFIG_H
