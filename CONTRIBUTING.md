# 🤝 Contribuciones

¡Gracias por tu interés en contribuir a este proyecto!

## Cómo Reportar Bugs

Abre un **issue** con:
- Descripción clara del problema
- Pasos para reproducirlo
- Resultado esperado vs. actual
- Versión del firmware/software

**Ejemplo:**
```
Título: WebSocket desconecta después de 5 min
Descripción:
- Conecté a la interfaz
- La conexión se pierde después de ~5 minutos
- El indicador muestra DESCONECTADO en rojo
- Monitor serial no muestra errores
```

## Cómo Sugerir Mejoras

Abre un **issue** con etiqueta `enhancement`:
- Descripción de la mejora
- Caso de uso
- Beneficios esperados

**Ejemplo:**
```
Mejora: Agregar soporte para WiFi Station mode
Caso: Conectar a red doméstica para logging en cloud
```

## Desarrollo Local

### Setup

```bash
# Clonar repositorio
git clone https://github.com/Santi4216/actuadores-mezcladordepintura-iot.git
cd actuadores-mezcladordepintura-iot

# Instalar dependencias Python
pip install -r requirements.txt

# Iniciar servidor
python web_server.py
```

### Estructura de Código

```
├── esp32_firmware/
│   └── main/mezclador_pintura/
│       └── mezclador_v5.ino    ← Lógica principal + máquina de estados
├── web/
│   └── index.html              ← Frontend (comentado y modular)
├── web_server.py               ← Servidor Flask
└── README.md                    ← Documentación
```

### Standards

- **Arduino**: Código comentado, función por cada responsabilidad
- **Python**: PEP 8, variable names descriptivos
- **JavaScript**: camelCase, funciones <100 líneas
- **Commits**: Mensajes claros en inglés

### Testing

Antes de contribuir:
1. Prueba localmente con hardware real
2. Verifica no hay errores de compilación
3. Prueba la interfaz web (F12 console debe estar limpia)
4. Valida con el checklist en README.md

### Pull Request

1. Fork del repositorio
2. Branch descriptivo: `feature/wifi-station-mode`
3. Commits atómicos con mensajes claros
4. PR con descripción de cambios
5. Espera revisión

---

**¡Agradecemos tu contribución!** 🚀
