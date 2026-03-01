// content10.h
// Tetris Animation 

// manchmal wenn anderes content vorher aufgerufen wurde funktioniert das clear des displays am anfang nicht richtig

#ifndef CONTENT10_H
#define CONTENT10_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Tetris Spielfeld - nutzt den kompletten 128x64 Display
const int TETRIS_FIELD_WIDTH = 32;   // 32 Blöcke breit (4 Pixel pro Block)
const int TETRIS_FIELD_HEIGHT = 16;  // 16 Blöcke hoch (4 Pixel pro Block)
const int TETRIS_BLOCK_SIZE = 4;     // 4x4 Pixel pro Tetris-Block
const int TETRIS_FIELD_OFFSET_X = 0; // Nutze kompletten Bildschirm
const int TETRIS_FIELD_OFFSET_Y = 0;

// Tetromino Formen (4x4 Matrix für jede Form)
const bool TETRIS_TETROMINOS[7][4][4][4] = {
  // I-Block
  {{{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
   {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
   {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
   {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}},
  
  // O-Block
  {{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
   {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
   {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
   {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}},
  
  // T-Block
  {{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
   {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
   {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
   {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
  
  // S-Block
  {{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
   {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
   {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
   {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}}},
  
  // Z-Block
  {{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
   {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
   {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
   {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}},
  
  // J-Block
  {{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
   {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
   {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
   {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}},
  
  // L-Block
  {{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
   {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
   {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
   {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}}
};

// Farben für die Tetrominos
const uint16_t TETRIS_COLORS[7][3] = {
  {0, 255, 255},    // I - Cyan
  {255, 255, 0},    // O - Gelb
  {128, 0, 128},    // T - Lila
  {0, 255, 0},      // S - Grün
  {255, 0, 0},      // Z - Rot
  {0, 0, 255},      // J - Blau
  {255, 165, 0}     // L - Orange
};

struct TetrisFallingBlock {
  int x, y;
  int type;
  int rotation;
  bool active;
  float fallSpeed;
  unsigned long lastFall;
  bool aiControlled;
  int targetX;
  int targetRotation;
};

struct TetrisLineClearEffect {
  int line;
  bool active;
  unsigned long startTime;
  int phase; // 0=flash, 1=clear
};

struct AIMove {
  int x;
  int rotation;
  float score;
};

static uint8_t tetrisPlayField[TETRIS_FIELD_HEIGHT][TETRIS_FIELD_WIDTH]; // 0=leer, 1-7=Tetromino-Typ
static TetrisFallingBlock tetrisCurrentBlock;
static unsigned long tetrisLastUpdate = 0;
static unsigned long tetrisLastSpawn = 0;
static TetrisLineClearEffect tetrisLineEffect;
static int tetrisScore = 0;
static int tetrisLevel = 1;
static bool gameRunning = true;

// Standalone test function
void testContent10() {
  Serial.println("Content10 Test: Spektakuläre Tetris-Animation mit KI wird gestartet!");
}

// Forward declarations
AIMove calculateBestMove();

void initTetris() {
  // Spielfeld leeren
  for (int y = 0; y < TETRIS_FIELD_HEIGHT; y++) {
    for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
      tetrisPlayField[y][x] = 0;
    }
  }
  
  tetrisCurrentBlock.active = false;
  tetrisLineEffect.active = false;
  tetrisScore = 0;
  tetrisLevel = 1;
  gameRunning = true;
}

bool checkGameOver() {
  // Prüfe ob die oberste Reihe (außer ganz oben) belegt ist
  for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
    if (tetrisPlayField[1][x] != 0) { // Zweite Reihe von oben
      return true;
    }
  }
  return false;
}

void spawnNewTetrisBlock() {
  tetrisCurrentBlock.x = TETRIS_FIELD_WIDTH / 2 - 2;
  tetrisCurrentBlock.y = 0;
  tetrisCurrentBlock.type = random(0, 7);
  tetrisCurrentBlock.rotation = 0;
  tetrisCurrentBlock.active = true;
  tetrisCurrentBlock.fallSpeed = max(50, 300 - (tetrisLevel * 20)); // Viel schneller
  tetrisCurrentBlock.lastFall = millis();
  tetrisCurrentBlock.aiControlled = true;
  
  // KI berechnet sofort die beste Position
  AIMove bestMove = calculateBestMove();
  tetrisCurrentBlock.targetX = bestMove.x;
  tetrisCurrentBlock.targetRotation = bestMove.rotation;
}

bool canPlaceTetrisBlock(int x, int y, int type, int rotation) {
  for (int by = 0; by < 4; by++) {
    for (int bx = 0; bx < 4; bx++) {
      if (TETRIS_TETROMINOS[type][rotation][by][bx]) {
        int fx = x + bx;
        int fy = y + by;
        
        // Prüfe Grenzen
        if (fx < 0 || fx >= TETRIS_FIELD_WIDTH || fy >= TETRIS_FIELD_HEIGHT) {
          return false;
        }
        
        // Prüfe Kollision mit anderen Blöcken (aber nicht oberhalb des Spielfelds)
        if (fy >= 0 && tetrisPlayField[fy][fx] != 0) {
          return false;
        }
      }
    }
  }
  return true;
}

void placeTetrisBlock() {
  for (int by = 0; by < 4; by++) {
    for (int bx = 0; bx < 4; bx++) {
      if (TETRIS_TETROMINOS[tetrisCurrentBlock.type][tetrisCurrentBlock.rotation][by][bx]) {
        int fx = tetrisCurrentBlock.x + bx;
        int fy = tetrisCurrentBlock.y + by;
        
        if (fx >= 0 && fx < TETRIS_FIELD_WIDTH && fy >= 0 && fy < TETRIS_FIELD_HEIGHT) {
          tetrisPlayField[fy][fx] = tetrisCurrentBlock.type + 1;
        }
      }
    }
  }
  tetrisCurrentBlock.active = false;
}

void checkTetrisLines() {
  for (int y = TETRIS_FIELD_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;
    for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
      if (tetrisPlayField[y][x] == 0) {
        fullLine = false;
        break;
      }
    }
    
    if (fullLine) {
      // Starte Linien-Clear-Effekt
      tetrisLineEffect.line = y;
      tetrisLineEffect.active = true;
      tetrisLineEffect.startTime = millis();
      tetrisLineEffect.phase = 0;
      tetrisScore += 100;
      return; // Nur eine Linie auf einmal für besseren visuellen Effekt
    }
  }
}

void clearTetrisLine(int line) {
  // Bewege alle Linien darüber nach unten
  for (int y = line; y > 0; y--) {
    for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
      tetrisPlayField[y][x] = tetrisPlayField[y-1][x];
    }
  }
  
  // Oberste Linie leeren
  for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
    tetrisPlayField[0][x] = 0;
  }
  
  // Level erhöhen alle 10 Linien
  if ((tetrisScore / 100) % 10 == 0 && tetrisScore > 0) {
    tetrisLevel++;
  }
}

// KI-Bewertungsfunktion
float evaluatePosition(int x, int y, int type, int rotation) {
  // Simuliere das Platzieren des Blocks
  uint8_t tempField[TETRIS_FIELD_HEIGHT][TETRIS_FIELD_WIDTH];
  memcpy(tempField, tetrisPlayField, sizeof(tetrisPlayField));
  
  // Platziere Block in temporärem Feld
  for (int by = 0; by < 4; by++) {
    for (int bx = 0; bx < 4; bx++) {
      if (TETRIS_TETROMINOS[type][rotation][by][bx]) {
        int fx = x + bx;
        int fy = y + by;
        if (fx >= 0 && fx < TETRIS_FIELD_WIDTH && fy >= 0 && fy < TETRIS_FIELD_HEIGHT) {
          tempField[fy][fx] = type + 1;
        }
      }
    }
  }
  
  float score = 0;
  
  // 1. Linien vervollständigen (höchste Priorität)
  int completedLines = 0;
  for (int row = 0; row < TETRIS_FIELD_HEIGHT; row++) {
    bool fullLine = true;
    for (int col = 0; col < TETRIS_FIELD_WIDTH; col++) {
      if (tempField[row][col] == 0) {
        fullLine = false;
        break;
      }
    }
    if (fullLine) completedLines++;
  }
  score += completedLines * 2000; // Sehr hoch bewerten
  
  // 2. Spalten-Höhen berechnen
  int heights[TETRIS_FIELD_WIDTH];
  for (int col = 0; col < TETRIS_FIELD_WIDTH; col++) {
    heights[col] = TETRIS_FIELD_HEIGHT;
    for (int row = 0; row < TETRIS_FIELD_HEIGHT; row++) {
      if (tempField[row][col] != 0) {
        heights[col] = row;
        break;
      }
    }
  }
  
  // 3. Löcher stark bestrafen
  int holes = 0;
  for (int col = 0; col < TETRIS_FIELD_WIDTH; col++) {
    for (int row = heights[col] + 1; row < TETRIS_FIELD_HEIGHT; row++) {
      if (tempField[row][col] == 0) {
        holes++;
      }
    }
  }
  score -= holes * 200; // Sehr stark bestrafen
  
  // 4. Höhenunterschiede (Bumpiness) minimieren
  int bumpiness = 0;
  for (int col = 0; col < TETRIS_FIELD_WIDTH - 1; col++) {
    bumpiness += abs(heights[col] - heights[col + 1]);
  }
  score -= bumpiness * 15; // Stark bestrafen
  
  // 5. Gesamthöhe niedrig halten
  int totalHeight = 0;
  for (int col = 0; col < TETRIS_FIELD_WIDTH; col++) {
    totalHeight += TETRIS_FIELD_HEIGHT - heights[col];
  }
  score -= totalHeight * 3;
  
  // 6. Zentrale Positionen leicht bevorzugen (gegen Rand-Bias)
  int center = TETRIS_FIELD_WIDTH / 2;
  int distanceFromCenter = abs(x - center);
  score -= distanceFromCenter * 0.5;
  
  // 7. Tiefere Platzierung bevorzugen
  score += (TETRIS_FIELD_HEIGHT - y) * 5;
  
  // 8. Flache Strukturen bevorzugen
  int maxHeight = 0;
  for (int col = 0; col < TETRIS_FIELD_WIDTH; col++) {
    int colHeight = TETRIS_FIELD_HEIGHT - heights[col];
    if (colHeight > maxHeight) maxHeight = colHeight;
  }
  score -= maxHeight * 10;
  
  return score;
}

AIMove calculateBestMove() {
  AIMove bestMove = {0, 0, -999999};
  
  // Teste alle möglichen Positionen und Rotationen
  for (int rotation = 0; rotation < 4; rotation++) {
    for (int x = -3; x < TETRIS_FIELD_WIDTH + 3; x++) {
      // Finde die tiefste mögliche Position
      int dropY = 0;
      for (int y = 0; y < TETRIS_FIELD_HEIGHT; y++) {
        if (canPlaceTetrisBlock(x, y, tetrisCurrentBlock.type, rotation)) {
          dropY = y;
        } else {
          break;
        }
      }
      
      // Bewerte diese Position
      if (canPlaceTetrisBlock(x, dropY, tetrisCurrentBlock.type, rotation)) {
        float score = evaluatePosition(x, dropY, tetrisCurrentBlock.type, rotation);
        if (score > bestMove.score) {
          bestMove.x = x;
          bestMove.rotation = rotation;
          bestMove.score = score;
        }
      }
    }
  }
  
  return bestMove;
}

void updateTetris() {
  unsigned long now = millis();
  
  // Game Over Check
  if (checkGameOver()) {
    initTetris(); // Spiel neustarten
    return;
  }
  
  // Update Line Clear Effekt
  if (tetrisLineEffect.active) {
    if (tetrisLineEffect.phase == 0) { // Flash Phase
      if (now - tetrisLineEffect.startTime > 100) { // Schnellerer Flash
        tetrisLineEffect.phase = 1;
        clearTetrisLine(tetrisLineEffect.line);
      }
    } else { // Clear Phase
      if (now - tetrisLineEffect.startTime > 200) { // Schnellere Transition
        tetrisLineEffect.active = false;
      }
    }
    return; // Pausiere Spiel während Linien-Clear
  }
  
  // Spawn neuen Block wenn keiner aktiv
  if (!tetrisCurrentBlock.active) {
    if (now - tetrisLastSpawn > 100) { // Schneller spawnen
      spawnNewTetrisBlock();
      tetrisLastSpawn = now;
    }
    return;
  }
  
  // KI-Steuerung: Bewege Block zur Zielposition
  if (tetrisCurrentBlock.aiControlled) {
    // Rotation anpassen
    if (tetrisCurrentBlock.rotation != tetrisCurrentBlock.targetRotation) {
      int newRotation = (tetrisCurrentBlock.rotation + 1) % 4;
      if (canPlaceTetrisBlock(tetrisCurrentBlock.x, tetrisCurrentBlock.y, tetrisCurrentBlock.type, newRotation)) {
        tetrisCurrentBlock.rotation = newRotation;
      }
    }
    
    // Horizontale Bewegung zur Zielposition
    if (tetrisCurrentBlock.x != tetrisCurrentBlock.targetX) {
      int direction = (tetrisCurrentBlock.targetX > tetrisCurrentBlock.x) ? 1 : -1;
      if (canPlaceTetrisBlock(tetrisCurrentBlock.x + direction, tetrisCurrentBlock.y, tetrisCurrentBlock.type, tetrisCurrentBlock.rotation)) {
        tetrisCurrentBlock.x += direction;
      }
    }
  }
  
  // Block fallen lassen
  if (now - tetrisCurrentBlock.lastFall > tetrisCurrentBlock.fallSpeed) {
    if (canPlaceTetrisBlock(tetrisCurrentBlock.x, tetrisCurrentBlock.y + 1, tetrisCurrentBlock.type, tetrisCurrentBlock.rotation)) {
      tetrisCurrentBlock.y++;
    } else {
      placeTetrisBlock();
      checkTetrisLines();
    }
    tetrisCurrentBlock.lastFall = now;
  }
}

void drawTetrisBlock(MatrixPanel_I2S_DMA* display, int x, int y, int type, uint8_t brightness = 255) {
  if (type <= 0 || type > 7) return;
  
  uint8_t r = (TETRIS_COLORS[type-1][0] * brightness) / 255;
  uint8_t g = (TETRIS_COLORS[type-1][1] * brightness) / 255;
  uint8_t b = (TETRIS_COLORS[type-1][2] * brightness) / 255;
  
  int pixelX = TETRIS_FIELD_OFFSET_X + x * TETRIS_BLOCK_SIZE;
  int pixelY = TETRIS_FIELD_OFFSET_Y + y * TETRIS_BLOCK_SIZE;
  
  // Zeichne 4x4 Block mit Rand für 3D-Effekt
  for (int by = 0; by < TETRIS_BLOCK_SIZE; by++) {
    for (int bx = 0; bx < TETRIS_BLOCK_SIZE; bx++) {
      int px = pixelX + bx;
      int py = pixelY + by;
      
      if (px >= 0 && px < 128 && py >= 0 && py < 64) {
        // Rand für 3D-Effekt
        if (bx == 0 || by == 0) {
          display->drawPixel(px, py, display->color565(min(255, r + 60), min(255, g + 60), min(255, b + 60)));
        } else if (bx == TETRIS_BLOCK_SIZE-1 || by == TETRIS_BLOCK_SIZE-1) {
          display->drawPixel(px, py, display->color565(r/2, g/2, b/2));
        } else {
          display->drawPixel(px, py, display->color565(r, g, b));
        }
      }
    }
  }
}

void drawTetris(MatrixPanel_I2S_DMA* display) {
  static uint8_t lastField[TETRIS_FIELD_HEIGHT][TETRIS_FIELD_WIDTH];
  static TetrisFallingBlock lastBlock = {-1, -1, -1, -1, false, 0, 0, false, -1, -1};
  static bool firstDraw = true;
  
  // Bei erstem Aufruf alles zeichnen
  if (firstDraw) {
    display->clearScreen();
    memset(lastField, 0, sizeof(lastField));
    firstDraw = false;
  }
  
  // Lösche alte Block-Position
  if (lastBlock.active) {
    for (int by = 0; by < 4; by++) {
      for (int bx = 0; bx < 4; bx++) {
        if (TETRIS_TETROMINOS[lastBlock.type][lastBlock.rotation][by][bx]) {
          int fx = lastBlock.x + bx;
          int fy = lastBlock.y + by;
          
          if (fx >= 0 && fx < TETRIS_FIELD_WIDTH && fy >= 0 && fy < TETRIS_FIELD_HEIGHT) {
            int pixelX = TETRIS_FIELD_OFFSET_X + fx * TETRIS_BLOCK_SIZE;
            int pixelY = TETRIS_FIELD_OFFSET_Y + fy * TETRIS_BLOCK_SIZE;
            
            // Lösche den Block (schwarze Pixel)
            for (int py = 0; py < TETRIS_BLOCK_SIZE; py++) {
              for (int px = 0; px < TETRIS_BLOCK_SIZE; px++) {
                if (pixelX + px < 128 && pixelY + py < 64) {
                  display->drawPixel(pixelX + px, pixelY + py, 0);
                }
              }
            }
          }
        }
      }
    }
  }
  
  // Zeichne nur geänderte Felder
  for (int y = 0; y < TETRIS_FIELD_HEIGHT; y++) {
    for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
      if (tetrisPlayField[y][x] != lastField[y][x] || 
          (tetrisLineEffect.active && tetrisLineEffect.line == y)) {
        
        // Lösche alte Position
        int pixelX = TETRIS_FIELD_OFFSET_X + x * TETRIS_BLOCK_SIZE;
        int pixelY = TETRIS_FIELD_OFFSET_Y + y * TETRIS_BLOCK_SIZE;
        
        for (int py = 0; py < TETRIS_BLOCK_SIZE; py++) {
          for (int px = 0; px < TETRIS_BLOCK_SIZE; px++) {
            if (pixelX + px < 128 && pixelY + py < 64) {
              display->drawPixel(pixelX + px, pixelY + py, 0);
            }
          }
        }
        
        // Zeichne neuen Block falls vorhanden
        if (tetrisPlayField[y][x] != 0) {
          // Spezialeffekt für Linie die gelöscht wird
          if (tetrisLineEffect.active && tetrisLineEffect.line == y && tetrisLineEffect.phase == 0) {
            uint8_t flash = (millis() % 100 < 50) ? 255 : 50;
            drawTetrisBlock(display, x, y, tetrisPlayField[y][x], flash);
          } else {
            drawTetrisBlock(display, x, y, tetrisPlayField[y][x]);
          }
        }
        
        lastField[y][x] = tetrisPlayField[y][x];
      }
    }
  }
  
  // Zeichne fallenden Block
  if (tetrisCurrentBlock.active) {
    for (int by = 0; by < 4; by++) {
      for (int bx = 0; bx < 4; bx++) {
        if (TETRIS_TETROMINOS[tetrisCurrentBlock.type][tetrisCurrentBlock.rotation][by][bx]) {
          int fx = tetrisCurrentBlock.x + bx;
          int fy = tetrisCurrentBlock.y + by;
          
          if (fx >= 0 && fx < TETRIS_FIELD_WIDTH && fy >= 0 && fy < TETRIS_FIELD_HEIGHT) {
            drawTetrisBlock(display, fx, fy, tetrisCurrentBlock.type + 1);
          }
        }
      }
    }
  }
  
  // Speichere aktuellen Block-Zustand
  lastBlock = tetrisCurrentBlock;
}

void showContent10(MatrixPanel_I2S_DMA* display) {
  static bool initialized = false;
  if (!initialized) {
    initTetris();
    initialized = true;
    randomSeed(millis());
  }
  
  // Update at 120fps for smoother animation
  if (millis() - tetrisLastUpdate < 8) {
    return;
  }
  tetrisLastUpdate = millis();
  
  updateTetris();
  drawTetris(display);
}

// Alternative pattern - Ultra-schnelles Tetris
void showContent10Pattern2(MatrixPanel_I2S_DMA* display) {
  static bool initialized = false;
  if (!initialized) {
    initTetris();
    tetrisLevel = 10; // Sehr hohes Level für maximale Geschwindigkeit
    initialized = true;
    randomSeed(millis());
  }
  
  if (millis() - tetrisLastUpdate < 4) { // Noch schneller
    return;
  }
  tetrisLastUpdate = millis();
  
  updateTetris();
  drawTetris(display);
}

#endif