# 🎨 Mezclador Automático de Pintura — ESP32 IoT

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino IDE](https://img.shields.io/badge/Arduino%20IDE-v2.0%2B-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/en/software)
[![Python](https://img.shields.io/badge/Python-3.8%2B-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM%2032-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/)
[![WebSocket](https://img.shields.io/badge/WebSocket-<50ms%20latency-brightgreen)](#)
[![Status](https://img.shields.io/badge/Status-Production%20Ready-success)](#)

**Sistema IoT completo para dosificación y mezcla automática de pigmentos, controlado en tiempo real desde una interfaz web sin dependencias externas.**

[Ver demo](#-interfaz-web) · [Quick Start](#-quick-start) · [Arquitectura](#-arquitectura) · [GPIO Map](#-mapa-de-pines)

</div>

---

## 📋 Tabla de contenidos

- [¿Qué hace este proyecto?](#-qué-hace-este-proyecto)
- [Stack tecnológico](#-stack-tecnológico)
- [Arquitectura](#-arquitectura)
- [Quick Start](#-quick-start)
- [Interfaz web](#-interfaz-web)
- [Protocolo WebSocket](#-protocolo-websocket)
- [Mapa de pines](#-mapa-de-pines)
- [Calibración de bombas](#-calibración-de-bombas)
- [Máquina de estados](#-máquina-de-estados)
- [Troubleshooting](#-troubleshooting)
- [Roadmap](#-roadmap)

---

## 🔍 ¿Qué hace este proyecto?

Un ESP32 WROOM 32 controla **6 bombas peristálticas** y un agitador para dosificar pigmentos (Rojo, Amarillo, Azul, Blanco) con precisión de ±5%. El operario configura los volúmenes desde una interfaz web responsive —sin apps, sin cables— y el ESP32 ejecuta la secuencia de forma completamente autónoma.

```
Operario ingresa volúmenes (R/G/B/W en mL)
         ↓
 Interfaz web envía JSON por WebSocket
         ↓
 ESP32 calcula tiempos y ejecuta secuencia
         ↓
 Bombas dosifican pigmento en orden
         ↓
 Extracción automática del volumen total
         ↓
 Mezcla lista ✓ — estado visible en tiempo real
```

### Características principales

| Feature | Descripción |
|---|---|
| 🎨 **Mezcla automática** | 4 colores independientes, tiempos calculados por calibración |
| 💧 **Limpieza manual** | Agua + agitador sin timeout, el operario decide cuándo parar |
| ⚙️ **Calibración dinámica** | 3 pulsos por bomba, promedio con detección de outliers, guardado en NVS |
| 📋 **Historial** | Recetas guardadas con ID único, operario y timestamp |
| 🔴 **Emergencia** | Parada inmediata de todos los actuadores con un botón |
| 🔌 **Sin cables** | Control por WiFi AP; no requiere router externo |

---

## 🛠 Stack tecnológico

| Capa | Tecnología | Detalle |
|---|---|---|
| **Microcontrolador** | ESP32 WROOM 32 | WiFi AP mode, 19 GPIO configurados |
| **Firmware** | Arduino C++ | No-blocking, `millis()`-based, sin `delay()` |
| **Comunicación** | WebSocket (puerto 81) | Latencia <50 ms, protocolo JSON |
| **Persistencia** | NVS (ESP32) | Calibración sobrevive reinicios |
| **Frontend** | HTML5 / CSS3 / JS vanilla | 0 dependencias externas, responsive |
| **Backend** | Flask (Python 3.8+) | Sirve el HTML estático |
| **Librerías Arduino** | `WebSocketsServer`, `ArduinoJson`, `Preferences` | Versiones v2.4+, v6.19+, built-in |

---

## 🏗 Arquitectura

```
┌─────────────────────────────────────────────────────────┐
│                    Navegador del operario                │
│  ┌─────────────────────────────────────────────────┐    │
│  │           Interfaz Web (index.html)              │    │
│  │  Conexión · Mezcla · Calibración · Historial    │    │
│  └───────────────────┬─────────────────────────────┘    │
│                      │ HTTP (puerto 5000)                │
└──────────────────────┼──────────────────────────────────┘
                       │
              ┌────────▼────────┐
              │  Flask Server   │  ← python web_server.py
              │  (web_server.py)│
              └─────────────────┘
                       
┌─────────────────────────────────────────────────────────┐
│                    WiFi AP: Mezclador-ESP32              │
│                                                          │
│   PC/Móvil ──WebSocket:81──▶ ESP32 WROOM 32            │
│                               │                          │
│            ┌──────────────────┼──────────────────┐      │
│            │                  │                  │      │
│         Bomba R            Bomba G            Bomba B   │
│         Bomba W          Bomba Agua         Extracción  │
│                            Agitador                     │
└─────────────────────────────────────────────────────────┘
```

---

## 🚀 Quick Start

### Requisitos previos

- [Arduino IDE 2.0+](https://www.arduino.cc/en/software)
- [Python 3.8+](https://www.python.org/downloads/)
- ESP32 WROOM 32 + hardware (ver [mapa de pines](#-mapa-de-pines))
- Navegador Chrome, Firefox o Edge

### 1 — Cargar el firmware

**Instalar el ESP32 core en Arduino IDE:**

```
File → Preferences → Additional Boards Manager URLs:
https://dl.espressif.com/dl/package_esp32_index.json

Tools → Board → Boards Manager → busca "esp32" → instala v2.x
```

**Instalar librerías:**

```
Sketch → Include Library → Manage Libraries
  • WebSocketsServer  (Links2004)     v2.4.0+
  • ArduinoJson       (B. Blanchon)   v6.19.0+
```

**Subir el sketch:**

```
1. Tools → Board → ESP32 Dev Module
2. File → Open → esp32_firmware/mezclador_v5/mezclador_v5.ino
3. Conecta el ESP32 por USB
4. Ctrl+U  (Upload)
```

El Monitor Serial (115200 baud) debe mostrar:

```
[WiFi AP] Mezclador-ESP32 | IP: 192.168.4.1
[WebSocket] Escuchando en puerto 81
Firmware listo — esperando conexión...
```

### 2 — Iniciar el servidor web

```bash
git clone https://github.com/Santi4216/actuadores-mezcladordepintura-iot.git
cd actuadores-mezcladordepintura-iot

pip install -r requirements.txt
python web_server.py
```

Salida esperada:

```
🚀 Servidor web v5 iniciando...
📱 Accede a: http://localhost:5000
 * Running on http://0.0.0.0:5000
```

### 3 — Conectar y mezclar

```
1. Conecta tu PC/teléfono al WiFi: Mezclador-ESP32
   Contraseña: mezclador123

2. Abre el navegador: http://localhost:5000

3. Pestaña "Conexión" → click CONECTAR
   Indicador: 🔴 DESCONECTADO → 🟢 CONECTADO

4. Pestaña "Mezcla" → ingresa volúmenes → INICIAR
```

---

## 💻 Interfaz web

La interfaz tiene **7 pestañas funcionales** servidas como un único archivo `index.html` sin frameworks:

| Pestaña | Función |
|---|---|
| **Conexión** | Estado WiFi/WebSocket, conectar/desconectar |
| **Mezcla** | Configurar volúmenes R/G/B/W e iniciar proceso |
| **Individual** | Probar cada bomba de forma independiente |
| **Limpieza** | Control manual de bomba de agua + agitador |
| **Calibración** | 3 pulsos por bomba, detección de outliers, guardar en NVS |
| **Historial** | Recetas guardadas con ID, operario y timestamp |
| **Consola** | Debug WebSocket en tiempo real (JSON crudo) |

---

## 📡 Protocolo WebSocket

Todos los mensajes son JSON. El ESP32 escucha en `ws://192.168.4.1:81`.

### Comandos (cliente → ESP32)

```json
// Iniciar mezcla
{ "cmd": "mix", "r": 50, "g": 0, "b": 30, "w": 20 }

// Extracción manual
{ "cmd": "extract", "volume": 100 }

// Limpieza manual
{ "cmd": "cleanOn" }
{ "cmd": "cleanOff" }

// Calibración
{ "cmd": "calibPulse", "pump": 0 }
{ "cmd": "setCalib",   "pump": 0, "mlPerSec": 1.667 }

// Control
{ "cmd": "stop" }
{ "cmd": "reset" }
{ "cmd": "status" }
```

### Respuesta de estado (ESP32 → cliente)

```json
{
  "state":       1,
  "activePump":  "rojo",
  "step":        2,
  "totalSteps":  4,
  "progress":    45,
  "remainingMs": 8200,
  "emergency":   false,
  "cleanActive": false,
  "totalMixMl":  100,
  "flowRates":   [1.667, 1.667, 1.667, 1.667, 1.667, 2.0],
  "tanks":       { "r": 100, "g": 100, "b": 100, "w": 100, "clean": 100 }
}
```

### Estados del sistema (`state`)

| Valor | Estado | Descripción |
|---|---|---|
| `0` | `IDLE` | Listo para recibir comandos |
| `1` | `MIXING` | Secuencia de mezcla activa |
| `2` | `EXTRACTING` | Extracción automática |
| `3` | `CLEANING` | Limpieza manual activa |
| `4` | `EMERGENCY` | Parada de emergencia |
| `5` | `READY` | Mezcla completada exitosamente |
| `6` | `CALIBRATING` | Pulso de calibración activo |

---

## 📌 Mapa de pines

| Bomba | IN1 | IN2 | EN (PWM) | Notas |
|---|---|---|---|---|
| Rojo | GPIO 12 | GPIO 13 | GPIO 14 | LEDC canal 0 |
| Amarillo | GPIO 15 | GPIO 16 | GPIO 17 | LEDC canal 1 |
| Azul | GPIO 18 | GPIO 8 | GPIO 9 | LEDC canal 2 |
| Blanco | GPIO 10 | GPIO 11 | GPIO 6 | LEDC canal 3 |
| Limpieza | GPIO 40 | GPIO 41 | GPIO 42 | Solo manual |
| Extracción | GPIO 4 | GPIO 5 | GPIO 7 | LEDC canal 5 |
| **Agitador** | — | — | **GPIO 2** | Digital simple |

> Todos los canales PWM usan LEDC a 5 kHz / 8 bits. PWM fijo en 255 (máxima velocidad); el volumen se controla por tiempo de activación según la calibración.

---

## ⚙️ Calibración de bombas

El sistema usa calibración volumétrica real para garantizar precisión de ±5%.

**Proceso por bomba:**

1. Abrir pestaña **Calibración**
2. Hacer clic en **▶ PULSO** (la bomba corre 10 segundos)
3. Medir los mL dispensados con una probeta e ingresar el valor
4. Repetir 3 veces (el sistema promedia y detecta outliers)
5. Hacer clic en **💾 GUARDAR EN ESP32**

Los valores se persisten en la NVS del ESP32 (`namespace: "calib"`) y sobreviven a reinicios y cortes de energía.

**Valores por defecto:** 1.667 mL/s para todas las bombas, 2.0 mL/s para extracción.

---

## 🔄 Máquina de estados

```
              ┌─────────────────────────────────────┐
              │              S_IDLE (0)              │◀──────────┐
              └──────────────┬──────────────────────┘           │
                   cmd:mix   │          cmd:cleanOn              │
                             ▼                                   │
              ┌──────────────────────────┐                       │
              │       S_MIXING (1)       │                       │
              │  Bomba R → G → B → W    │                       │
              └──────────────┬───────────┘                       │
                             │ secuencia completa                │
                             ▼                                   │
              ┌──────────────────────────┐                       │
              │     S_EXTRACTING (2)     │                       │
              └──────────────┬───────────┘                       │
                             │                                   │
                             ▼                       cmd:reset   │
              ┌──────────────────────────┐                       │
              │       S_READY (5)        │                       │
              └──────────────────────────┘                       │
                                                                 │
    cmd:stop → S_EMERGENCY (4) ─────────────────────────────────┘
    cmd:cleanOn → S_CLEANING (3) ←→ cmd:cleanOff → S_IDLE
    cmd:calibPulse → S_CALIBRATING (6) → S_IDLE (auto)
```

---

## 🐛 Troubleshooting

<details>
<summary><strong>ESP32 no aparece en puertos del IDE</strong></summary>

- Verifica que el cable USB transfiere datos (no solo carga)
- Instala el driver CH340: https://ch340.github.io/
- Prueba otro puerto USB

</details>

<details>
<summary><strong>WiFi "Mezclador-ESP32" no aparece</strong></summary>

- Verifica en Monitor Serial que el upload fue exitoso
- Reinicia el ESP32 (botón EN o desconectar/conectar USB)
- Mantén presionado BOOT 5 s para recovery mode

</details>

<details>
<summary><strong>WebSocket no conecta (indicador rojo permanente)</strong></summary>

```bash
# Verifica conectividad
ping 192.168.4.1

# Verifica puerto abierto
netstat -an | findstr 192.168.4.1
```

Abre F12 en el navegador y revisa la consola para ver el error exacto del WebSocket.

</details>

<details>
<summary><strong>Bomba no se activa</strong></summary>

- Verifica alimentación del motor (GND/VCC del driver)
- Confirma que el GPIO en el `.ino` coincide con el hardware físico
- Usa "Prueba Individual" en la interfaz para aislar el problema
- Revisa que el relé o driver L298N no esté dañado

</details>

<details>
<summary><strong>Calibración no persiste entre reinicios</strong></summary>

- Confirma que hiciste clic en **💾 GUARDAR EN ESP32** (no solo registrar el pulso)
- Monitor Serial debe mostrar: `[NVS] Calibración guardada en NVS`
- Si la NVS está llena, borra el namespace: `prefs.clear()` (temporal, en el `.ino`)

</details>

---

## 📁 Estructura del repositorio

```
actuadores-mezcladordepintura-iot/
│
├── esp32_firmware/
│   └── mezclador_v5/
│       └── mezclador_v5.ino       ← Firmware principal (~800 líneas)
│
├── web/
│   └── index.html                 ← Interfaz web completa (7 pestañas, ~25 KB)
│
├── data/
│   └── historial.json             ← Ejemplo de historial exportado
│
├── web_server.py                  ← Servidor Flask
├── requirements.txt               ← Dependencias Python
├── .env.example                   ← Variables de entorno (template)
├── .gitignore
├── CHANGELOG.md
├── CONTRIBUTING.md
└── README.md
```

---

## 🗺 Roadmap

**v5.0** ✅ Completo y en producción

**v6.0** — Planificado
- [ ] WiFi Station mode (conectar a red doméstica)
- [ ] Cloud logging con ThingSpeak o Firebase
- [ ] Autenticación básica (usuario/contraseña)
- [ ] Generación de reportes PDF
- [ ] API REST + base de datos para historial persistente en servidor
- [ ] App móvil (React Native)

---

## 📄 Licencia

Distribuido bajo la licencia MIT. Ver [`LICENSE`](LICENSE) para más información.

---

## 👨‍💻 Autor

**David Santiago García**  
Proyecto Final — Actuadores y Laboratorio, 6to semestre  
Universidad Militar Nueva Granada · 2026

[![GitHub](https://img.shields.io/badge/GitHub-Santi4216-181717?logo=github)](https://github.com/Santi4216)

---


- [WebSocketsServer](https://github.com/Links2004/arduinoWebSockets) — Markus Sattler
- [ArduinoJson](https://arduinojson.org/) — Benoît Blanchon
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) — Espressif Systems
