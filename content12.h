//content12.h
// epilepsi generator flacker licht
// working

#ifndef CONTENT12_H
#define CONTENT12_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Farbpaletten für schnelle Wechsel
static uint16_t flashColors[] = {
    0xF800, // Rot
    0x07E0, // Grün  
    0x001F, // Blau
    0xFFE0, // Gelb
    0xF81F, // Magenta
    0x07FF, // Cyan
    0xFFFF, // Weiß
    0x0000, // Schwarz
    0xFD20, // Orange
    0x8010, // Lila
    0x07E0, // Hellgrün
    0x001F  // Hellblau
};

static uint16_t strobeColors[] = {
    0xFFFF, // Weiß
    0x0000, // Schwarz
    0xF800, // Rot
    0x0000, // Schwarz
    0x07E0, // Grün
    0x0000, // Schwarz
    0x001F, // Blau
    0x0000  // Schwarz
};

// Noise-Muster für Chaos-Effekt
static uint8_t noisePattern[128][64];
static bool noiseInitialized = false;

// Schnelle Zufallszahl
uint16_t fastRandom() {
    static uint32_t seed = 12345;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

// Initiale Noise-Generierung
void initNoise() {
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 64; y++) {
            noisePattern[x][y] = fastRandom() % 256;
        }
    }
    noiseInitialized = true;
}

void testContent12() {
    Serial.println("Content12 Test: Schnelle Farbwechsel Effekte");
}

void showContent12(MatrixPanel_I2S_DMA* display) {
    static unsigned long lastUpdate = 0;
    static int effectMode = 0;
    static int frameCount = 0;
    static unsigned long modeChangeTime = 0;
    
    // Sehr schnelle Updates - 60 FPS (16ms)
    if (millis() - lastUpdate < 16) return;
    lastUpdate = millis();
    
    frameCount++;
    
    // Effekt-Modi alle 2 Sekunden wechseln
    if (millis() - modeChangeTime > 2000) {
        effectMode = (effectMode + 1) % 8;
        modeChangeTime = millis();
        frameCount = 0;
    }
    
    // Noise-Pattern initialisieren
    if (!noiseInitialized) {
        initNoise();
    }
    
    switch (effectMode) {
        case 0: // Vollbild Farbwechsel
            {
                uint16_t color = flashColors[frameCount % 12];
                display->fillScreen(color);
            }
            break;
            
        case 1: // Strobe-Effekt
            {
                uint16_t color = strobeColors[frameCount % 8];
                display->fillScreen(color);
            }
            break;
            
        case 2: // Chaos-Noise
            {
                for (int x = 0; x < 128; x++) {
                    for (int y = 0; y < 64; y++) {
                        uint8_t noise = noisePattern[x][y] + frameCount;
                        uint16_t color = flashColors[noise % 12];
                        display->drawPixel(x, y, color);
                    }
                }
                
                // Noise-Pattern aktualisieren
                if (frameCount % 3 == 0) {
                    for (int i = 0; i < 500; i++) { // Nur Teil des Patterns updaten
                        int x = fastRandom() % 128;
                        int y = fastRandom() % 64;
                        noisePattern[x][y] = fastRandom() % 256;
                    }
                }
            }
            break;
            
        case 3: // Horizontal Split
            {
                uint16_t topColor = flashColors[frameCount % 12];
                uint16_t bottomColor = flashColors[(frameCount + 6) % 12];
                
                for (int y = 0; y < 32; y++) {
                    for (int x = 0; x < 128; x++) {
                        display->drawPixel(x, y, topColor);
                    }
                }
                for (int y = 32; y < 64; y++) {
                    for (int x = 0; x < 128; x++) {
                        display->drawPixel(x, y, bottomColor);
                    }
                }
            }
            break;
            
        case 4: // Vertikal Split mit Bewegung
            {
                int splitPos = 32 + 20 * sin(frameCount * 0.3);
                uint16_t leftColor = flashColors[frameCount % 12];
                uint16_t rightColor = flashColors[(frameCount + 6) % 12];
                
                for (int x = 0; x < splitPos; x++) {
                    for (int y = 0; y < 64; y++) {
                        display->drawPixel(x, y, leftColor);
                    }
                }
                for (int x = splitPos; x < 128; x++) {
                    for (int y = 0; y < 64; y++) {
                        display->drawPixel(x, y, rightColor);
                    }
                }
            }
            break;
            
        case 5: // Schachbrett-Muster
            {
                uint16_t color1 = flashColors[frameCount % 12];
                uint16_t color2 = flashColors[(frameCount + 6) % 12];
                int blockSize = 8;
                
                for (int x = 0; x < 128; x += blockSize) {
                    for (int y = 0; y < 64; y += blockSize) {
                        uint16_t color = ((x/blockSize + y/blockSize + frameCount) % 2 == 0) ? color1 : color2;
                        
                        for (int dx = 0; dx < blockSize && x + dx < 128; dx++) {
                            for (int dy = 0; dy < blockSize && y + dy < 64; dy++) {
                                display->drawPixel(x + dx, y + dy, color);
                            }
                        }
                    }
                }
            }
            break;
            
        case 6: // Radialer Farbwechsel
            {
                int centerX = 64;
                int centerY = 32;
                
                for (int x = 0; x < 128; x++) {
                    for (int y = 0; y < 64; y++) {
                        int dx = x - centerX;
                        int dy = y - centerY;
                        int dist = sqrt(dx*dx + dy*dy);
                        
                        int colorIndex = (dist + frameCount * 2) % 12;
                        display->drawPixel(x, y, flashColors[colorIndex]);
                    }
                }
            }
            break;
            
        case 7: // Zufällige Pixel-Explosion
            {
                // Schwarzer Hintergrund
                display->fillScreen(0x0000);
                
                // Zufällige helle Pixel
                for (int i = 0; i < 800; i++) {
                    int x = fastRandom() % 128;
                    int y = fastRandom() % 64;
                    uint16_t color = flashColors[fastRandom() % 12];
                    display->drawPixel(x, y, color);
                }
                
                // Einige größere Blitze
                for (int i = 0; i < 20; i++) {
                    int x = fastRandom() % 126;
                    int y = fastRandom() % 62;
                    uint16_t color = 0xFFFF; // Weiß
                    
                    // 3x3 Block
                    for (int dx = 0; dx < 3; dx++) {
                        for (int dy = 0; dy < 3; dy++) {
                            display->drawPixel(x + dx, y + dy, color);
                        }
                    }
                }
            }
            break;
    }
    
    // Gelegentliche Vollbild-Blitze für Extra-Dramatik
    if (frameCount % 47 == 0) { // Alle ~0.8 Sekunden
        display->fillScreen(0xFFFF); // Weißer Blitz
        delay(33); // Kurzer Blitz
    }
}

#endif