// content3.h - Regenbogen Wasser Wellen Animation



#ifndef CONTENT3_H
#define CONTENT3_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Reduzierte Puffer für bessere Performance
static uint16_t frameBuffer[128][64];
static bool bufferInitialized = false;

// Lookup-Tabellen für Performance
static float sinLUT[360];
static float cosLUT[360];
static bool lutInitialized = false;

// Optimierte HSV zu RGB Konversion
uint16_t fastHSVtoRGB565(float h, float s, float v) {
    h = fmod(h, 360.0f);
    if (h < 0) h += 360.0f;
    
    int hi = (int)(h / 60.0f) % 6;
    float f = (h / 60.0f) - hi;
    
    uint8_t vInt = (uint8_t)(v * 255);
    uint8_t p = (uint8_t)(v * (1.0f - s) * 255);
    uint8_t q = (uint8_t)(v * (1.0f - s * f) * 255);
    uint8_t t = (uint8_t)(v * (1.0f - s * (1.0f - f)) * 255);
    
    uint8_t r, g, b;
    switch(hi) {
        case 0: r = vInt; g = t; b = p; break;
        case 1: r = q; g = vInt; b = p; break;
        case 2: r = p; g = vInt; b = t; break;
        case 3: r = p; g = q; b = vInt; break;
        case 4: r = t; g = p; b = vInt; break;
        default: r = vInt; g = p; b = q; break;
    }
    
    // Direkte 565 Konversion ohne color565() Aufruf
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Fast sine/cosine mit Lookup-Tabelle
float fastSin(float angle) {
    int index = (int)(fmod(angle * 57.2958f, 360.0f)); // 180/PI = 57.2958
    if (index < 0) index += 360;
    return sinLUT[index];
}

float fastCos(float angle) {
    int index = (int)(fmod(angle * 57.2958f, 360.0f));
    if (index < 0) index += 360;
    return cosLUT[index];
}

void testContent3() {
    Serial.println("Content3 Test: Optimized Rainbow Pattern");
}

void showContent3(MatrixPanel_I2S_DMA* display) {
    static unsigned long lastUpdate = 0;
    static float globalTime = 0;
    static int frameCount = 0;
    
    // Update alle 33ms für 30 FPS (war 25ms)
    if (millis() - lastUpdate < 33) return;
    lastUpdate = millis();
    
    // Lookup-Tables beim ersten Aufruf initialisieren
    if (!lutInitialized) {
        for (int i = 0; i < 360; i++) {
            sinLUT[i] = sin(i * PI / 180.0f);
            cosLUT[i] = cos(i * PI / 180.0f);
        }
        lutInitialized = true;
    }
    
    // Buffer initialisieren
    if (!bufferInitialized) {
        memset(frameBuffer, 0, sizeof(frameBuffer));
        bufferInitialized = true;
    }
    
    // Langsamere, flüssigere Zeit-Parameter
    globalTime += 0.08f; // Viel langsamer für flüssigere Animation
    frameCount++;
    
    // Vorberechnete Werte für bessere Performance
    float time1 = globalTime * 1.5f;
    float time2 = globalTime * 2.0f;
    float time3 = globalTime * 1.8f;
    
    // Ripple-Zentren vorberechnen (weniger frequent updates)
    static float rippleX1 = 64, rippleY1 = 32;
    static float rippleX2 = 64, rippleY2 = 32;
    if (frameCount % 3 == 0) { // Nur alle 3 Frames updaten
        rippleX1 = 64 + 30 * fastSin(time1 * 0.6f);
        rippleY1 = 32 + 20 * fastCos(time1 * 0.5f);
        rippleX2 = 64 - 25 * fastCos(time2 * 0.7f);
        rippleY2 = 32 + 18 * fastSin(time2 * 0.6f);
    }
    
    // Optimierte Pixel-Berechnung
    for (int y = 0; y < 64; y++) {
        float yWave = fastCos(y * 0.12f + time2) * 0.3f;
        float yFactor = y * 0.8f;
        
        for (int x = 0; x < 128; x++) {
            // Vereinfachte Wellen-Berechnung
            float xWave = fastSin(x * 0.15f + time1) * 0.4f;
            float diagWave = fastSin((x + y) * 0.08f + time3) * 0.25f;
            
            // Schnellere Distanz-Berechnung (ohne sqrt)
            float dx1 = x - rippleX1;
            float dy1 = y - rippleY1;
            float dist1 = dx1 * dx1 + dy1 * dy1; // Quadrat-Distanz reicht
            float ripple1 = fastSin(dist1 * 0.02f + time1 * 2.0f) * 0.3f;
            
            float dx2 = x - rippleX2;
            float dy2 = y - rippleY2;
            float dist2 = dx2 * dx2 + dy2 * dy2;
            float ripple2 = fastCos(dist2 * 0.018f + time2 * 1.8f) * 0.25f;
            
            // Kombinierte Welle
            float totalWave = xWave + yWave + diagWave + ripple1 + ripple2;
            
            // Vereinfachte Farbberechnung
            float hue = time1 * 30 + x * 1.5f + yFactor + totalWave * 20;
            hue = fmod(hue, 360.0f);
            
            // Konstante Sättigung und variable Helligkeit
            float brightness = 0.7f + 0.25f * fastSin(totalWave * 1.2f + time1);
            brightness = constrain(brightness, 0.5f, 0.95f);
            
            // Direkte RGB Berechnung
            frameBuffer[x][y] = fastHSVtoRGB565(hue, 0.9f, brightness);
        }
    }
    
    // Weniger häufige Effekte
    if (frameCount % 20 == 0) { // Alle 20 Frames statt 8
        for (int i = 0; i < 2; i++) { // Nur 2 statt 4 Tropfen
            int dropX = random(5, 123);
            int dropY = random(5, 59);
            
            // Kleinere Tropfen für bessere Performance
            frameBuffer[dropX][dropY] = 0xFFFF; // Weiß
            if (dropX > 0) frameBuffer[dropX-1][dropY] = 0xCE79; // Hellblau
            if (dropX < 127) frameBuffer[dropX+1][dropY] = 0xCE79;
            if (dropY > 0) frameBuffer[dropX][dropY-1] = 0xCE79;
            if (dropY < 63) frameBuffer[dropX][dropY+1] = 0xCE79;
        }
    }
    
    // Seltenere Sonnenreflexe
    if (frameCount % 30 == 0) { // Alle 30 Frames statt 12
        for (int i = 0; i < 3; i++) { // Nur 3 statt 6
            int refX = random(128);
            int refY = random(64);
            frameBuffer[refX][refY] = 0xFFE0; // Gelb
        }
    }
    
    // Optimierter Display-Update: Zeilen-weise statt Pixel für Pixel
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 128; x++) {
            display->drawPixel(x, y, frameBuffer[x][y]);
        }
    }
}

#endif