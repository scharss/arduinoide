#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Preferences.h>
#include <pgmspace.h>

// --- NO INCLUIR ASSETS EXTERNOS ---

// --- Configuración de Pines ---
#define JOYSTICK_Y_PIN 35
#define JOYSTICK_SW_PIN 25

// --- Configuración de la Pantalla ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences preferences;

//--------------------------------------------------------------------//
// GRAPHICS - ¡¡PLACEHOLDERS GEOMÉTRICOS SIMPLES!!                    //
// !!==============================================================!! //
// !!  REEMPLAZA ESTOS DATOS Y TAMAÑOS CON TUS GRÁFICOS XBM REALES   !! //
// !!==============================================================!! //
//--------------------------------------------------------------------//

// --- Oveja Placeholders (con "cabeza" añadida) ---
#define SHEEP_RUN_WIDTH 20
#define SHEEP_RUN_HEIGHT 18
const unsigned char sheep_run_1_bitmap[] PROGMEM = {
  SHEEP_RUN_WIDTH, SHEEP_RUN_HEIGHT, // Width, Height
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x0C, 0x0C, 0x00, 0x00,
  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char sheep_run_2_bitmap[] PROGMEM = {
  SHEEP_RUN_WIDTH, SHEEP_RUN_HEIGHT, // Width, Height
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x0C, 0x0C, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define SHEEP_FAIL_WIDTH SHEEP_RUN_WIDTH
#define SHEEP_FAIL_HEIGHT SHEEP_RUN_HEIGHT
// --- Esta es la única definición correcta ---
const unsigned char* sheep_fail_bitmap_ptr = sheep_run_1_bitmap; // Usa frame 1 como placeholder

// --- Arbusto Placeholder PELIGROSO ---
#define BUSH_HARMFUL_WIDTH 16
#define BUSH_HARMFUL_HEIGHT 20
const unsigned char bush_harmful_bitmap[] PROGMEM = {
  BUSH_HARMFUL_WIDTH, BUSH_HARMFUL_HEIGHT,
  0x00, 0x00, 0x00, 0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00
};

// --- Item de Puntos Placeholder (Círculo pequeño) ---
#define POINT_ITEM_WIDTH 8
#define POINT_ITEM_HEIGHT 8
const unsigned char point_item_bitmap[] PROGMEM = {
  POINT_ITEM_WIDTH, POINT_ITEM_HEIGHT,
  0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x18
};

// --- Suelo, Game Over, etc. (Placeholders) ---
#define GROUND_WIDTH 12
#define GROUND_HEIGHT 6
#define GROUND_Y (SCREEN_HEIGHT - GROUND_HEIGHT + 1)
const unsigned char ground_placeholder_bitmap[] PROGMEM = {GROUND_WIDTH, GROUND_HEIGHT, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
const unsigned char* ground_bm_ptr = ground_placeholder_bitmap;
#define GAME_OVER_WIDTH 94
#define GAME_OVER_HEIGHT 12
const unsigned char game_over_placeholder_bitmap[] PROGMEM = {GAME_OVER_WIDTH, GAME_OVER_HEIGHT, 0x00};
#define HEARTS_WIDTH 5
#define HEARTS_HEIGHT 5
#define COIN_ICON_X 10
#define COIN_ICON_Y 2
#define COIN_ICON_RADIUS 3

// --- Constantes del Juego ---
#define SHEEP_X 5
#define SHEEP_FLOOR_Y (GROUND_Y - SHEEP_RUN_HEIGHT)
#define GRAVITY 0.50f
#define JUMP_FORCE -5.5f
#define INITIAL_GAME_SPEED 3.0f
#define MAX_GAME_SPEED 10.0f
#define SPEED_INCREMENT_FACTOR 0.0005f
#define MAX_OBSTACLES 3
#define OBSTACLE_MIN_SPAWN_DIST 50
#define OBSTACLE_RAND_SPAWN_DIST 100
#define LIVES_START 3
#define LIVES_MAX 5
#define POINTS_PER_ITEM 1
#define HIGH_ITEM_PROBABILITY 40

// --- Estado del Juego ---
enum GameState { MENU, PLAYING, GAME_OVER };
GameState gameState = MENU;
enum ObstacleType { BUSH_HARMFUL, POINT_ITEM };
struct Obstacle { int x, y, width, height; ObstacleType type; bool active; };
Obstacle obstacles[MAX_OBSTACLES];

// --- Variables del Juego ---
float sheepY, sheepVelY; bool sheepIsOnGround;
int currentSheepFrame; unsigned long lastSheepFrameChange;
int sheepAnimRunDelay = 130;
float gameSpeed; unsigned long score, highScore = 0; unsigned long lastScoreUpdate;
int lives = LIVES_START;
int coinsCollected = 0;
int groundX[12];

// --- Arrays de Punteros a Bitmaps ---
const unsigned char* sheep_run_bitmaps[] = {sheep_run_1_bitmap, sheep_run_2_bitmap};
// La línea duplicada fue eliminada

// --- PROTOTIPOS ---
void drawScoreboard();
void drawGround();

// --- Funciones del Juego --- (Sin cambios desde la versión anterior)

void resetGame() {
    sheepY = SHEEP_FLOOR_Y; sheepVelY = 0; sheepIsOnGround = true;
    currentSheepFrame = 0; lastSheepFrameChange = millis();
    for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
    score = 0; lastScoreUpdate = millis(); gameSpeed = INITIAL_GAME_SPEED; lives = LIVES_START;
    coinsCollected = 0;
    gameState = PLAYING;
}

void spawnObstacle() {
    int lastObstacleX = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++) { if (obstacles[i].active && obstacles[i].x + obstacles[i].width > lastObstacleX) lastObstacleX = obstacles[i].x + obstacles[i].width; }
    int nextObstacleDist = OBSTACLE_MIN_SPAWN_DIST + random(0, OBSTACLE_RAND_SPAWN_DIST);
    nextObstacleDist += (int)(gameSpeed * 3.0f);
    if (SCREEN_WIDTH - lastObstacleX >= nextObstacleDist) {
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!obstacles[i].active) {
                obstacles[i].active = true; obstacles[i].x = SCREEN_WIDTH;
                if (random(0, 10) < 7) {
                   obstacles[i].type = BUSH_HARMFUL; obstacles[i].width = BUSH_HARMFUL_WIDTH; obstacles[i].height = BUSH_HARMFUL_HEIGHT;
                   obstacles[i].y = GROUND_Y - obstacles[i].height;
                } else {
                   obstacles[i].type = POINT_ITEM; obstacles[i].width = POINT_ITEM_WIDTH; obstacles[i].height = POINT_ITEM_HEIGHT;
                   if (random(0, 100) < HIGH_ITEM_PROBABILITY) { obstacles[i].y = SHEEP_FLOOR_Y - SHEEP_RUN_HEIGHT / 2 - POINT_ITEM_HEIGHT; }
                   else { obstacles[i].y = GROUND_Y - obstacles[i].height - random(0, 5); }
                   obstacles[i].y = max(0, obstacles[i].y); obstacles[i].y = min(obstacles[i].y, GROUND_Y - obstacles[i].height);
                } break;
            }
        }
    }
}

bool checkCollision() {
    int sheepH = SHEEP_RUN_HEIGHT; int sheepW = SHEEP_RUN_WIDTH;
    int sheepMarginX = 3; int sheepMarginY = 3;
    int sheepLeft = SHEEP_X + sheepMarginX; int sheepRight = SHEEP_X + sheepW - sheepMarginX;
    int sheepTop = (int)sheepY + sheepMarginY; int sheepBottom = (int)sheepY + sheepH - sheepMarginY;
    sheepBottom = min(sheepBottom, GROUND_Y - 1);
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            int obsMarginX = 1; int obsMarginY = 1;
            int obsLeft = obstacles[i].x + obsMarginX; int obsRight = obstacles[i].x + obstacles[i].width - obsMarginX;
            int obsTop = obstacles[i].y + obsMarginY; int obsBottom = obstacles[i].y + obstacles[i].height - obsMarginY;
            if (sheepRight > obsLeft && sheepLeft < obsRight && sheepBottom > obsTop && sheepTop < obsBottom) {
                if (obstacles[i].type == BUSH_HARMFUL) {
                     lives--; obstacles[i].active = false;
                     Serial.printf("Collision with bush! Lives: %d\n", lives);
                     if (lives <= 0) { Serial.println("GAME OVER condition met in checkCollision."); return true; }
                     return false;
                } else if (obstacles[i].type == POINT_ITEM) {
                     coinsCollected += POINTS_PER_ITEM; score += POINTS_PER_ITEM; obstacles[i].active = false;
                     Serial.printf("Coin collected! Total: %d\n", coinsCollected);
                     return false;
                }
            }
        }
    } return false;
}

// --- Funciones de Dibujo ---

void drawGround() {
     for(int i=0; i<12; i++) { display.drawXBitmap(groundX[i], GROUND_Y, ground_bm_ptr + 2, GROUND_WIDTH, GROUND_HEIGHT, SSD1306_WHITE); }
}

void drawScoreboard() {
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE); display.setFont();
    display.setCursor(SCREEN_WIDTH - 65, 2); display.print(F("HI ")); char hiScoreStr[7]; sprintf(hiScoreStr, "%05lu", highScore); display.print(hiScoreStr);
    display.setCursor(SCREEN_WIDTH - 30, 2); char scoreStr[7]; sprintf(scoreStr, "%05lu", score); display.print(scoreStr);
    display.drawCircle(COIN_ICON_X, COIN_ICON_Y + HEARTS_HEIGHT / 2, COIN_ICON_RADIUS, SSD1306_WHITE);
    display.setCursor(COIN_ICON_X + COIN_ICON_RADIUS*2 + 2 , COIN_ICON_Y); display.print(coinsCollected);
    int heartsToShow = max(0, lives);
    for(int i=0; i<heartsToShow; ++i) { display.fillRect(SCREEN_WIDTH - 75 - (i*7), 10, 5, 5, SSD1306_WHITE); }
}

void drawGame() {
    drawGround(); drawScoreboard();
    const unsigned char* sheepBitmap; int sheepDrawW, sheepDrawH; unsigned long currentTime = millis();
    if (!sheepIsOnGround) { sheepBitmap = sheep_run_bitmaps[0]; sheepDrawW = SHEEP_RUN_WIDTH; sheepDrawH = SHEEP_RUN_HEIGHT; }
    else { if (currentTime - lastSheepFrameChange > sheepAnimRunDelay) { currentSheepFrame = 1 - currentSheepFrame; lastSheepFrameChange = currentTime; }
        sheepBitmap = sheep_run_bitmaps[currentSheepFrame]; sheepDrawW = SHEEP_RUN_WIDTH; sheepDrawH = SHEEP_RUN_HEIGHT; }
    int drawYSheep = max(0, (int)sheepY); drawYSheep = min(SCREEN_HEIGHT - sheepDrawH, drawYSheep);
    display.drawXBitmap(SHEEP_X, drawYSheep, sheepBitmap + 2, sheepDrawW, sheepDrawH, SSD1306_WHITE);
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            const unsigned char* itemBitmapToDraw = nullptr; int itemWidthToDraw = 0; int itemHeightToDraw = 0;
            if (obstacles[i].type == BUSH_HARMFUL) { itemBitmapToDraw = bush_harmful_bitmap; itemWidthToDraw = BUSH_HARMFUL_WIDTH; itemHeightToDraw = BUSH_HARMFUL_HEIGHT; }
            else { itemBitmapToDraw = point_item_bitmap; itemWidthToDraw = POINT_ITEM_WIDTH; itemHeightToDraw = POINT_ITEM_HEIGHT; }
            int drawX = obstacles[i].x; int drawY = obstacles[i].y;
            if (itemBitmapToDraw != nullptr && drawX < SCREEN_WIDTH && drawX + itemWidthToDraw > 0) {
                display.drawXBitmap(drawX, drawY, itemBitmapToDraw + 2, itemWidthToDraw, itemHeightToDraw, SSD1306_WHITE);
            }
        }
    }
}

void drawMenu() {
    display.setFont(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.setCursor(35, 15); display.print(F("Jumper"));
    display.setCursor(10, 35); display.print(F("Pulsa Arriba/Boton"));
    display.setCursor(25, 48); display.print(F("para empezar"));
    drawScoreboard();
}

void drawGameOver() {
    drawGround(); drawScoreboard();
    int drawYSheep = max(0, (int)sheepY); drawYSheep = min(SHEEP_FLOOR_Y, drawYSheep);
    display.drawXBitmap(SHEEP_X, drawYSheep, sheep_fail_bitmap_ptr + 2, SHEEP_FAIL_WIDTH, SHEEP_FAIL_HEIGHT, SSD1306_WHITE);
    display.setTextSize(2); display.setCursor(15, 25); display.print(F("GAME OVER"));
}

// --- Setup ---
void setup() {
    Serial.begin(115200); Serial.println(F("Iniciando Jumper..."));
    pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP); pinMode(JOYSTICK_Y_PIN, INPUT);
    preferences.begin("jumperGame", false);
    highScore = preferences.getULong("highScore", 0); Serial.printf("High Score cargado: %lu\n", highScore);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { Serial.println(F("Fallo OLED")); while(1); }
    Serial.println(F("OLED OK")); display.clearDisplay(); display.display(); delay(100);
    for(int i=0; i<12; i++) { groundX[i] = i * GROUND_WIDTH; }
    randomSeed(analogRead(A0));
    gameState = MENU;
}

// --- Loop Principal ---
void loop() {
    unsigned long currentTime = millis(); bool jumpPressed = false;
    int joyYValue = analogRead(JOYSTICK_Y_PIN); bool joySWPressed = (digitalRead(JOYSTICK_SW_PIN) == LOW);
    if (joyYValue < 500 || joySWPressed) { jumpPressed = true; }

    switch (gameState) {
        case MENU: if (jumpPressed) resetGame(); break;
        case PLAYING:
            sheepVelY += GRAVITY; sheepY += sheepVelY;
            if (sheepY >= SHEEP_FLOOR_Y) { sheepY = SHEEP_FLOOR_Y; sheepVelY = 0; if (!sheepIsOnGround) { sheepIsOnGround = true; currentSheepFrame = 0; lastSheepFrameChange = currentTime; } }
            else { sheepIsOnGround = false; }
            if (jumpPressed && sheepIsOnGround) { sheepVelY = JUMP_FORCE; sheepIsOnGround = false; }
            for (int i = 0; i < MAX_OBSTACLES; i++) { if (obstacles[i].active) { obstacles[i].x -= gameSpeed; if (obstacles[i].x + obstacles[i].width < 0) obstacles[i].active = false; } }
            spawnObstacle();
            for(int i=0; i<12; i++) { groundX[i] -= gameSpeed; if (groundX[i] <= -GROUND_WIDTH) { int max_x = -GROUND_WIDTH; for(int j=0; j<12; j++) { if(groundX[j] > max_x) max_x = groundX[j]; } groundX[i] = max_x + GROUND_WIDTH -1 ; } }
            gameSpeed = INITIAL_GAME_SPEED + (score * SPEED_INCREMENT_FACTOR); gameSpeed = min(gameSpeed, MAX_GAME_SPEED);
            if (score == 0 || currentTime - lastScoreUpdate > (1000 / (int)(max(1.0f, gameSpeed) * 5.0f))) { if (gameState == PLAYING) score++; lastScoreUpdate = currentTime; }
            if (checkCollision()) { if (lives <= 0) { gameState = GAME_OVER; if (score > highScore) { highScore = score; preferences.putULong("highScore", highScore); } } }
            break;
        case GAME_OVER:
            static unsigned long gameOverEntryTime = 0; if (gameOverEntryTime == 0) gameOverEntryTime = millis();
            if (jumpPressed && (millis() - gameOverEntryTime > 500) ) { gameState = MENU; gameOverEntryTime = 0; } break;
    }

    display.clearDisplay();
    switch(gameState) { case MENU: drawMenu(); break; case PLAYING: drawGame(); break; case GAME_OVER: drawGameOver(); break; }
    display.display();
    delay(5);
}