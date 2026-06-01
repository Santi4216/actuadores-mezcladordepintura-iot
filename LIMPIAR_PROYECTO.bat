@echo off
REM ═══════════════════════════════════════════════════════════════════════════
REM Limpieza de proyecto — Elimina archivos y carpetas no necesarias para v5
REM ═══════════════════════════════════════════════════════════════════════════

setlocal enabledelayedexpansion

echo.
echo 🧹 LIMPIEZA DE PROYECTO — Mezclador ESP32 v5
echo ═══════════════════════════════════════════════════════════════════════════
echo.
echo Esto eliminará archivos obsoletos:
echo   × interfaz_gui.py (GUI PyQt6 vieja)
echo   × main.py (punto de entrada viejo)
echo   × test*.py (tests viejos)
echo   × sincronizar*.py (sincronización vieja)
echo   × gui_pyqt6/ (carpeta completa)
echo   × conectar_azure.py, setup_azure.bat
echo   × ESTADO_ACTUAL.md, SINCRONIZACION_GUIA.md, WEB_v5_README.md
echo   × __pycache__/, .venv/
echo.
echo ✓ Se MANTIENE:
echo   - web_server.py, requirements.txt
echo   - INICIO_RAPIDO.md, README.md
echo   - esp32_firmware/, web/, config/, data/
echo.

set /p confirm="¿Continuar? (s/n): "
if /i not "%confirm%"=="s" (
    echo ❌ Cancelado
    exit /b 0
)

REM Archivos individuales
for %%F in (
    interfaz_gui.py
    main.py
    test.py
    test_auto_sync.py
    test_conexion.py
    test_final.py
    test_gui_flow.py
    verify_setup.py
    sincronizar.py
    sincronizar_manual.py
    sincronizar.ps1
    conectar_azure.py
    setup_azure.bat
    ver_historial.py
    interfaz.txt
    proyecto_completo.txt
    ESTADO_ACTUAL.md
    SINCRONIZACION_GUIA.md
    WEB_v5_README.md
    app.log
) do (
    if exist "%%F" (
        del /q "%%F"
        echo ✓ Eliminado: %%F
    )
)

REM Carpetas
for %%D in (
    gui_pyqt6
    __pycache__
    .venv
) do (
    if exist "%%D" (
        rmdir /s /q "%%D"
        echo ✓ Eliminada: %%D\
    )
)

echo.
echo ✅ LIMPIEZA COMPLETADA
echo.
echo 📁 Estructura final:
echo    - web_server.py
echo    - requirements.txt
echo    - INICIO_RAPIDO.md
echo    - README.md
echo    - esp32_firmware/
echo    - web/
echo    - config/
echo    - data/
echo.
echo 🚀 Próximo paso: pip install -r requirements.txt
echo.

pause
