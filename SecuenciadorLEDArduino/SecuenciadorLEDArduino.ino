// --- Definición de Pines ---
const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // Pines para los 8 LEDs
const int numLeds = 8;                         // Número total de LEDs

const int redButtonPin = 10;   // Pin para el botón ROJO (lento)
const int blueButtonPin = 11;  // Pin para el botón AZUL (medio)
const int whiteButtonPin = 12; // Pin para el botón BLANCO (rápido)

// --- Definición de Velocidades (tiempo de pausa en milisegundos) ---
const int delaySlow = 500;    // Velocidad lenta
const int delayMedium = 200;  // Velocidad media
const int delayFast = 50;     // Velocidad rápida

// --- Variables Globales ---
int currentDelay = delayMedium; // Velocidad actual, empieza en media
int ledIndex = 0;               // Índice del LED actual que debe encenderse
unsigned long lastUpdateTime = 0; // Variable para temporización no bloqueante (millis())

// Variables para detectar el estado del botón (evitar cambios múltiples si se mantiene presionado)
int lastRedButtonState = HIGH;
int lastBlueButtonState = HIGH;
int lastWhiteButtonState = HIGH;

void setup() {
  // Configurar los pines de los LEDs como SALIDA (OUTPUT)
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Asegurarse que todos empiecen apagados
  }

  // Configurar los pines de los botones como ENTRADA con PULLUP interno
  // El pin leerá HIGH cuando no esté presionado y LOW cuando sí lo esté.
  pinMode(redButtonPin, INPUT_PULLUP);
  pinMode(blueButtonPin, INPUT_PULLUP);
  pinMode(whiteButtonPin, INPUT_PULLUP);

  // Opcional: Iniciar comunicación serial para depuración
  // Serial.begin(9600);
  // Serial.println("Sistema iniciado. Velocidad media por defecto.");
}

void loop() {
  // --- Leer el estado actual de los botones ---
  int redState = digitalRead(redButtonPin);
  int blueState = digitalRead(blueButtonPin);
  int whiteState = digitalRead(whiteButtonPin);

  // --- Comprobar si se ha presionado un botón (flanco descendente: de HIGH a LOW) ---
  // Botón Rojo (Lento)
  if (redState == LOW && lastRedButtonState == HIGH) {
    currentDelay = delaySlow;
    // Serial.println("Velocidad cambiada a LENTA");
    delay(50); // Pequeña pausa para debounce (anti-rebote) básico
  }
  // Botón Azul (Medio)
  if (blueState == LOW && lastBlueButtonState == HIGH) {
    currentDelay = delayMedium;
    // Serial.println("Velocidad cambiada a MEDIA");
    delay(50); // Pequeña pausa para debounce
  }
  // Botón Blanco (Rápido)
  if (whiteState == LOW && lastWhiteButtonState == HIGH) {
    currentDelay = delayFast;
    // Serial.println("Velocidad cambiada a RAPIDA");
    delay(50); // Pequeña pausa para debounce
  }

  // Actualizar el último estado conocido de los botones para la próxima iteración
  lastRedButtonState = redState;
  lastBlueButtonState = blueState;
  lastWhiteButtonState = whiteState;


  // --- Actualizar el efecto de luz corrediza usando millis() ---
  // Esto permite que el programa siga leyendo los botones mientras los LEDs "corren"
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= currentDelay) {
    // Es hora de mover la luz al siguiente LED

    // 1. Apagar todos los LEDs (más simple que apagar solo el anterior)
    for (int i = 0; i < numLeds; i++) {
      digitalWrite(ledPins[i], LOW);
    }

    // 2. Encender el LED actual
    digitalWrite(ledPins[ledIndex], HIGH);

    // 3. Actualizar el tiempo de la última actualización
    lastUpdateTime = currentTime;

    // 4. Incrementar el índice para el siguiente LED, volviendo a 0 si llega al final
    ledIndex++;
    if (ledIndex >= numLeds) {
      ledIndex = 0; // Volver al inicio
    }
    // Alternativa más concisa usando módulo: ledIndex = (ledIndex + 1) % numLeds;
  }
}