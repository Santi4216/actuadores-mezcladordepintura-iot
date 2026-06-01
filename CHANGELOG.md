# Changelog

Todos los cambios notables de este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/),
y este proyecto adhiere a [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [5.0.0] - 2024-06-01

### ✨ Features

- **Firmware completo**: Máquina de estados no-bloqueante para ESP32 WROOM 32
- **6 bombas peristálticas**: Control independiente con PWM (LEDC)
- **Agitador integrado**: GPIO digital para agitación sin timeout
- **WebSocket server**: Latencia <50ms, puerto 81 en WiFi AP
- **Calibración dinámica**: 3-pulse averaging, guardado en NVS (persistente)
- **Interfaz web responsive**: HTML5/CSS3/JS vanilla, 0 dependencias externas
- **7 pestañas funcionales**:
  - Conexión: WiFi status + manual connect
  - Consola: Debug WebSocket con JSON
  - Mezcla: Dosificación automática 4 colores
  - Prueba Individual: Cada bomba por separado
  - Calibración: 3 pulsos con detección de outliers
  - Historial: Recetas guardadas con timestamp
  - (Hidden) Preview: Visualización de color final
- **Limpieza manual**: Bomba agua + agitador, sin timeout
- **Extracción automática**: Tras mezcla completa
- **Emergencia & Reset**: Parada segura de todos los actuadores
- **Historial persistente**: LocalStorage en navegador
- **Servidor Flask**: Minimal, solo sirve HTML estático

### 🔧 Technical

- **Arduino compatibility**: IDE v2.0+, ESP32 Core v2.x
- **GPIO mapping**: 19 pines configurados, sin conflictos UART
- **Non-blocking**: Todos los delays basados en millis(), no delay()
- **JSON protocol**: 9 comandos validados
- **NVS persistence**: Calibración guardada entre reinicios

### 📚 Documentation

- README.md completo (500+ líneas)
- Código fuente comentado (800 líneas)
- Troubleshooting detallado
- Especificaciones técnicas
- Mapeo GPIO exhaustivo

### 🎯 Quality

- ✓ Código compilable sin warnings
- ✓ Máquina de estados validada (7 estados)
- ✓ WebSocket protocol robusto
- ✓ Interfaz probada en Chrome, Firefox, Edge
- ✓ Hardware tested en WROOM 32

---

## Roadmap v6.0+

- [ ] WiFi Station mode (conexión a red doméstica)
- [ ] Cloud logging (ThingSpeak / Firebase)
- [ ] Autenticación básica
- [ ] Estadísticas de uso
- [ ] Mobile app (React Native)
- [ ] Generación de reportes PDF
- [ ] Integración con sistemas ERP

---

## Información de Versiones

**v5.0.0** (Current)
- Versión estable y production-ready
- Tested en hardware real
- Documentación completa

---

**Formato de versión**: `MAJOR.MINOR.PATCH`

- **MAJOR**: Cambios incompatibles
- **MINOR**: Nuevas features compatibles
- **PATCH**: Fixes y mejoras menores

Para más detalles, ver [Semantic Versioning](https://semver.org/).
