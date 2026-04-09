"""
Interfaz GUI - Sistema de Mezclado de Pintura
Dark Mode Extremo + Bordes Neon Verde
Tkinter - 1200x800px - Alto Contraste
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import requests
import json
from datetime import datetime
import threading

# ==================== CONFIGURACIÓN ====================
BACKEND_URL = "http://localhost:5000"
ESP32_IP = "192.168.1.50"
ESP32_PORT = 80

# ==================== TEMA ÚNICO: DARK NEON ====================
TEMA = {
    # Fondos
    "bg_main": "#000000",          # Negro puro
    "bg_panel": "#0a0a0a",         # Negro muy oscuro
    "bg_input": "#151515",         # Negro gris oscuro
    "bg_borde": "#1a1a1a",         # Para bordes
    
    # Acentos - Verde Neon
    "neon_verde": "#00ff00",       # Verde puro
    "neon_verde_claro": "#00ff88", # Verde claro
    "neon_verde_oscuro": "#00dd66",# Verde oscuro
    
    # Colores específicos
    "rojo": "#ff1744",             # Rojo magenta
    "verde": "#00ff00",            # Verde neon
    "azul": "#00aaff",             # Azul cian
    "blanco": "#ffffff",           # Blanco puro
    "naranja": "#ffaa00",          # Naranja dorado
    "cian": "#00ddff",             # Cian neon
    
    # Estados
    "success": "#00ff00",          # Verde
    "error": "#ff0055",            # Rojo magenta
    "warning": "#ffaa00",          # Naranja
    "info": "#00ddff",             # Cian
    
    # Texto
    "text_primary": "#ffffff",     # Blanco puro
    "text_secondary": "#aaaaaa",   # Gris claro
    "text_tertiary": "#666666",    # Gris medio
    
    # Bordes
    "border_neon": "#00ff00",      # Verde neon
    "border_glow": "#00dd66",      # Verde más suave
}

# ==================== APLICACIÓN PRINCIPAL ====================
class SistemaMezclado(tk.Tk):
    def __init__(self):
        super().__init__()
        
        self.title("Sistema de Mezclado de Pintura - Control Local")
        self.geometry("1200x800")
        self.resizable(False, False)
        self.config(bg=TEMA["bg_main"])
        
        # Variables
        self.color_actual_rgb = (200, 100, 50)
        self.historico = []
        self.pwm_widgets = {}
        self.nivel_widgets = {}
        self.rgb_sliders = {}
        
        # Iniciar
        self.crear_interfaz()
        self.actualizar_estado()
        
    def crear_interfaz(self):
        """Crear interfaz principal"""
        
        # Contenedor principal
        self.main_frame = tk.Frame(self, bg=TEMA["bg_main"])
        self.main_frame.pack(fill=tk.BOTH, expand=True, padx=0, pady=0)
        
        # Barra superior
        self.crear_barra_superior()
        
        # Área principal - 2 columnas
        content = tk.Frame(self.main_frame, bg=TEMA["bg_main"])
        content.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)
        
        # Columna izquierda (420px)
        left_col = tk.Frame(content, bg=TEMA["bg_main"])
        left_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=False, padx=(0, 8))
        left_col.pack_propagate(False)
        left_col.config(width=420)
        
        self.crear_panel_niveles(left_col)
        self.crear_panel_selector_color(left_col)
        
        # Columna derecha (expandible)
        right_col = tk.Frame(content, bg=TEMA["bg_main"])
        right_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        self.crear_panel_control_pwm(right_col)
        
        # Barra inferior
        self.crear_panel_historico()
        
    def crear_barra_superior(self):
        """Barra superior con título y estado"""
        
        # Frame con borde neon
        barra_outer = tk.Frame(self.main_frame, bg=TEMA["border_neon"], height=75)
        barra_outer.pack(fill=tk.X, padx=8, pady=(8, 0))
        barra_outer.pack_propagate(False)
        
        barra = tk.Frame(barra_outer, bg=TEMA["bg_panel"], height=73)
        barra.place(x=2, y=2, relwidth=0.98, height=73)
        barra.pack_propagate(False)
        
        # Lado izquierdo - Título
        left = tk.Frame(barra, bg=TEMA["bg_panel"])
        left.pack(side=tk.LEFT, padx=20, pady=15)
        
        titulo = tk.Label(
            left, text="🎨 SISTEMA DE MEZCLADO",
            bg=TEMA["bg_panel"],
            fg=TEMA["neon_verde"],
            font=("Arial", 18, "bold")
        )
        titulo.pack(side=tk.LEFT)
        
        # Lado derecho - Estado
        right = tk.Frame(barra, bg=TEMA["bg_panel"])
        right.pack(side=tk.RIGHT, padx=20, pady=15)
        
        # ESP32
        frame_esp32 = tk.Frame(right, bg=TEMA["bg_panel"])
        frame_esp32.pack(side=tk.LEFT, padx=15)
        
        self.esp32_dot = tk.Label(
            frame_esp32, text="🔴",
            bg=TEMA["bg_panel"],
            fg=TEMA["error"],
            font=("Arial", 20)
        )
        self.esp32_dot.pack(side=tk.LEFT, padx=(0, 10))
        
        self.esp32_label = tk.Label(
            frame_esp32, text="ESP32",
            bg=TEMA["bg_panel"],
            fg=TEMA["text_primary"],
            font=("Arial", 12, "bold")
        )
        self.esp32_label.pack(side=tk.LEFT)
        
        # Backend
        frame_backend = tk.Frame(right, bg=TEMA["bg_panel"])
        frame_backend.pack(side=tk.LEFT, padx=15)
        
        self.backend_dot = tk.Label(
            frame_backend, text="🔴",
            bg=TEMA["bg_panel"],
            fg=TEMA["error"],
            font=("Arial", 20)
        )
        self.backend_dot.pack(side=tk.LEFT, padx=(0, 10))
        
        self.backend_label = tk.Label(
            frame_backend, text="BACKEND",
            bg=TEMA["bg_panel"],
            fg=TEMA["text_primary"],
            font=("Arial", 12, "bold")
        )
        self.backend_label.pack(side=tk.LEFT)
        
        # Botón tema
        self.crear_boton_neon(right, "🌙", self.cambiar_tema, 50).pack(side=tk.LEFT, padx=15)
    
    def crear_panel_niveles(self, parent):
        """Panel de niveles de tanques"""
        panel = self.crear_panel_neon(parent, "TANQUES", altura=300)
        panel.pack(fill=tk.BOTH, expand=False, pady=(0, 8))
        
        self.nivel_widgets = {}
        
        tanques = [
            ("ROJO", TEMA["rojo"]),
            ("VERDE", TEMA["neon_verde"]),
            ("AZUL", TEMA["azul"]),
            ("BLANCO", TEMA["blanco"]),
            ("LIMPIEZA", TEMA["cian"])
        ]
        
        for nombre, color in tanques:
            frame = tk.Frame(panel, bg=TEMA["bg_panel"])
            frame.pack(fill=tk.X, pady=8, padx=10)
            
            # Nombre y porcentaje
            top = tk.Frame(frame, bg=TEMA["bg_panel"])
            top.pack(fill=tk.X, pady=(0, 5))
            
            tk.Label(
                top, text=nombre,
                bg=TEMA["bg_panel"],
                fg=TEMA["text_primary"],
                font=("Arial", 11, "bold"),
                width=10
            ).pack(side=tk.LEFT)
            
            lbl_pct = tk.Label(
                top, text="0%",
                bg=TEMA["bg_panel"],
                fg=color,
                font=("Arial", 11, "bold"),
                width=8
            )
            lbl_pct.pack(side=tk.RIGHT)
            
            # Barra de nivel con borde neon
            canvas_outer = tk.Canvas(
                frame, width=400, height=32,
                bg=TEMA["border_neon"],
                highlightthickness=0,
                relief=tk.FLAT
            )
            canvas_outer.pack(fill=tk.X)
            
            canvas = tk.Canvas(
                canvas_outer, width=395, height=28,
                bg=TEMA["bg_input"],
                highlightthickness=0,
                relief=tk.FLAT
            )
            canvas.place(x=1, y=2)
            
            self.nivel_widgets[nombre] = {
                "canvas": canvas,
                "label": lbl_pct,
                "color": color,
            }
    
    def crear_panel_selector_color(self, parent):
        """Panel selector de color RGB"""
        panel = self.crear_panel_neon(parent, "SELECTOR RGB", altura=410)
        panel.pack(fill=tk.BOTH, expand=True, pady=8)
        
        # Sliders RGB
        self.rgb_sliders = {}
        
        for i, (letra, color) in enumerate([("R", TEMA["rojo"]), ("G", TEMA["neon_verde"]), ("B", TEMA["azul"])]):
            frame = tk.Frame(panel, bg=TEMA["bg_panel"])
            frame.pack(fill=tk.X, pady=10, padx=10)
            
            # Label
            tk.Label(
                frame, text=f"{letra}:",
                bg=TEMA["bg_panel"],
                fg=TEMA["text_primary"],
                font=("Arial", 11, "bold"),
                width=3
            ).pack(side=tk.LEFT, padx=(0, 10))
            
            # Slider personalizado
            slider = tk.Scale(
                frame, from_=0, to=255,
                orient=tk.HORIZONTAL,
                bg=TEMA["bg_input"],
                fg=color,
                troughcolor=TEMA["bg_input"],
                activebackground=color,
                command=self.actualizar_color,
                bd=1,
                highlightthickness=1,
                highlightbackground=TEMA["border_neon"],
                highlightcolor=TEMA["border_neon"]
            )
            slider.set([200, 100, 50][i])
            slider.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
            
            # Valor
            lbl_val = tk.Label(
                frame, text="200",
                bg=TEMA["bg_panel"],
                fg=color,
                font=("Arial", 11, "bold"),
                width=5
            )
            lbl_val.pack(side=tk.LEFT, padx=10)
            
            self.rgb_sliders[letra] = {"slider": slider, "label": lbl_val}
        
        # Vista previa del color
        frame_preview = tk.Frame(panel, bg=TEMA["bg_panel"])
        frame_preview.pack(fill=tk.X, pady=20)
        
        # Canvas con borde neon
        canvas_outer = tk.Canvas(
            frame_preview, width=104, height=104,
            bg=TEMA["border_neon"],
            highlightthickness=0
        )
        canvas_outer.pack()
        
        self.preview_canvas = tk.Canvas(
            canvas_outer, width=100, height=100,
            bg=TEMA["bg_input"],
            highlightthickness=0
        )
        self.preview_canvas.place(x=2, y=2)
        
        self.dibujar_preview()
        
        # Código hex
        frame_hex = tk.Frame(panel, bg=TEMA["bg_panel"])
        frame_hex.pack(fill=tk.X, pady=10, padx=10)
        
        tk.Label(
            frame_hex, text="HEX:",
            bg=TEMA["bg_panel"],
            fg=TEMA["text_primary"],
            font=("Arial", 10, "bold")
        ).pack(side=tk.LEFT, padx=5)
        
        self.hex_entry = tk.Entry(
            frame_hex,
            font=("Courier", 12, "bold"),
            width=12,
            bg=TEMA["bg_input"],
            fg=TEMA["neon_verde"],
            insertbackground=TEMA["neon_verde"],
            bd=1,
            relief=tk.FLAT
        )
        self.hex_entry.pack(side=tk.LEFT, padx=5)
        self.hex_entry.insert(0, "#C82D32")
        
        # Botones
        frame_botones = tk.Frame(panel, bg=TEMA["bg_panel"])
        frame_botones.pack(fill=tk.X, pady=15, padx=10)
        
        self.crear_boton_neon(
            frame_botones, "💾 Guardar", self.guardar_receta, 120
        ).pack(side=tk.LEFT, padx=5)
        
        self.crear_boton_neon(
            frame_botones, "📂 Cargar", self.cargar_receta, 120
        ).pack(side=tk.LEFT, padx=5)
        
        self.crear_boton_neon(
            frame_botones, "🧹 Limpiar", self.limpiar_sistema, 120, tipo="error"
        ).pack(side=tk.LEFT, padx=5)
    
    def crear_panel_control_pwm(self, parent):
        """Panel de control PWM"""
        panel = self.crear_panel_neon(parent, "CONTROL PWM", altura=350)
        panel.pack(fill=tk.BOTH, expand=False, pady=(0, 8))
        
        self.pwm_widgets = {}
        
        bombas = [
            ("ROJO", TEMA["rojo"]),
            ("VERDE", TEMA["neon_verde"]),
            ("AZUL", TEMA["azul"]),
            ("BLANCO", TEMA["blanco"]),
            ("LIMPIEZA", TEMA["cian"])
        ]
        
        for nombre, color in bombas:
            frame = tk.Frame(panel, bg=TEMA["bg_panel"])
            frame.pack(fill=tk.X, pady=8, padx=10)
            
            # Nombre
            tk.Label(
                frame, text=nombre,
                bg=TEMA["bg_panel"],
                fg=TEMA["text_primary"],
                font=("Arial", 10, "bold"),
                width=10
            ).pack(side=tk.LEFT, padx=(0, 10))
            
            # Slider PWM
            slider = tk.Scale(
                frame, from_=0, to=255,
                orient=tk.HORIZONTAL,
                bg=TEMA["bg_input"],
                fg=color,
                troughcolor=TEMA["bg_input"],
                activebackground=color,
                bd=1,
                highlightthickness=1,
                highlightbackground=TEMA["border_neon"]
            )
            slider.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
            
            # Porcentaje
            lbl_pct = tk.Label(
                frame, text="0%",
                bg=TEMA["bg_panel"],
                fg=color,
                font=("Arial", 10, "bold"),
                width=5
            )
            lbl_pct.pack(side=tk.LEFT, padx=10)
            
            # Botón individual
            btn = self.crear_boton_neon(
                frame, "▶", None, 45, color_custom=color
            )
            btn.pack(side=tk.LEFT, padx=5)
            
            self.pwm_widgets[nombre] = {
                "slider": slider,
                "label": lbl_pct,
                "button": btn,
            }
        
        # Botones principales
        frame_botones = tk.Frame(panel, bg=TEMA["bg_panel"])
        frame_botones.pack(fill=tk.X, pady=20, padx=10)
        
        self.crear_boton_neon(
            frame_botones, "🎨 MEZCLAR TODO", self.mezclar_color,
            200, tipo="success"
        ).pack(side=tk.LEFT, padx=5, fill=tk.X, expand=True)
        
        self.crear_boton_neon(
            frame_botones, "⏹ PARAR TODO", self.parar_todo,
            200, tipo="error"
        ).pack(side=tk.LEFT, padx=5, fill=tk.X, expand=True)
    
    def crear_panel_historico(self):
        """Panel de histórico"""
        panel_outer = tk.Frame(self.main_frame, bg=TEMA["border_neon"], height=130)
        panel_outer.pack(fill=tk.X, padx=8, pady=(8, 8))
        panel_outer.pack_propagate(False)
        
        panel = tk.Frame(panel_outer, bg=TEMA["bg_panel"], height=128)
        panel.place(x=2, y=2, relwidth=0.99, height=128)
        
        # Título
        titulo = tk.Label(
            panel, text="HISTÓRICO",
            bg=TEMA["bg_panel"],
            fg=TEMA["neon_verde"],
            font=("Arial", 11, "bold"),
            padx=10,
            pady=8
        )
        titulo.pack(anchor=tk.W)
        
        # Frame con scroll
        frame_scroll = tk.Frame(panel, bg=TEMA["bg_panel"])
        frame_scroll.pack(fill=tk.BOTH, expand=True, pady=5, padx=10)
        
        scrollbar = tk.Scrollbar(frame_scroll)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.historico_listbox = tk.Listbox(
            frame_scroll,
            bg=TEMA["bg_input"],
            fg=TEMA["text_primary"],
            yscrollcommand=scrollbar.set,
            height=4,
            font=("Courier", 9),
            bd=0,
            highlightthickness=1,
            highlightbackground=TEMA["border_neon"]
        )
        self.historico_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.config(command=self.historico_listbox.yview)
    
    # ==================== MÉTODOS AUXILIARES ====================
    
    def crear_panel_neon(self, parent, titulo, altura=None):
        """Crear panel con borde neon verde"""
        
        # Marco exterior (borde neon)
        panel_outer = tk.Frame(parent, bg=TEMA["border_neon"], relief=tk.FLAT, bd=0)
        panel_outer.pack(fill=tk.BOTH, expand=True if altura is None else False)
        
        if altura:
            panel_outer.config(height=altura)
            panel_outer.pack_propagate(False)
        
        # Panel interior (contenido)
        panel = tk.Frame(panel_outer, bg=TEMA["bg_panel"], relief=tk.FLAT, bd=0)
        panel.place(x=2, y=2, relwidth=0.98, relheight=0.96)
        
        # Título
        titulo_lbl = tk.Label(
            panel,
            text=titulo,
            bg=TEMA["bg_panel"],
            fg=TEMA["neon_verde"],
            font=("Arial", 12, "bold"),
            padx=10,
            pady=8
        )
        titulo_lbl.pack(anchor=tk.W)
        
        # Separador
        sep = tk.Frame(panel, bg=TEMA["border_neon"], height=1)
        sep.pack(fill=tk.X, padx=10)
        sep.pack_propagate(False)
        
        return panel
    
    def crear_boton_neon(self, parent, texto, comando, ancho=100, tipo="primary", color_custom=None):
        """Crear botón neon"""
        
        if tipo == "success":
            bg = TEMA["success"]
            fg = "#000000"
        elif tipo == "error":
            bg = TEMA["error"]
            fg = TEMA["text_primary"]
        elif color_custom:
            bg = color_custom
            fg = "#000000" if self._es_claro(color_custom) else TEMA["text_primary"]
        else:
            bg = TEMA["neon_verde"]
            fg = "#000000"
        
        boton = tk.Button(
            parent,
            text=texto,
            command=comando,
            bg=bg,
            fg=fg,
            font=("Arial", 10, "bold"),
            relief=tk.FLAT,
            bd=0,
            padx=10,
            pady=5,
            activebackground=bg,
            activeforeground=fg,
            cursor="hand2",
            width=ancho//8 if ancho else None
        )
        
        # Hover effect
        def on_enter(event):
            boton.config(relief=tk.RAISED, bd=2)
        
        def on_leave(event):
            boton.config(relief=tk.FLAT, bd=0)
        
        boton.bind("<Enter>", on_enter)
        boton.bind("<Leave>", on_leave)
        
        return boton
    
    def _es_claro(self, color_hex):
        """Determinar si color es claro"""
        color = color_hex.lstrip('#')
        r, g, b = tuple(int(color[i:i+2], 16) for i in (0, 2, 4))
        luminancia = (0.299 * r + 0.587 * g + 0.114 * b) / 255
        return luminancia > 0.5
    
    def dibujar_preview(self, r=200, g=100, b=50):
        """Dibujar preview circular"""
        self.preview_canvas.delete("all")
        color_hex = f"#{r:02x}{g:02x}{b:02x}"
        
        # Círculo
        self.preview_canvas.create_oval(
            10, 10, 90, 90,
            fill=color_hex,
            outline=TEMA["neon_verde"],
            width=2
        )
    
    def actualizar_color(self, value=None):
        """Actualizar color RGB"""
        r = self.rgb_sliders["R"]["slider"].get()
        g = self.rgb_sliders["G"]["slider"].get()
        b = self.rgb_sliders["B"]["slider"].get()
        
        self.color_actual_rgb = (r, g, b)
        
        self.rgb_sliders["R"]["label"].config(text=str(r))
        self.rgb_sliders["G"]["label"].config(text=str(g))
        self.rgb_sliders["B"]["label"].config(text=str(b))
        
        color_hex = f"#{r:02x}{g:02x}{b:02x}"
        self.dibujar_preview(r, g, b)
        self.hex_entry.delete(0, tk.END)
        self.hex_entry.insert(0, color_hex)
    
    def mezclar_color(self):
        """Mezclar"""
        threading.Thread(target=self._mezclar_thread, daemon=True).start()
    
    def _mezclar_thread(self):
        """Thread mezcla"""
        try:
            r, g, b = self.color_actual_rgb
            response = requests.post(
                f"{BACKEND_URL}/api/mezclar",
                json={"r": r, "g": g, "b": b},
                timeout=5
            )
            
            if response.status_code == 200:
                color_hex = f"#{r:02x}{g:02x}{b:02x}"
                self.agregar_historico(f"✓ Mezcla: {color_hex}", "success")
            else:
                self.agregar_historico("✗ Error mezcla", "error")
        except Exception as e:
            self.agregar_historico(f"✗ {str(e)}", "error")
    
    def parar_todo(self):
        """Parar"""
        threading.Thread(target=self._parar_thread, daemon=True).start()
    
    def _parar_thread(self):
        """Thread parada"""
        try:
            response = requests.post(f"{BACKEND_URL}/api/parar", timeout=5)
            
            if response.status_code == 200:
                self.agregar_historico("⏹ Sistema parado", "warning")
            else:
                self.agregar_historico("✗ Error parada", "error")
        except Exception as e:
            self.agregar_historico(f"✗ {str(e)}", "error")
    
    def limpiar_sistema(self):
        """Limpiar"""
        if messagebox.askyesno("Confirmar", "¿Limpiar sistema?"):
            threading.Thread(target=self._limpiar_thread, daemon=True).start()
    
    def _limpiar_thread(self):
        """Thread limpieza"""
        try:
            response = requests.post(f"{BACKEND_URL}/api/limpiar", timeout=5)
            
            if response.status_code == 200:
                self.agregar_historico("🧹 Limpieza completada", "success")
            else:
                self.agregar_historico("✗ Error limpieza", "error")
        except Exception as e:
            self.agregar_historico(f"✗ {str(e)}", "error")
    
    def guardar_receta(self):
        """Guardar receta"""
        nombre = tk.simpledialog.askstring("Guardar", "Nombre receta:")
        if nombre:
            threading.Thread(
                target=self._guardar_receta_thread,
                args=(nombre,),
                daemon=True
            ).start()
    
    def _guardar_receta_thread(self, nombre):
        """Thread guardar"""
        try:
            r, g, b = self.color_actual_rgb
            response = requests.post(
                f"{BACKEND_URL}/api/recetas",
                json={"nombre": nombre, "r": r, "g": g, "b": b},
                timeout=5
            )
            
            if response.status_code == 200:
                self.agregar_historico(f"💾 Receta: {nombre}", "info")
            else:
                self.agregar_historico("✗ Error guardando", "error")
        except Exception as e:
            self.agregar_historico(f"✗ {str(e)}", "error")
    
    def cargar_receta(self):
        """Cargar receta"""
        messagebox.showinfo("Info", "En desarrollo...")
    
    def cambiar_tema(self):
        """Cambiar tema"""
        messagebox.showinfo("Info", "Un solo tema neon disponible")
    
    def actualizar_estado(self):
        """Actualizar estado cada 2s"""
        threading.Thread(target=self._actualizar_estado_thread, daemon=True).start()
        self.after(2000, self.actualizar_estado)
    
    def _actualizar_estado_thread(self):
        """Thread actualización"""
        try:
            response = requests.get(f"{BACKEND_URL}/api/estado", timeout=3)
            
            if response.status_code == 200:
                self.esp32_dot.config(text="🟢", fg=TEMA["success"])
                self.esp32_label.config(text="ESP32 OK", fg=TEMA["text_primary"])
                self.backend_dot.config(text="🟢", fg=TEMA["success"])
                self.backend_label.config(text="BACKEND OK", fg=TEMA["text_primary"])
                
                datos = response.json()
                if "esp32" in datos and "niveles" in datos["esp32"]:
                    self.actualizar_niveles(datos["esp32"]["niveles"])
            else:
                self._conexion_error()
        except:
            self._conexion_error()
    
    def _conexion_error(self):
        """Error conexión"""
        self.esp32_dot.config(text="🔴", fg=TEMA["error"])
        self.esp32_label.config(text="DESCONECTADO")
        self.backend_dot.config(text="🔴", fg=TEMA["error"])
        self.backend_label.config(text="DESCONECTADO")
    
    def actualizar_niveles(self, niveles):
        """Actualizar niveles"""
        for color, nivel in niveles.items():
            if color.upper() in self.nivel_widgets:
                widget = self.nivel_widgets[color.upper()]
                canvas = widget["canvas"]
                
                canvas.delete("all")
                
                # Fondo
                canvas.create_rectangle(0, 0, 395, 28, fill=TEMA["bg_input"], outline=TEMA["border_neon"])
                
                # Barra
                ancho = (nivel / 100) * 395
                if ancho > 0:
                    canvas.create_rectangle(0, 0, ancho, 28, fill=widget["color"], outline=widget["color"])
                
                # Texto
                widget["label"].config(text=f"{nivel}%")
    
    def agregar_historico(self, mensaje, tipo="info"):
        """Agregar historico"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        entrada = f"[{timestamp}] {mensaje}"
        
        colores = {
            "success": TEMA["success"],
            "error": TEMA["error"],
            "warning": TEMA["warning"],
            "info": TEMA["info"]
        }
        
        self.historico_listbox.insert(0, entrada)
        self.historico_listbox.itemconfig(0, {'fg': colores.get(tipo, TEMA["info"])})

# ==================== MAIN ====================
if __name__ == "__main__":
    import tkinter.simpledialog
    app = SistemaMezclado()
    app.mainloop()