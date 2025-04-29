#include <Wire.h>             // Para comunicación I2C
#include <Adafruit_GFX.h>     // Librería gráfica de Adafruit
#include <Adafruit_SSD1306.h> // Librería específica para el controlador SSD1306

// --- Configuración de la Pantalla OLED ---
#define SCREEN_WIDTH 128 // Ancho en píxeles de la pantalla OLED
#define SCREEN_HEIGHT 64 // Alto en píxeles de la pantalla OLED
#define OLED_RESET -1    // Pin de Reset (la mayoría de los módulos I2C no lo necesitan)
#define SCREEN_ADDRESS 0x3C // Dirección I2C de la pantalla (puede ser 0x3D en algunos casos)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Configuración del Joystick ---
#define JOYSTICK_X_PIN 34 // Pin analógico para el eje X (ej. VRX)
#define JOYSTICK_Y_PIN 35 // Pin analógico para el eje Y (ej. VRY - no usado en este juego)
#define JOYSTICK_BTN_PIN 27 // Pin digital para el botón (ej. SW)

// --- Configuración del Juego ---
// Paleta
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 3
int paddleX; // Posición X de la paleta

// Pelota
#define BALL_SIZE 3
float ballX, ballY;       // Posición de la pelota (usamos float para movimiento suave)
float ballVX, ballVY;     // Velocidad/Dirección de la pelota
bool ballMoving = false; // Estado de la pelota

// Ladrillos
#define BRICK_ROWS 5
#define BRICKS_PER_ROW 10
#define BRICK_WIDTH 10
#define BRICK_HEIGHT 4
#define BRICK_PADDING 2 // Espacio entre ladrillos
#define BRICK_OFFSET_TOP 8 // Espacio desde la parte superior para los ladrillos
#define BRICK_OFFSET_LEFT ((SCREEN_WIDTH - (BRICKS_PER_ROW * (BRICK_WIDTH + BRICK_PADDING)) + BRICK_PADDING) / 2) // Centrar ladrillos

bool bricks[BRICK_ROWS][BRICKS_PER_ROW]; // Matriz para saber si el ladrillo está activo

// --- Variables del Juego ---
enum GameState { WAITING_TO_START, PLAYING, GAME_OVER, GAME_WON };
GameState currentGameState = WAITING_TO_START;

// --- Tiempo de Juego ---
unsigned long lastUpdateTime = 0;
#define FRAME_DELAY_MS 20 // Aproximadamente 50 FPS (1000 / 50 = 20)

// --- Funciones ---

void setup() {
  Serial.begin(115200);

  // --- Inicializar OLED ---
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // No seguir si falla
  }
  display.display(); // Mostrar el logo de Adafruit brevemente
  delay(2000);
  display.clearDisplay();

  // --- Configurar Pines del Joystick ---
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP); // Configurar el botón con pull-up interna

  resetGame();
}

void loop() {
  // Controlar la velocidad de actualización para un frame rate más estable
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < FRAME_DELAY_MS) {
    return; // Esperar hasta que pase el tiempo del frame
  }
  lastUpdateTime = currentTime;

  // --- Leer Input ---
  int joystickX = analogRead(JOYSTICK_X_PIN);
  bool buttonPressed = (digitalRead(JOYSTICK_BTN_PIN) == LOW); // LOW si el botón está presionado

  // --- Lógica del Juego ---
  switch (currentGameState) {
    case WAITING_TO_START:
      // Mover paleta para posicionar la pelota
      movePaddle(joystickX);
      // Posicionar la pelota encima de la paleta
      ballX = paddleX + (PADDLE_WIDTH / 2) - (BALL_SIZE / 2);
      ballY = SCREEN_HEIGHT - PADDLE_HEIGHT - BALL_SIZE - 1;

      displayWaitingScreen(); // Mostrar pantalla de espera

      if (buttonPressed) {
        currentGameState = PLAYING;
        ballMoving = true;
        // Lanzar la pelota (velocidad inicial)
        ballVX = 1.5; // Ajusta la velocidad X si quieres
        ballVY = -1.5; // Ajusta la velocidad Y (negativo para ir hacia arriba)
        // Ajuste aleatorio de la velocidad X inicial (opcional)
        randomSeed(millis()); // Inicializa la semilla aleatoria
        ballVX = (random(0, 2) == 0) ? -ballVX : ballVX; // Dirección inicial aleatoria X
      }
      break;

    case PLAYING:
      movePaddle(joystickX);
      updateBall();
      checkCollisions();
      drawGame(); // Dibujar el estado actual del juego

      if (currentGameState == GAME_OVER) { // La colisión con el suelo puede cambiar el estado
          delay(1000); // Esperar un poco antes de mostrar el mensaje
          displayGameOverScreen();
          // Esperar a que el botón sea liberado antes de esperar el siguiente press para reiniciar
          while(digitalRead(JOYSTICK_BTN_PIN) == LOW);
      } else if (currentGameState == GAME_WON) { // Si no fue game over, podría haber ganado
          delay(1000); // Esperar un poco
          displayGameWonScreen();
          // Esperar a que el botón sea liberado antes de esperar el siguiente press para reiniciar
          while(digitalRead(JOYSTICK_BTN_PIN) == LOW);
      }

      break;

    case GAME_OVER:
    case GAME_WON:
       // Esperar por el botón para reiniciar
      if (buttonPressed) {
        resetGame();
        currentGameState = WAITING_TO_START;
      }
      break;
  }

  // Mostrar todo lo dibujado en el buffer
  display.display();
}

void resetGame() {
  // Reiniciar paleta
  paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;

  // Reiniciar pelota
  ballX = paddleX + (PADDLE_WIDTH / 2) - (BALL_SIZE / 2);
  ballY = SCREEN_HEIGHT - PADDLE_HEIGHT - BALL_SIZE - 1;
  ballVX = 0;
  ballVY = 0;
  ballMoving = false;

  // Reiniciar ladrillos
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICKS_PER_ROW; j++) {
      bricks[i][j] = true; // Todos los ladrillos activos
    }
  }
  // Reiniciar estado del juego
  currentGameState = WAITING_TO_START;
}

void movePaddle(int joystickX) {
  // Mapear el valor del joystick (0-4095) a la posición X de la paleta (0 a SCREEN_WIDTH - PADDLE_WIDTH)
  // Ajusta el rango de mapeo para añadir sensibilidad o "dead zone" si es necesario
  paddleX = map(joystickX, 0, 4095, 0, SCREEN_WIDTH - PADDLE_WIDTH);

  // Asegurarse de que la paleta no salga de los límites
  paddleX = constrain(paddleX, 0, SCREEN_WIDTH - PADDLE_WIDTH);
}

void updateBall() {
  if (!ballMoving) return;

  // Mover la pelota
  ballX += ballVX;
  ballY += ballVY;

  // --- Colisiones con las paredes ---
  // Pared Izquierda/Derecha
  if (ballX <= 0 || ballX >= SCREEN_WIDTH - BALL_SIZE) {
    ballVX = -ballVX; // Invertir velocidad X
    // Ajustar posición para evitar que se pegue a la pared
    if (ballX < 0) ballX = 0;
    if (ballX > SCREEN_WIDTH - BALL_SIZE) ballX = SCREEN_WIDTH - BALL_SIZE;
  }

  // Pared Superior
  if (ballY <= 0) {
    ballVY = -ballVY; // Invertir velocidad Y
    ballY = 0; // Ajustar posición
  }

  // Pared Inferior (Fin del juego)
  if (ballY >= SCREEN_HEIGHT - BALL_SIZE) {
    currentGameState = GAME_OVER;
    ballMoving = false; // Detener la pelota
  }
}

void checkCollisions() {
   if (!ballMoving) return;

  // --- Colisión con la Paleta ---
  // Check si la pelota está a la altura de la paleta Y
  // Y si la pelota está horizontalmente dentro de la paleta
  if (ballY + BALL_SIZE >= SCREEN_HEIGHT - PADDLE_HEIGHT &&
      ballX + BALL_SIZE > paddleX &&
      ballX < paddleX + PADDLE_WIDTH) {

    // Si venía cayendo (VY positivo) y golpeó la paleta
    if (ballVY > 0) {
      ballVY = -ballVY; // Invertir velocidad Y

      // Opcional: Ajustar velocidad X basada en dónde golpeó la paleta
      float hitPos = (ballX + BALL_SIZE / 2.0) - (paddleX + PADDLE_WIDTH / 2.0); // -10 a +10 si PADDLE_WIDTH=20
      ballVX = hitPos * 0.1; // Ajusta el factor (0.1) para controlar cuánto afecta

      // Ajustar posición para evitar que se pegue a la paleta
      ballY = SCREEN_HEIGHT - PADDLE_HEIGHT - BALL_SIZE - 1;
    }
  }

  // --- Colisión con los Ladrillos ---
  bool allBricksDestroyed = true; // Bandera para comprobar si se ganó

  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICKS_PER_ROW; j++) {
      if (bricks[i][j]) { // Si el ladrillo está activo
        allBricksDestroyed = false; // Todavía quedan ladrillos

        // Calcular posición del ladrillo
        int brickX = BRICK_OFFSET_LEFT + j * (BRICK_WIDTH + BRICK_PADDING);
        int brickY = BRICK_OFFSET_TOP + i * (BRICK_HEIGHT + BRICK_PADDING);

        // Comprobar colisión entre la pelota y el ladrillo
        // Simplificado: AABB (Axis-Aligned Bounding Box) collision
        if (ballX < brickX + BRICK_WIDTH &&
            ballX + BALL_SIZE > brickX &&
            ballY < brickY + BRICK_HEIGHT &&
            ballY + BALL_SIZE > brickY) {

          // ¡Colisión detectada!
          bricks[i][j] = false; // Desactivar el ladrillo

          // Determinar la dirección del rebote (simple: solo invertir VY)
          // Una colisión más precisa determinaría si golpeó arriba/abajo (invertir VY)
          // o izquierda/derecha (invertir VX). Para simplicidad, solo VY.
          ballVY = -ballVY;

          // Opcional: Romper solo 1 ladrillo por frame para evitar rebotes múltiples extraños
          return; // Salir de la función de colisión después de la primera colisión
        }
      }
    }
  }

  // Comprobar si se ganó después de revisar todos los ladrillos
  if (allBricksDestroyed) {
    currentGameState = GAME_WON;
    ballMoving = false; // Detener la pelota
  }
}

void drawGame() {
  display.clearDisplay(); // Limpiar el buffer

  // Dibujar Paleta
  display.fillRect(paddleX, SCREEN_HEIGHT - PADDLE_HEIGHT, PADDLE_WIDTH, PADDLE_HEIGHT, SSD1306_WHITE);

  // Dibujar Pelota
  display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, SSD1306_WHITE); // Usamos fillRect para un cuadrado

  // Dibujar Ladrillos
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICKS_PER_ROW; j++) {
      if (bricks[i][j]) { // Si el ladrillo está activo, dibujarlo
        int brickX = BRICK_OFFSET_LEFT + j * (BRICK_WIDTH + BRICK_PADDING);
        int brickY = BRICK_OFFSET_TOP + i * (BRICK_HEIGHT + BRICK_PADDING);
        display.fillRect(brickX, brickY, BRICK_WIDTH, BRICK_HEIGHT, SSD1306_WHITE);
      }
    }
  }

  // NOTA: display.display() se llama al final del loop() principal
}

void displayWaitingScreen() {
  display.clearDisplay();
  display.setTextSize(1); // Tamaño de texto normal (1x)
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Breakout!"); // Nombre del juego

  // Ajustar posiciones Y para que quepan 3 líneas de texto tamaño 1
  display.setCursor(0, 10); // Mover abajo
  display.println("Mueve paddle (Joystick X)");

  display.setCursor(0, 20); // Mover abajo
  display.println("Pulsa Boton Inicio"); // Texto acortado

  display.setCursor(ballX, ballY); // Mostrar la pelota en la paleta en la pantalla de espera
  display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, SSD1306_WHITE);
  display.display();
}

void displayGameOverScreen() {
  display.clearDisplay();
  display.setTextSize(1); // Cambiado a tamaño 1 para que quepa
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, (SCREEN_HEIGHT / 2) - 10); // Posición ajustada
  display.println("FIN DEL JUEGO"); // Texto traducido

  display.setTextSize(1); // Ya está en tamaño 1, solo para claridad
  display.setCursor(0, (SCREEN_HEIGHT / 2) + 5); // Posición ajustada debajo del texto principal
  display.println("Pulsa Boton Reiniciar"); // Texto acortado

  display.display();
}

void displayGameWonScreen() {
  display.clearDisplay();
  display.setTextSize(1); // Cambiado a tamaño 1 para que quepa
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, (SCREEN_HEIGHT / 2) - 10); // Posición ajustada
  display.println("GANASTE!"); // Texto traducido

  display.setTextSize(1); // Ya está en tamaño 1, solo para claridad
  display.setCursor(0, (SCREEN_HEIGHT / 2) + 5); // Posición ajustada debajo del texto principal
  display.println("Pulsa Boton Reiniciar"); // Texto acortado

  display.display();
}