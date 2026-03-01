// content9
// Feuerwerk Animation - Optimiert für 128x64 Display

#ifndef CONTENT9_H
#define CONTENT9_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

struct Particle {
  float x, y;
  float vx, vy;
  uint8_t r, g, b;
  int life;
  int maxLife;
  bool active;
};

struct Firework {
  float x, y;
  float vx, vy;
  uint8_t r, g, b;
  bool exploded;
  bool active;
  int type; // 0=normal, 1=burst, 2=sparkle, 3=ring
  float targetY; // Explosion height
};

const int MAX_FIREWORKS = 8;
const int MAX_PARTICLES = 200;

static Firework fireworks[MAX_FIREWORKS];
static Particle particles[MAX_PARTICLES];
static unsigned long lastFireworkSpawn = 0;
static unsigned long lastUpdate = 0;

// Standalone test function
void testContent9() {
  Serial.println("Content9 Test: Spektakuläre Feuerwerk-Show wird gestartet!");
}

void initFireworks() {
  for (int i = 0; i < MAX_FIREWORKS; i++) {
    fireworks[i].active = false;
  }
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
  }
}

void spawnFirework() {
  for (int i = 0; i < MAX_FIREWORKS; i++) {
    if (!fireworks[i].active) {
      fireworks[i].x = random(20, 108);
      fireworks[i].y = 63;
      fireworks[i].vx = (random(-20, 21)) * 0.1f;
      // Reduzierte Geschwindigkeit damit Feuerwerke nicht zu hoch fliegen
      fireworks[i].vy = -(random(40, 80)) * 0.1f;
      fireworks[i].r = random(100, 255);
      fireworks[i].g = random(100, 255);
      fireworks[i].b = random(100, 255);
      fireworks[i].exploded = false;
      fireworks[i].active = true;
      fireworks[i].type = random(0, 4);
      // Bestimme Explosionshöhe zwischen y=10 und y=50 (sichtbarer Bereich)
      fireworks[i].targetY = random(10, 50);
      break;
    }
  }
}

void explodeFirework(int fw_idx) {
  Firework& fw = fireworks[fw_idx];
  int particleCount = 0;
  
  switch (fw.type) {
    case 0: // Normal explosion
      particleCount = random(40, 70);
      break;
    case 1: // Big burst
      particleCount = random(70, 100);
      break;
    case 2: // Sparkle effect
      particleCount = random(50, 80);
      break;
    case 3: // Ring effect
      particleCount = random(60, 90);
      break;
  }
  
  int created = 0;
  for (int i = 0; i < MAX_PARTICLES && created < particleCount; i++) {
    if (!particles[i].active) {
      particles[i].x = fw.x;
      particles[i].y = fw.y;
      
      if (fw.type == 3) { // Ring effect
        float angle = (created * 2.0f * PI) / particleCount;
        // Reduzierte Geschwindigkeit für bessere Sichtbarkeit
        float speed = random(20, 50) * 0.1f;
        particles[i].vx = cos(angle) * speed;
        particles[i].vy = sin(angle) * speed;
      } else {
        // Reduzierte Partikelgeschwindigkeit
        particles[i].vx = (random(-100, 101)) * 0.1f;
        particles[i].vy = (random(-100, 101)) * 0.1f;
      }
      
      if (fw.type == 2) { // Sparkle effect - white/gold
        particles[i].r = random(200, 255);
        particles[i].g = random(180, 255);
        particles[i].b = random(50, 150);
      } else {
        // Color variations around the firework color
        int colorVar = (int)random(-50, 51);
        particles[i].r = max(0, min(255, (int)fw.r + colorVar));
        particles[i].g = max(0, min(255, (int)fw.g + colorVar));
        particles[i].b = max(0, min(255, (int)fw.b + colorVar));
      }
      
      particles[i].maxLife = random(60, 150); // Längere Lebensdauer
      particles[i].life = particles[i].maxLife;
      particles[i].active = true;
      created++;
    }
  }
  
  fw.exploded = true;
}

void updateFireworks() {
  // Update fireworks
  for (int i = 0; i < MAX_FIREWORKS; i++) {
    if (!fireworks[i].active) continue;
    
    if (!fireworks[i].exploded) {
      fireworks[i].x += fireworks[i].vx;
      fireworks[i].y += fireworks[i].vy;
      fireworks[i].vy += 0.15f; // Reduzierte Schwerkraft
      
      // Explodiere bei Ziel-Höhe oder wenn Geschwindigkeit positiv wird
      if (fireworks[i].y <= fireworks[i].targetY || fireworks[i].vy > -0.5f) {
        // Stelle sicher, dass Explosion im sichtbaren Bereich stattfindet
        if (fireworks[i].y < 5) fireworks[i].y = 5;
        if (fireworks[i].y > 59) fireworks[i].y = 59;
        explodeFirework(i);
      }
      
      // Entferne wenn außerhalb des Bildschirms
      if (fireworks[i].y > 64 || fireworks[i].y < -5) {
        fireworks[i].active = false;
      }
    } else {
      fireworks[i].active = false; // Remove exploded firework
    }
  }
  
  // Update particles
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) continue;
    
    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;
    particles[i].vx *= 0.99f; // Weniger Luftwiderstand
    particles[i].vy *= 0.99f;
    particles[i].vy += 0.08f; // Reduzierte Schwerkraft für längere Sichtbarkeit
    
    particles[i].life--;
    if (particles[i].life <= 0 || particles[i].y > 68 || particles[i].x < -5 || particles[i].x > 133) {
      particles[i].active = false;
    }
  }
}

void drawFireworks(MatrixPanel_I2S_DMA* display) {
  display->clearScreen();
  
  // Draw stars background
  static int starPhase = 0;
  for (int i = 0; i < 40; i++) {
    int x = (i * 37) % 128;
    int y = (i * 23) % 64;
    if ((starPhase + i) % 120 < 8) {
      uint8_t brightness = 80 + (i % 3) * 40;
      display->drawPixel(x, y, display->color565(brightness, brightness, brightness));
    }
  }
  starPhase++;
  
  // Draw fireworks (rockets) - Heller und größer!
  for (int i = 0; i < MAX_FIREWORKS; i++) {
    if (fireworks[i].active && !fireworks[i].exploded) {
      int x = (int)fireworks[i].x;
      int y = (int)fireworks[i].y;
      if (x >= 0 && x < 128 && y >= 0 && y < 64) {
        // Helle Rakete (weißer Kern + Farbe)
        display->drawPixel(x, y, display->color565(255, 255, 255)); // Weißer Kern
        
        // Farbiger Rand um die Rakete
        if (x > 0) display->drawPixel(x-1, y, display->color565(fireworks[i].r, fireworks[i].g, fireworks[i].b));
        if (x < 127) display->drawPixel(x+1, y, display->color565(fireworks[i].r, fireworks[i].g, fireworks[i].b));
        if (y > 0) display->drawPixel(x, y-1, display->color565(fireworks[i].r, fireworks[i].g, fireworks[i].b));
        
        // Langer, heller Trail
        for (int t = 1; t <= 8; t++) {
          if (y + t < 64) {
            uint8_t brightness = 255 - (t * 25); // Allmähliches Abdunkeln
            uint8_t trailR = (fireworks[i].r * brightness) / 255;
            uint8_t trailG = (fireworks[i].g * brightness) / 255;
            uint8_t trailB = (fireworks[i].b * brightness) / 255;
            display->drawPixel(x, y + t, display->color565(trailR, trailG, trailB));
            
            // Breiterer Trail für bessere Sichtbarkeit
            if (t <= 4) {
              if (x > 0) display->drawPixel(x-1, y + t, display->color565(trailR/2, trailG/2, trailB/2));
              if (x < 127) display->drawPixel(x+1, y + t, display->color565(trailR/2, trailG/2, trailB/2));
            }
          }
        }
        
        // Funken-Effekt während des Flugs
        if (random(0, 3) == 0) {
          int sparkX = x + random(-2, 3);
          int sparkY = y + random(1, 4);
          if (sparkX >= 0 && sparkX < 128 && sparkY >= 0 && sparkY < 64) {
            display->drawPixel(sparkX, sparkY, display->color565(255, 200, 100)); // Goldene Funken
          }
        }
      }
    }
  }
  
  // Draw particles
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) continue;
    
    int x = (int)particles[i].x;
    int y = (int)particles[i].y;
    
    if (x >= 0 && x < 128 && y >= 0 && y < 64) {
      // Fade particles based on life
      float fade = (float)particles[i].life / particles[i].maxLife;
      uint8_t r = particles[i].r * fade;
      uint8_t g = particles[i].g * fade;
      uint8_t b = particles[i].b * fade;
      
      display->drawPixel(x, y, display->color565(r, g, b));
      
      // Add extra brightness for fresh particles
      if (particles[i].life > particles[i].maxLife * 0.8f) {
        display->drawPixel(x, y, display->color565(min(255, r + 50), min(255, g + 50), min(255, b + 50)));
        
        // Draw bigger particles occasionally for more impact
        if (random(0, 3) == 0) {
          if (x > 0) display->drawPixel(x-1, y, display->color565(r/3, g/3, b/3));
          if (x < 127) display->drawPixel(x+1, y, display->color565(r/3, g/3, b/3));
          if (y > 0) display->drawPixel(x, y-1, display->color565(r/3, g/3, b/3));
          if (y < 63) display->drawPixel(x, y+1, display->color565(r/3, g/3, b/3));
        }
      }
    }
  }
}

void showContent9(MatrixPanel_I2S_DMA* display) {
  static bool initialized = false;
  if (!initialized) {
    initFireworks();
    initialized = true;
    randomSeed(millis());
  }
  
  // Update at 60fps
  if (millis() - lastUpdate < 16) {
    return;
  }
  lastUpdate = millis();
  
  // Spawn new fireworks
  if (millis() - lastFireworkSpawn > random(400, 1200)) {
    // Spawn 1-2 fireworks at once
    int spawnCount = random(1, 3);
    for (int i = 0; i < spawnCount; i++) {
      spawnFirework();
    }
    lastFireworkSpawn = millis();
  }
  
  updateFireworks();
  drawFireworks(display);
}

// Alternative pattern - Finale show (many fireworks at once)
void showContent9Pattern2(MatrixPanel_I2S_DMA* display) {
  static bool initialized = false;
  static unsigned long finaleStart = 0;
  
  if (!initialized) {
    initFireworks();
    initialized = true;
    finaleStart = millis();
    randomSeed(millis());
  }
  
  if (millis() - lastUpdate < 16) {
    return;
  }
  lastUpdate = millis();
  
  // Spawn fireworks much more frequently for finale
  if (millis() - lastFireworkSpawn > random(100, 300)) {
    // Spawn multiple fireworks for epic finale
    int spawnCount = random(3, 6);
    for (int i = 0; i < spawnCount; i++) {
      spawnFirework();
    }
    lastFireworkSpawn = millis();
  }
  
  // Reset finale every 12 seconds
  if (millis() - finaleStart > 12000) {
    finaleStart = millis();
    initFireworks();
  }
  
  updateFireworks();
  drawFireworks(display);
}

#endif