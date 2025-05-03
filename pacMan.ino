#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

// Configuración de pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    16
#define OLED_DC       17
#define OLED_CS       5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// Pines de los botones
#define UP_BUTTON 32    // Botón 1: Arriba
#define DOWN_BUTTON 33  // Botón 2: Abajo
#define LEFT_BUTTON 25  // Botón 3: Izquierda
#define RIGHT_BUTTON 26 // Botón 4: Derecha

// Declaración de funciones
void resetPoints();
void resetPacmanPosition();
void spawnFruit();
void spawnGhosts();
void moveGhosts();
void collectPoints();
void collectFruit();
bool checkCollisionWithGhosts();
void eatGhost();
bool checkAllPointsCollected();
void nextLevel();
void loseLife();
void drawGhost(int x, int y, bool eatable);
void drawGame();
void displayGameOver();

// Variables del juego
int pacmanX = 0, pacmanY = 0; // Posición inicial de Pac-Man
int pacmanDir = 0; // Dirección de Pac-Man: 0=Derecha, 1=Izquierda, 2=Arriba, 3=Abajo
bool pacmanMouthOpen = true; // Estado de la boca
int fruitX = -1, fruitY = -1; // Posición de la fruta
const int maxGhosts = 10; // Número máximo de fantasmas
int ghosts[maxGhosts][2]; // Posiciones de los fantasmas
int ghostDirs[maxGhosts][2]; // Direcciones de los fantasmas
int ghostCount = 3; // Número inicial de fantasmas
bool ghostsEatable = false; // Estado de vulnerabilidad de los fantasmas
int eatableTimer = 0; // Temporizador para el estado comestible
bool points[16][8]; // Matriz de puntos
int score = 0; // Puntuación del jugador
int level = 1; // Nivel actual
int ghostSpeed = 1; // Velocidad inicial de los fantasmas
int lives = 3; // Vidas iniciales de Pac-Man

void setup() {
  // Inicializar la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("Error al inicializar la pantalla OLED"));
    for (;;);
  }
  display.clearDisplay();

  // Configurar botones con resistencias pull-up internas
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);

  // Inicializar puntos en el laberinto
  resetPoints();
  spawnFruit(); // Generar la fruta inicial
  spawnGhosts(); // Generar los fantasmas iniciales
  resetPacmanPosition(); // Colocar a Pac-Man lejos de los fantasmas
}

void loop() {
  // Leer los botones para mover a Pac-Man
  if (!digitalRead(UP_BUTTON) && pacmanY > 0) {
    pacmanY -= 2;
    pacmanDir = 2; // Arriba
  }
  if (!digitalRead(DOWN_BUTTON) && pacmanY < SCREEN_HEIGHT - 8) {
    pacmanY += 2;
    pacmanDir = 3; // Abajo
  }
  if (!digitalRead(LEFT_BUTTON) && pacmanX > 0) {
    pacmanX -= 2;
    pacmanDir = 1; // Izquierda
  }
  if (!digitalRead(RIGHT_BUTTON) && pacmanX < SCREEN_WIDTH - 8) {
    pacmanX += 2;
    pacmanDir = 0; // Derecha
  }

  // Alternar la animación de la boca
  pacmanMouthOpen = !pacmanMouthOpen;

  // Mover a los fantasmas
  moveGhosts();

  // Detectar colisiones con los puntos
  collectPoints();

  // Detectar colisión con la fruta
  collectFruit();

  // Detectar colisión con los fantasmas
  if (checkCollisionWithGhosts()) {
    if (ghostsEatable) {
      eatGhost(); // Comer fantasma si es comestible
    } else {
      loseLife(); // Perder una vida
      if (lives <= 0) {
        displayGameOver(); // Mostrar GAME OVER
        delay(3000); // Esperar 3 segundos antes de reiniciar
        setup(); // Reiniciar el juego
        return;
      } else {
        resetPacmanPosition(); // Reiniciar posición de Pac-Man
      }
    }
  }

  // Verificar si todos los puntos fueron recolectados
  if (checkAllPointsCollected()) {
    nextLevel(); // Subir al siguiente nivel
  }

  // Temporizador para el estado comestible de los fantasmas
  if (ghostsEatable) {
    eatableTimer--;
    if (eatableTimer <= 0) {
      ghostsEatable = false; // Fin del estado comestible
    }
  }

  // Dibujar el juego
  drawGame();
  delay(100); // Controlar la velocidad del juego
}

void resetPoints() {
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 8; j++) {
      points[i][j] = true; // Todos los puntos están activos
    }
  }
}

void resetPacmanPosition() {
  pacmanX = (random(0, 2) == 0) ? 0 : SCREEN_WIDTH - 8; // Extremo izquierdo o derecho
  pacmanY = random(0, SCREEN_HEIGHT / 8) * 8; // Posición vertical aleatoria
}

void spawnFruit() {
  fruitX = (random(0, 16)) * 8;
  fruitY = (random(0, 8)) * 8;
}

void spawnGhosts() {
  for (int i = 0; i < ghostCount; i++) {
    ghosts[i][0] = random(8, SCREEN_WIDTH - 16); // Posición X aleatoria
    ghosts[i][1] = random(8, SCREEN_HEIGHT - 16); // Posición Y aleatoria
    ghostDirs[i][0] = (random(0, 2) == 0) ? 1 : -1; // Dirección X aleatoria
    ghostDirs[i][1] = (random(0, 2) == 0) ? 1 : -1; // Dirección Y aleatoria
  }
}

void moveGhosts() {
  for (int i = 0; i < ghostCount; i++) {
    ghosts[i][0] += ghostDirs[i][0] * ghostSpeed;
    ghosts[i][1] += ghostDirs[i][1] * ghostSpeed;

    // Cambiar dirección si chocan con los bordes
    if (ghosts[i][0] <= 0 || ghosts[i][0] >= SCREEN_WIDTH - 8) ghostDirs[i][0] = -ghostDirs[i][0];
    if (ghosts[i][1] <= 0 || ghosts[i][1] >= SCREEN_HEIGHT - 8) ghostDirs[i][1] = -ghostDirs[i][1];
  }
}

void collectPoints() {
  int gridX = pacmanX / 8;
  int gridY = pacmanY / 8;
  if (points[gridX][gridY]) {
    points[gridX][gridY] = false;
    score++;
  }
}

void collectFruit() {
  if (pacmanX == fruitX && pacmanY == fruitY) {
    score += 10; // Puntos adicionales por la fruta
    ghostsEatable = true; // Los fantasmas se vuelven comestibles
    eatableTimer = 50; // 5 segundos de estado comestible
    spawnFruit(); // Generar nueva fruta
  }
}

bool checkCollisionWithGhosts() {
  for (int i = 0; i < ghostCount; i++) {
    if (pacmanX < ghosts[i][0] + 8 && pacmanX + 8 > ghosts[i][0] &&
        pacmanY < ghosts[i][1] + 8 && pacmanY + 8 > ghosts[i][1]) {
      return true;
    }
  }
  return false;
}

void eatGhost() {
  for (int i = 0; i < ghostCount; i++) {
    if (pacmanX < ghosts[i][0] + 8 && pacmanX + 8 > ghosts[i][0] &&
        pacmanY < ghosts[i][1] + 8 && pacmanY + 8 > ghosts[i][1]) {
      ghosts[i][0] = -10; // Enviar al fantasma fuera de la pantalla temporalmente
      ghosts[i][1] = -10;
      score += 20; // Puntos por comer un fantasma
    }
  }
}

bool checkAllPointsCollected() {
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 8; j++) {
      if (points[i][j]) return false;
    }
  }
  return true;
}

void nextLevel() {
  level++;
  ghostSpeed++; // Incrementar velocidad de los fantasmas
  ghostCount = min(ghostCount + 1, maxGhosts); // Incrementar número de fantasmas
  resetPoints(); // Reiniciar puntos
  spawnGhosts(); // Generar nuevos fantasmas
  resetPacmanPosition(); // Reiniciar posición de Pac-Man
  spawnFruit(); // Generar nueva fruta
}

void loseLife() {
  lives--;
}

void drawGhost(int x, int y, bool eatable) {
  int radius = 4; // Radio del semicírculo
  int baseY = y + radius; // Posición vertical de la base del fantasma

  // Dibujar la cabeza del fantasma (semicírculo)
  display.fillCircle(x + radius, y + radius, radius, SSD1306_WHITE);
  display.fillRect(x, y + radius, radius * 2, radius, SSD1306_WHITE); // Conectar con las patas

  // Dibujar las patas (ondulaciones)
  for (int i = 0; i < 4; i++) {
    int startX = x + i * 2; // Espaciado entre las patas
    int startY = baseY + ((i % 2 == 0) ? 1 : -1); // Alternar arriba/abajo
    display.drawLine(startX, baseY, startX + 2, startY, SSD1306_WHITE);
  }

  // Dibujar los ojos
  int eyeOffsetX = 1; // Desplazamiento horizontal de los ojos
  int eyeOffsetY = -1; // Desplazamiento vertical de los ojos
  int eyeRadius = 1; // Tamaño de los ojos

  display.fillCircle(x + radius - eyeOffsetX, y + radius + eyeOffsetY, eyeRadius, SSD1306_BLACK);
  display.fillCircle(x + radius + eyeOffsetX, y + radius + eyeOffsetY, eyeRadius, SSD1306_BLACK);

  // Si el fantasma es comestible, dibujar un borde
  if (eatable) {
    display.drawCircle(x + radius, y + radius, radius + 2, SSD1306_WHITE);
  }
}

void drawGame() {
  display.clearDisplay();

  // Dibujar a Pac-Man
  if (pacmanMouthOpen) {
    switch (pacmanDir) {
      case 0: // Derecha
        display.fillCircle(pacmanX + 4, pacmanY + 4, 4, SSD1306_WHITE);
        display.fillTriangle(pacmanX + 4, pacmanY + 4, pacmanX + 8, pacmanY, pacmanX + 8, pacmanY + 8, SSD1306_BLACK);
        break;
      case 1: // Izquierda
        display.fillCircle(pacmanX + 4, pacmanY + 4, 4, SSD1306_WHITE);
        display.fillTriangle(pacmanX + 4, pacmanY + 4, pacmanX, pacmanY, pacmanX, pacmanY + 8, SSD1306_BLACK);
        break;
      case 2: // Arriba
        display.fillCircle(pacmanX + 4, pacmanY + 4, 4, SSD1306_WHITE);
        display.fillTriangle(pacmanX + 4, pacmanY + 4, pacmanX, pacmanY, pacmanX + 8, pacmanY, SSD1306_BLACK);
        break;
      case 3: // Abajo
        display.fillCircle(pacmanX + 4, pacmanY + 4, 4, SSD1306_WHITE);
        display.fillTriangle(pacmanX + 4, pacmanY + 4, pacmanX, pacmanY + 8, pacmanX + 8, pacmanY + 8, SSD1306_BLACK);
        break;
    }
  } else {
    display.fillCircle(pacmanX + 4, pacmanY + 4, 4, SSD1306_WHITE);
  }

  // Dibujar fantasmas
  for (int i = 0; i < ghostCount; i++) {
    drawGhost(ghosts[i][0], ghosts[i][1], ghostsEatable);
  }

  // Dibujar puntos
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 8; j++) {
      if (points[i][j]) {
        display.drawPixel(i * 8 + 4, j * 8 + 4, SSD1306_WHITE);
      }
    }
  }

  // Dibujar fruta
  if (fruitX > -1 && fruitY > -1) {
    display.fillCircle(fruitX + 4, fruitY + 4, 3, SSD1306_WHITE);
  }

  // Dibujar puntuación, nivel y vidas
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Score: ");
  display.print(score);
  display.setCursor(80, 0);
  display.print("Level: ");
  display.print(level);
  display.setCursor(0, 56);
  display.print("Lives: ");
  display.print(lives);

  display.display();
}

void displayGameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("GAME OVER");
  display.display();
  delay(3000); // Esperar 3 segundos antes de reiniciar
}
