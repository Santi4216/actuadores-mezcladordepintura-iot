# ================================================================
# SCRIPT DE CARGA A GITHUB — Mezclador ESP32
# Ejecuta este script en PowerShell desde la carpeta del proyecto
# ================================================================

$projectPath = "D:\Documents\Archivos UMNG\8. OCTAVO SEMESTRE\2. Actuadores y Lab\Teoría\Proyecto Final"
$repoUrl     = "https://github.com/Santi4216/actuadores-mezcladordepintura-iot.git"

Write-Host "📁 Entrando al proyecto..." -ForegroundColor Cyan
Set-Location $projectPath

# 1. Inicializar git si no existe
if (-not (Test-Path ".git")) {
    Write-Host "🔧 Inicializando repositorio git..." -ForegroundColor Yellow
    git init
    git branch -M main
} else {
    Write-Host "✅ Repositorio git ya existe" -ForegroundColor Green
}

# 2. Configurar remote origin
$remoteExists = git remote get-url origin 2>$null
if (-not $remoteExists) {
    Write-Host "🔗 Conectando con GitHub..." -ForegroundColor Yellow
    git remote add origin $repoUrl
} else {
    Write-Host "✅ Remote origin ya configurado: $remoteExists" -ForegroundColor Green
}

# 3. Copiar el nuevo README profesional al proyecto
# (Copia manualmente el README.md generado aquí antes de ejecutar este script)

# 4. Agregar todos los archivos
Write-Host "📦 Preparando archivos..." -ForegroundColor Cyan
git add .

# 5. Ver qué se va a subir
Write-Host "`n📋 Archivos a subir:" -ForegroundColor Cyan
git status --short

# 6. Commit
Write-Host "`n💾 Creando commit..." -ForegroundColor Cyan
git commit -m "feat: sistema completo mezclador IoT ESP32 v5.0

- Firmware Arduino no-bloqueante con maquina de estados (7 estados)
- Control de 6 bombas peristalticas via PWM LEDC
- WebSocket server con latencia <50ms en WiFi AP
- Calibracion dinamica persistente en NVS
- Interfaz web responsive (HTML5/CSS3/JS vanilla, 0 deps)
- 7 pestanas: Conexion, Mezcla, Individual, Limpieza, Calibracion, Historial, Consola
- Servidor Flask para servir la interfaz web
- Documentacion completa con README profesional"

# 7. Push a GitHub
Write-Host "`n🚀 Subiendo a GitHub..." -ForegroundColor Cyan
git push -u origin main

Write-Host "`n✅ ¡Listo! Proyecto subido a:" -ForegroundColor Green
Write-Host "   https://github.com/Santi4216/actuadores-mezcladordepintura-iot" -ForegroundColor White
Write-Host "`n💡 Próximos pasos recomendados:" -ForegroundColor Yellow
Write-Host "   1. Ve al repo en GitHub y agrega una descripción" -ForegroundColor White
Write-Host "   2. Settings → Topics: esp32, iot, websocket, embedded-systems, arduino, flask" -ForegroundColor White
Write-Host "   3. Fija el repo en tu perfil de GitHub (Pin repository)" -ForegroundColor White
