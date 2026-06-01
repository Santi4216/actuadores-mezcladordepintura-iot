# ═══════════════════════════════════════════════════════════════════════════
# LIMPIAR_PROYECTO.ps1 — Limpia archivos obsoletos del proyecto
# Alternativa PowerShell al .bat (compatible Windows 7+)
# ═══════════════════════════════════════════════════════════════════════════

Write-Host "
🧹 LIMPIEZA DE PROYECTO — Mezclador ESP32 v5
═══════════════════════════════════════════════════════════════════════════
" -ForegroundColor Green

Write-Host "Esto eliminará archivos obsoletos:" -ForegroundColor Yellow
Write-Host "  × interfaz_gui.py (GUI PyQt6 vieja)" -ForegroundColor Red
Write-Host "  × main.py (punto de entrada viejo)" -ForegroundColor Red
Write-Host "  × test*.py (tests viejos)" -ForegroundColor Red
Write-Host "  × sincronizar*.py (sincronización vieja)" -ForegroundColor Red
Write-Host "  × gui_pyqt6/ (carpeta completa)" -ForegroundColor Red
Write-Host "  × conectar_azure.py, setup_azure.bat" -ForegroundColor Red
Write-Host "  × Otros archivos obsoletos" -ForegroundColor Red
Write-Host ""
Write-Host "✓ Se MANTIENE:" -ForegroundColor Green
Write-Host "  - web_server.py, requirements.txt" -ForegroundColor Green
Write-Host "  - INICIO_RAPIDO.md, README.md" -ForegroundColor Green
Write-Host "  - esp32_firmware/, web/, config/, data/" -ForegroundColor Green
Write-Host ""

$confirm = Read-Host "¿Continuar? (s/n)"
if ($confirm -ne "s" -and $confirm -ne "S") {
    Write-Host "❌ Cancelado" -ForegroundColor Red
    exit
}

# Archivos individuales a eliminar
$archivos = @(
    "interfaz_gui.py",
    "main.py",
    "test.py",
    "test_auto_sync.py",
    "test_conexion.py",
    "test_final.py",
    "test_gui_flow.py",
    "verify_setup.py",
    "sincronizar.py",
    "sincronizar_manual.py",
    "sincronizar.ps1",
    "conectar_azure.py",
    "setup_azure.bat",
    "ver_historial.py",
    "interfaz.txt",
    "proyecto_completo.txt",
    "ESTADO_ACTUAL.md",
    "SINCRONIZACION_GUIA.md",
    "WEB_v5_README.md",
    "app.log"
)

# Eliminar archivos
foreach ($archivo in $archivos) {
    if (Test-Path $archivo) {
        Remove-Item $archivo -Force
        Write-Host "✓ Eliminado: $archivo" -ForegroundColor Green
    }
}

# Carpetas a eliminar
$carpetas = @(
    "gui_pyqt6",
    "__pycache__",
    ".venv"
)

# Eliminar carpetas
foreach ($carpeta in $carpetas) {
    if (Test-Path $carpeta) {
        Remove-Item $carpeta -Recurse -Force
        Write-Host "✓ Eliminada: $carpeta\" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "✅ LIMPIEZA COMPLETADA" -ForegroundColor Green
Write-Host ""
Write-Host "📁 Estructura final:" -ForegroundColor Cyan
Write-Host "   - web_server.py" -ForegroundColor Cyan
Write-Host "   - requirements.txt" -ForegroundColor Cyan
Write-Host "   - INICIO_RAPIDO.md" -ForegroundColor Cyan
Write-Host "   - README.md" -ForegroundColor Cyan
Write-Host "   - ARQUITECTURA.md" -ForegroundColor Cyan
Write-Host "   - esp32_firmware/" -ForegroundColor Cyan
Write-Host "   - web/" -ForegroundColor Cyan
Write-Host "   - config/" -ForegroundColor Cyan
Write-Host "   - data/" -ForegroundColor Cyan
Write-Host ""
Write-Host "🚀 Próximo paso: pip install -r requirements.txt" -ForegroundColor Green
Write-Host ""

pause
