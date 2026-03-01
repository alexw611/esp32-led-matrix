// content5.h - Bild aus github
// format .xbm

// in psram laden macht nicht soviel sin 1kb


#ifndef CONTENT5_H
#define CONTENT5_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* imageUrl = "https://raw.githubusercontent.com/alexw611/esp32_projekt/main/cotive.xbm";

#define IMG_WIDTH 128
#define IMG_HEIGHT 64
#define IMG_BYTES ((IMG_WIDTH + 7) / 8 * IMG_HEIGHT)

uint8_t imageBuffer[IMG_BYTES];

// Status-Variablen
static bool content5_loaded = false;
static bool content5_error = false;
static bool content5_displayed = false;
static MatrixPanel_I2S_DMA* content5_last_display = nullptr;
static uint32_t content5_last_call_time = 0;
static const uint32_t CONTENT5_MIN_REDRAW_INTERVAL = 1000; // 1 Sekunde

// XBM-Download und Parsing
bool downloadXBMData(const char* url, uint8_t* buffer, size_t bufsize) {
    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP Error: %d\n", httpCode);
        http.end();
        return false;
    }
    String payload = http.getString();
    http.end();

    int startPos = payload.indexOf('{');
    int endPos = payload.lastIndexOf('}');
    if (startPos == -1 || endPos == -1) {
        Serial.println("XBM Format Error: No data array found");
        return false;
    }
    String dataSection = payload.substring(startPos + 1, endPos);

    size_t bytesRead = 0;
    int pos = 0;
    while (pos < dataSection.length() && bytesRead < bufsize) {
        int hexStart = dataSection.indexOf("0x", pos);
        if (hexStart == -1) {
            while (pos < dataSection.length() && bytesRead < bufsize) {
                char c = dataSection.charAt(pos);
                if (isxdigit(c)) {
                    String hexByte = "";
                    while (pos < dataSection.length() && isxdigit(dataSection.charAt(pos))) {
                        hexByte += dataSection.charAt(pos);
                        pos++;
                    }
                    if (hexByte.length() > 0) {
                        buffer[bytesRead++] = (uint8_t)strtol(hexByte.c_str(), NULL, 16);
                    }
                } else {
                    pos++;
                }
            }
            break;
        }
        int hexEnd = hexStart + 2;
        while (hexEnd < dataSection.length() && 
               (isxdigit(dataSection.charAt(hexEnd)) || dataSection.charAt(hexEnd) == 'x')) {
            hexEnd++;
        }
        String hexValue = dataSection.substring(hexStart + 2, hexEnd);
        if (hexValue.length() > 0) {
            buffer[bytesRead++] = (uint8_t)strtol(hexValue.c_str(), NULL, 16);
        }
        pos = hexEnd;
    }
    Serial.printf("XBM loaded: %d bytes (expected: %d)\n", bytesRead, bufsize);
    return (bytesRead > 0);
}

void showContent5(MatrixPanel_I2S_DMA* display) {
    uint32_t current_time = millis();
    bool needs_refresh = false;

    if (!content5_displayed || content5_last_display != display) {
        needs_refresh = true;
    } else if (current_time - content5_last_call_time > CONTENT5_MIN_REDRAW_INTERVAL) {
        needs_refresh = true;
    }

    // Solange nicht geladen, immer wieder probieren!
    if (!content5_loaded) {
        display->clearScreen();
        display->setTextColor(display->color565(255, 255, 0));
        display->setCursor(20, 30);
        display->print("Lade .XBM von github...");
        Serial.println("Trying to load XBM from GitHub...");
        if (downloadXBMData(imageUrl, imageBuffer, IMG_BYTES)) {
            content5_loaded = true;
            content5_error = false;
            Serial.println("XBM loaded successfully");
            needs_refresh = true;
        } else {
            content5_error = true;
            Serial.println("Failed to load XBM, will retry...");
            // needs_refresh bleibt true, damit Fehleranzeige kommt
        }
        content5_displayed = false; // Immer wieder probieren
        content5_last_display = display;
        content5_last_call_time = current_time;
        return; // Beim nächsten Loop wird erneut probiert
    }

    // Wenn Fehler, Fehleranzeige (wird aber beim nächsten Aufruf wieder probiert!)
    if (content5_error && !content5_loaded) {
        display->clearScreen();
        display->setTextColor(display->color565(255, 0, 0));
        display->setCursor(10, 25);
        display->print("Bild-Fehler");
        display->setCursor(5, 35);
        display->print("GitHub nicht");
        display->setCursor(15, 45);
        display->print("erreichbar");
        content5_displayed = true;
        content5_last_display = display;
        content5_last_call_time = current_time;
        // KEIN return, damit beim nächsten Aufruf neu probiert wird!
        return;
    }

    // Bild zeichnen, wenn geladen
    if (content5_loaded && needs_refresh) {
        display->clearScreen();
        int x0 = (display->width() - IMG_WIDTH) / 2;
        int y0 = (display->height() - IMG_HEIGHT) / 2;
        for (int y = 0; y < IMG_HEIGHT; y++) {
            for (int x = 0; x < IMG_WIDTH; x++) {
                int byteIndex = (y * ((IMG_WIDTH + 7) / 8)) + (x / 8);
                int bitIndex = x % 8;
                if (byteIndex < IMG_BYTES) {
                    bool pixelOn = (imageBuffer[byteIndex] >> bitIndex) & 0x01;
                    uint16_t color = pixelOn ?
                        display->color565(255, 255, 255) :
                        display->color565(0, 0, 0);
                    display->drawPixel(x0 + x, y0 + y, color);
                }
            }
        }
        content5_displayed = true;
        content5_last_display = display;
        Serial.println("XBM displayed successfully");
    }

    content5_last_call_time = current_time;
}

// Reset-Funktion für Debug-Zwecke
void resetContent5() {
    content5_loaded = false;
    content5_error = false;
    content5_displayed = false;
    content5_last_display = nullptr;
    content5_last_call_time = 0;
}

#endif // CONTENT5_H

