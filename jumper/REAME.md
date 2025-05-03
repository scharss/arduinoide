# Jumper - ESP32 OLED Game

Un juego de salto y recolección para ESP32 con pantalla OLED y control por joystick. Guía a la oveja para que salte sobre los arbustos peligrosos mientras recoges items para obtener la máxima puntuación.

![Gameplay Screenshot](placeholder.png)


## Características

*   Jugabilidad simple e intuitiva: salta con el joystick o botón.
*   Obstáculos generados proceduralmente (Arbustos peligrosos).
*   Items recolectables que otorgan puntos y aumentan el score.
*   Sistema de vidas para mayor desafío.
*   Puntuación y guardado automático de la puntuación más alta (High Score) usando la memoria no volátil del ESP32 (Preferences).
*   Dificultad progresiva: la velocidad del juego aumenta a medida que sumas puntos.
*   Visualización clara en pantalla OLED monocromática de 128x64.
*   Control preciso mediante Joystick analógico.

## Hardware Requerido

1.  **Placa de Desarrollo ESP32:** Cualquier placa común basada en ESP32 WROOM funcionará.
2.  **Pantalla OLED SSD1306 (I2C):** Pantalla monocromática de 128x64 píxeles con interfaz I2C (4 pines: GND, VCC, SCL, SDA).
3.  **Módulo Joystick Analógico:** Módulo estándar con 5 pines (GND, VCC, VRx, VRy, SW).
4.  **Protoboard y Cables Jumper:** Para realizar las conexiones.

## Conexiones / Wiring

Conecta los componentes siguiendo esta tabla. **¡MUY IMPORTANTE!** El ESP32 funciona a **3.3V**. Conecta los pines VCC/VDD de los módulos OLED y Joystick a la salida de **3.3V** del ESP32, **NO a 5V**.

| Componente      | Pin Módulo | Pin ESP32 | Notas                    |
| :-------------- | :--------- | :-------- | :----------------------- |
| **Pantalla OLED** | GND        | GND       | Tierra común             |
|                 | VCC / VDD  | **3.3V**  | **¡No usar 5V!**         |
|                 | SCL        | GPIO 22   | Pin I2C Clock            |
|                 | SDA        | GPIO 21   | Pin I2C Data             |
| **Joystick**    | GND        | GND       | Tierra común             |
|                 | VCC / +5V  | **3.3V**  | **¡No usar 5V!**         |
|                 | VRx        | *NC*      | Eje X (No conectado)    |
|                 | VRy        | GPIO 35   | Eje Y (Salto Arriba)   |
|                 | SW         | GPIO 25   | Botón (Salto)          |

*(NC = No Conectado)*

## Software y Configuración

1.  **Arduino IDE:** Descarga e instala desde [arduino.cc](https://www.arduino.cc/en/software).
2.  **Soporte ESP32 para Arduino IDE:**
    *   Ve a `Archivo` > `Preferencias`.
    *   En "Gestor de URLs Adicionales de Tarjetas", añade: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    *   Ve a `Herramientas` > `Placa` > `Gestor de Tarjetas...`, busca "esp32" e instala el paquete de Espressif Systems.
3.  **Bibliotecas Necesarias:**
    *   Ve a `Herramientas` > `Gestionar Bibliotecas...` e instala:
        *   `Adafruit GFX Library` (por Adafruit)
        *   `Adafruit SSD1306` (por Adafruit)
    *   La biblioteca `Preferences` viene incluida con el core ESP32.
4.  **Configuración de Placa en Arduino IDE:**
    *   Ve a `Herramientas` > `Placa` y selecciona "ESP32 Dev Module" o similar.
    *   Selecciona el puerto COM correcto en `Herramientas` > `Puerto`.

## Compilación y Uso

1.  **Obtén el Código:** Descarga o clona el archivo `.ino` de este proyecto.
2.  **Abre el Sketch:** Abre el archivo `.ino` en el Arduino IDE.
3.  **Gráficos Placeholders:** El código actual usa gráficos geométricos muy simples definidos directamente en el archivo. 
5.  **Juega:**
    *   Mueve el joystick o presiona el botón (SW) para que la oveja salte.
    *   Esquiva los arbustos altos (peligrosos).
    *   Recolecta los monedas (círculos pequeños) para sumar puntos y aumentar tu score. ¡Intenta superar tu High Score!


## Licencia