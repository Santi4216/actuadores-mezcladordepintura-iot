
# 🤖 FIRMWARE ESP32 - SISTEMA DE MEZCLADO DE PINTURA

## 📋 Descripción General

Firmware profesional para ESP32-WROOM-32 que controla un sistema automático de mezclado de pintura. Incluye:

- ✅ Control de 5 válvulas (Rojo, Verde, Azul, Blanco, Limpieza)
- ✅ Comunicación JSON por Serial USB
- ✅ Protocolo robusto con manejo de errores
- ✅ State Machine para gestión de estados
- ✅ Fallover: comandos directos si la interfaz PyQt6 falla
- ✅ Sensores de presión y temperatura
- ✅ Parada de emergencia
- ✅ Logging y debugging

---

## 🔧 Instalación

### Requisitos
- **Arduino IDE 2.0+** (o VS Code con PlatformIO)
- **Placa ESP32-WROOM-32**
- **Librería ArduinoJson** (`Sketch > Manage Libraries > ArduinoJson`)

### Pasos

1. **Abrir Arduino IDE 2.0**
2. **Instalar soporte para ESP32:**
   - Ir a `File > Preferences`
   - En "Additional Boards Manager URLs" agregar:
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Ir a `Tools > Board > Boards Manager`
   - Buscar "esp32" e instalar "esp32 by Espressif Systems"

3. **Instalar librería ArduinoJson:**
   - Ir a `Sketch > Include Library > Manage Libraries`
   - Buscar "ArduinoJson" e instalar versión 6.x

4. **Abrir el firmware:**
   - Abrir `esp32_firmware/main.ino`
   - Incluye automáticamente `config.h`

5. **Seleccionar configuración:**
   - `Tools > Board > ESP32 > ESP32 Dev Module`
   - `Tools > Port > COM# (seleccionar puerto)`
   - `Tools > Upload Speed > 921600`

6. **Compilar y cargar:**
   - Botón: `Sketch > Upload` (o Ctrl+U)
   - Esperar mensaje: "Hard resetting via RTS pin..."

---

## 📡 Protocolo de Comunicación

### Configuración Serial
- **Velocidad:** 115200 bps
- **Datos:** 8 bits
- **Paridad:** Ninguna
- **Parada:** 1 bit
- **Formato:** JSON

### Estructura de Comandos

Todos los comandos son strings JSON que terminan con newline (`\n`).

#### Ejemplo General
```json
{"cmd":"COMMAND_NAME","param1":value1,"param2":value2}
```

---

## 📝 Comandos Disponibles

### 1. **MIX** - Iniciar Mezcla
Abre válvulas proporcional a valores RGB (0-255).

**Formato:**
```json
{"cmd":"MIX","r":255,"g":165,"b":0}
```

**Respuesta OK:**
```json
{"timestamp":1234,"status":"OK","message":"Iniciando mezcla","r":255,"g":165,"b":0,"hex":"0xffa500","state":1}
```

**Respuesta Error:**
```json
{"timestamp":1234,"status":"ERROR","message":"Valores RGB fuera de rango (0-255)","state":0}
```

| Parámetro | Tipo | Rango | Descripción |
|-----------|------|-------|-------------|
| `cmd` | string | - | Debe ser "MIX" |
| `r` | int | 0-255 | Intensidad de rojo |
| `g` | int | 0-255 | Intensidad de verde |
| `b` | int | 0-255 | Intensidad de azul |

---

### 2. **STOP** - Detener Inmediatamente
Cierra todas las válvulas y vuelve a estado IDLE.

**Formato:**
```json
{"cmd":"STOP"}
```

**Respuesta:**
```json
{"timestamp":1234,"status":"OK","message":"Sistema detenido","r":0,"g":0,"b":0,"state":0}
```

---

### 3. **CLEAN** - Limpiar Sistema
Abre válvula de limpieza por 3 segundos.

**Formato:**
```json
{"cmd":"CLEAN"}
```

**Respuesta:**
```json
{"timestamp":1234,"status":"OK","message":"Iniciando limpieza","state":2}
```

---

### 4. **STATUS** - Solicitar Estado
Obtiene información completa del sistema en tiempo real.

**Formato:**
```json
{"cmd":"STATUS"}
```

**Respuesta Completa:**
```json
{
  "timestamp": 45678,
  "state": 0,
  "status": "ACTIVE",
  "esp32": {
    "mac": "AA:BB:CC:DD:EE:FF",
    "uptime": 123,
    "freeHeap": 245000
  },
  "tanks": {
    "red": 85,
    "green": 75,
    "blue": 90,
    "white": 60,
    "cleaning": 45
  },
  "sensors": {
    "pressure": 512,
    "temperature": 25.3
  },
  "current_color": {
    "r": 200,
    "g": 100,
    "b": 50
  },
  "firmware_version": "1.0.0",
  "build_date": "May 19 2026"
}
```

---

### 5. **CALIBRATE** - Calibrar Sistema
Inicia secuencia de calibración.

**Formato:**
```json
{"cmd":"CALIBRATE"}
```

---

### 6. **HELP** - Ver Comandos
Lista todos los comandos disponibles.

**Formato:**
```json
{"cmd":"HELP"}
```

---

### 7. **RESET** - Reiniciar ESP32
Reinicia el microcontrolador.

**Formato:**
```json
{"cmd":"RESET"}
```

---

## 🎯 Estados del Sistema

| Estado | ID | Descripción |
|--------|----|-----------  |
| `STATE_IDLE` | 0 | Sistema en reposo, listo para comandos |
| `STATE_MIXING` | 1 | Actualmente mezclando |
| `STATE_CLEANING` | 2 | Actualmente limpiando |
| `STATE_ERROR` | 3 | Error del sistema |
| `STATE_EMERGENCY_STOP` | 4 | Parada de emergencia activa |

---

## 🔌 Configuración de Pines

| Componente | GPIO | Descripción |
|-----------|------|-------------|
| Válvula ROJO | 19 | Controla apertura/cierre válvula roja |
| Válvula VERDE | 20 | Controla apertura/cierre válvula verde |
| Válvula AZUL | 21 | Controla apertura/cierre válvula azul |
| Válvula BLANCO | 22 | Controla apertura/cierre válvula blanca |
| Válvula LIMPIEZA | 23 | Controla apertura/cierre válvula limpieza |
| Parada Emergencia | 14 | Entrada de parada de emergencia |
| Sensor Presión | A0 | Entrada analógica presión |
| Sensor Temperatura | A1 | Entrada analógica temperatura |

---

## 💻 Prueba por Monitor Serial (Fallover Manual)

Si la interfaz PyQt6 no funciona, puedes enviar comandos directamente:

1. Abrir `Tools > Serial Monitor` en Arduino IDE
2. Configurar velocidad a **115200 bps**
3. Copiar y pegar comandos JSON:

### Ejemplos:

**Mezclar Rojo Puro:**
```json
{"cmd":"MIX","r":255,"g":0,"b":0}
```

**Mezclar Naranja:**
```json
{"cmd":"MIX","r":255,"g":165,"b":0}
```

**Mezclar Púrpura:**
```json
{"cmd":"MIX","r":128,"g":0,"b":128}
```

**Obtener Estado:**
```json
{"cmd":"STATUS"}
```

**Limpiar:**
```json
{"cmd":"CLEAN"}
```

**Parar:**
```json
{"cmd":"STOP"}
```

---

## 🛡️ Seguridad y Manejo de Errores

### Validación de Entrada
- Valores RGB validados (0-255)
- Detección de buffer overflow
- Timeout de mezcla: 30 segundos máximo
- Timeout de limpieza: 20 segundos máximo

### Parada de Emergencia
- Pin de emergencia monitorizado continuamente
- Cierra todas las válvulas inmediatamente
- Envía alerta al host

### Recuperación de Errores
- Respuestas JSON con descripción de error
- Logging detallado en modo DEBUG
- Transición automática a estado IDLE en timeout

---

## 🔧 Configuración Personalizable

Todos los parámetros editables en `config.h`:

```cpp
#define SERIAL_BAUD 115200          // Velocidad de comunicación
#define MIXING_TIMEOUT 30000        // Timeout de mezcla (ms)
#define STATE_UPDATE_INTERVAL 1000  // Envío de estado (ms)
#define DEBUG_MODE true             // Modo depuración
```

---

## 📊 Diagrama de Flujo - State Machine

```
┌─────────────────────────────────────┐
│        IDLE (Estado Inicial)        │
└────┬────────────────────────────────┘
     │
     ├──[MIX cmd]──────► MIXING
     │                      │
     │                      ├──[timeout]─────► IDLE
     │                      └──[STOP cmd]────► IDLE
     │
     ├──[CLEAN cmd]────► CLEANING
     │                      │
     │                      └──[timeout]─────► IDLE
     │
     ├──[STOP cmd]─────► (sin cambio)
     │
     └──[ERROR]─────────► ERROR
                             │
                             └──[RESET cmd]──► IDLE
```

---

## 🧪 Testing Recomendado

1. **Test de Conexión**
   ```json
   {"cmd":"STATUS"}
   ```
   Debe responder con JSON completo

2. **Test RGB Individual**
   ```json
   {"cmd":"MIX","r":255,"g":0,"b":0}
   ```
   Debe abrir solo válvula roja

3. **Test de Timeout**
   ```json
   {"cmd":"MIX","r":255,"g":255,"b":255}
   ```
   Esperar 30s, debe cancelarse automáticamente

4. **Test de Error**
   ```json
   {"cmd":"MIX","r":300,"g":0,"b":0}
   ```
   Debe responder con error

---

## 📝 Notas Importantes

- ⚠️ El ESP32 opera a 3.3V - no conectar 5V directamente
- ⚠️ Las válvulas requieren control de potencia externa (relés o transistores)
- ⚠️ Usar cable USB de calidad para comunicación serial confiable
- ✅ Todos los valores se envían en JSON para facilitar integración
- ✅ Compatible con Python, Node.js, C#, Java, etc.

---

## 🤝 Integración con Python (interfaz_gui.py)

El código Python está configurado para comunicarse automáticamente con este firmware:

```python
BACKEND_URL = "http://localhost:5000"  # API REST (backend)
# El backend traduce comandos Python a JSON del ESP32
```

---

## 📞 Soporte / Debugging

Activar modo DEBUG en `config.h`:
```cpp
#define DEBUG_MODE true
```

Esto añade timestamps y mensajes de depuración en el Monitor Serial.

---

**Versión:** 1.0.0  
**Autor:** Sistema de Mezclado Automático  
**Fecha:** May 19, 2026  
**Licencia:** Proyecto Académico
