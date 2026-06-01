"""
Servidor web Flask para servir la interfaz v5 del Mezclador ESP32
Sirve el HTML en http://localhost:5000
"""
from flask import Flask, render_template_string
from pathlib import Path

app = Flask(__name__)

# Cargar el HTML desde el archivo
WEB_DIR = Path(__file__).parent / "web"
HTML_FILE = WEB_DIR / "index.html"

@app.route('/')
@app.route('/index.html')
def index():
    """Sirve la interfaz principal"""
    with open(HTML_FILE, 'r', encoding='utf-8') as f:
        html_content = f.read()
    return html_content

@app.route('/health')
def health():
    """Endpoint para verificar que el servidor está activo"""
    return {'status': 'ok', 'version': '5.0'}, 200

if __name__ == '__main__':
    print("🚀 Servidor web v5 iniciando...")
    print("📱 Accede a: http://localhost:5000")
    print("💡 Conecta tu PC al WiFi: Mezclador-ESP32 (contraseña: mezclador123)")
    print("🔌 El HTML se conectará directamente al ESP32 por WebSocket (192.168.4.1:81)")
    print("\n⚠️  NOTA: El ESP32 debe estar ejecutando el firmware v5 con soporte WebSocket")
    
    app.run(
        host='0.0.0.0',
        port=5000,
        debug=False,
        threaded=True
    )
