// content8.h
// GIF Player für ESP32 Matrix Display - lädt GIF von GitHub in den PSRAM (ESP32-Wrover)


// aktuelle problem bis mx 93328 bytes darf das gif gross sein weil free heap before download 
/*
PSRAM detected! Free PSRAM: 4192123 bytes
Free heap before download: 93328 bytes
Largest free block: 49140 bytes
Downloading GIF from: https://raw.githubusercontent.com/alexw611/esp32_projekt/main/gif4.gif
HTTP Response Code: 200
Content Length: 61916 bytes
Allocating 61916 bytes in PSRAM...
Buffer allocated in PSRAM. Free PSRAM now: 4116247 bytes
Download complete: 61916/61916 bytes
Valid GIF downloaded!
Attempting to open GIF...
GIF opened successfully (Method 1)

*/

// max gif size = 80kb

#ifndef CONTENT8_H
#define CONTENT8_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <AnimatedGIF.h>
#include <esp_heap_caps.h> // Für PSRAM-Allocation

// Globale Variablen für GIF-Wiedergabe
static AnimatedGIF gif;
static unsigned long lastFrameTime = 0;
static bool gifInitialized = false;
static bool gifPlaying = false;
static uint8_t* gifBuffer = nullptr;
static int gifBufferSize = 0;
static int gifBufferPos = 0;

// GitHub URL - ÄNDERE DIESE URL zu deinem GitHub Repository
const char* GIF_URL = "https://raw.githubusercontent.com/alexw611/esp32_projekt/main/gif4.gif";

// Callback-Funktion für GIF-Pixel-Drawing
void GIFDraw(GIFDRAW *pDraw) {
    MatrixPanel_I2S_DMA* display = (MatrixPanel_I2S_DMA*)pDraw->pUser;
    uint8_t *s;
    uint16_t *usPalette;
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth > 128) iWidth = 128; // Matrix ist 128 pixel breit

    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // aktuelle Zeile

    if (y >= 64) return; // Matrix ist 64 pixel hoch

    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) {
        for (x = 0; x < iWidth; x++) {
            if (s[x] == pDraw->ucTransparent) continue;
            display->drawPixel(pDraw->iX + x, y, usPalette[s[x]]);
        }
    } else {
        for (x = 0; x < iWidth; x++) {
            if (s[x] != pDraw->ucTransparent)
                display->drawPixel(pDraw->iX + x, y, usPalette[s[x]]);
        }
    }
}

// Memory-basierte GIF-Callbacks
void * GIFOpenFile(const char *fname, int32_t *pSize) {
    *pSize = gifBufferSize;
    gifBufferPos = 0;
    return gifBuffer;
}

void GIFCloseFile(void *pHandle) {
    // Nichts zu tun für Memory-Buffer
}

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
    int32_t iBytesRead = 0;
    if (gifBufferPos < gifBufferSize) {
        iBytesRead = gifBufferSize - gifBufferPos;
        if (iBytesRead > iLen) iBytesRead = iLen;
        memcpy(pBuf, &gifBuffer[gifBufferPos], iBytesRead);
        gifBufferPos += iBytesRead;
    }
    return iBytesRead;
}

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
    if (iPosition < gifBufferSize) {
        gifBufferPos = iPosition;
    }
    return gifBufferPos;
}

bool downloadGIF() {
    // PSRAM-Status prüfen
    if (psramFound()) {
        Serial.printf("PSRAM detected! Free PSRAM: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    } else {
        Serial.println("No PSRAM found!");
        return false;
    }

    Serial.printf("Free heap before download: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected - cannot download GIF");
        return false;
    }

    // HTTPS-Client für GitHub!
    WiFiClientSecure client;
    client.setInsecure(); // Für Testzwecke, für Produktivcode Zertifikat prüfen!

    HTTPClient http;
    if (!http.begin(client, GIF_URL)) {
        Serial.println("HTTPClient begin() failed!");
        return false;
    }
    http.setTimeout(10000);

    Serial.print("Downloading GIF from: ");
    Serial.println(GIF_URL);

    int httpCode = http.GET();
    Serial.printf("HTTP Response Code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        Serial.printf("Content Length: %d bytes\n", contentLength);

        // Maximale GIF-Größe für ESP32: 2MB (PSRAM), ggf. anpassen
        if (contentLength > 0 && contentLength < 2 * 1024 * 1024) {
            if (gifBuffer != nullptr) {
                free(gifBuffer);
                gifBuffer = nullptr;
            }

            Serial.printf("Allocating %d bytes in PSRAM...\n", contentLength);
            gifBuffer = (uint8_t*)heap_caps_malloc(contentLength, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            if (gifBuffer == nullptr) {
                Serial.printf("PSRAM MALLOC FAILED! Tried to allocate %d bytes\n", contentLength);
                Serial.printf("Free PSRAM: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
                http.end();
                return false;
            }

            Serial.printf("Buffer allocated in PSRAM. Free PSRAM now: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

            WiFiClient *stream = http.getStreamPtr();
            int bytesRead = 0;
            unsigned long startTime = millis();

            while (http.connected() && (bytesRead < contentLength)) {
                size_t size = stream->available();
                if (size) {
                    int readBytes = stream->readBytes(&gifBuffer[bytesRead],
                                                      min(size, (size_t)(contentLength - bytesRead)));
                    bytesRead += readBytes;
                }
                if (millis() - startTime > 15000) {
                    Serial.println("Download timeout");
                    break;
                }
                delay(1);
            }

            gifBufferSize = bytesRead;
            Serial.printf("Download complete: %d/%d bytes\n", gifBufferSize, contentLength);

            if (gifBufferSize == contentLength && gifBufferSize > 100) {
                if (gifBuffer[0] == 'G' && gifBuffer[1] == 'I' && gifBuffer[2] == 'F') {
                    Serial.println("Valid GIF downloaded!");
                    http.end();
                    return true;
                } else {
                    Serial.printf("Invalid GIF header: %c%c%c\n", gifBuffer[0], gifBuffer[1], gifBuffer[2]);
                }
            }
        } else if (contentLength >= 2 * 1024 * 1024) {
            Serial.printf("ERROR: GIF too large! %d bytes (max 2MB for PSRAM)\n", contentLength);
        } else {
            Serial.printf("Invalid content length: %d\n", contentLength);
        }
    } else {
        Serial.printf("HTTP GET failed, error: %d\n", httpCode);
    }

    http.end();
    return false;
}

bool initializeGIF(MatrixPanel_I2S_DMA* display) {
    if (!downloadGIF()) {
        Serial.println("Download failed");
        return false;
    }

    gif.begin(LITTLE_ENDIAN_PIXELS);

    Serial.println("Attempting to open GIF...");

    if (gif.open(gifBuffer, gifBufferSize, GIFDraw)) {
        Serial.println("GIF opened successfully (Method 1)");
        return true;
    }

    Serial.println("Method 1 failed, trying alternative...");

    gif.close();
    gif.begin(LITTLE_ENDIAN_PIXELS);

    gifBufferPos = 0;
    if (gif.open("memory", GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
        Serial.println("GIF opened successfully (Method 2)");
        return true;
    }

    Serial.println("All methods failed to open GIF");
    Serial.printf("Buffer size: %d, First bytes: %02X %02X %02X %02X\n",
                  gifBufferSize, gifBuffer[0], gifBuffer[1], gifBuffer[2], gifBuffer[3]);

    return false;
}

void showContent8(MatrixPanel_I2S_DMA* display) {
    static unsigned long lastCall = 0;
    static bool forceRedraw = false;

    unsigned long now = millis();
    if (now - lastCall > 2000) {
        forceRedraw = true;
    }
    lastCall = now;

    if (forceRedraw || !gifInitialized) {
        display->clearScreen();

        display->setTextColor(display->color565(0, 255, 255));
        display->setTextSize(1);
        display->setCursor(20, 20);
        display->print("Loading GIF...");
        display->setCursor(25, 35);
        display->print("from GitHub");

        if (!initializeGIF(display)) {
            display->clearScreen();
            display->setTextColor(display->color565(255, 0, 0));
            display->setTextSize(1);
            display->setCursor(10, 15);
            display->print("GIF Error");
            display->setCursor(5, 25);
            display->print("Check WiFi &");
            display->setCursor(10, 35);
            display->print("GitHub URL");
            return;
        }

        display->clearScreen();
        gifInitialized = true;
        gifPlaying = true;
        forceRedraw = false;
        lastFrameTime = now;
    }

    if (!gifInitialized || !gifPlaying) {
        return;
    }

    if (gif.playFrame(true, NULL, display)) {
        if (now - lastFrameTime >= 100) {
            lastFrameTime = now;
        }
    } else {
        gif.reset();
        gif.playFrame(true, NULL, display);
        lastFrameTime = now;
    }
}

void cleanupContent8() {
    if (gifBuffer != nullptr) {
        free(gifBuffer);
        gifBuffer = nullptr;
        gifBufferSize = 0;
    }
    gifInitialized = false;
    gifPlaying = false;
    gif.close();
}

#endif
