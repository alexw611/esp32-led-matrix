// content1.h
// digitale Uhr mit NTP Server


#ifndef CONTENT1_H
#define CONTENT1_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <time.h>

// Hilfsfunktion: Einzelnes Zeichen löschen (mit Hintergrundfarbe)
void clearChar(MatrixPanel_I2S_DMA* display, int x, int y, int size) {
    int w = 6 * size; // Zeichenbreite
    int h = 8 * size; // Zeichenhöhe (Font ist 6x8)
    display->fillRect(x, y, w, h, display->color565(0, 0, 0)); // Schwarz
}

void showContent1(MatrixPanel_I2S_DMA* display) {
    static unsigned long lastUpdate = 0;
    static unsigned long lastCall = 0;
    static bool timeInitialized = false;
    static char lastTimeStr[9] = "        "; // "HH:MM:SS"
    static char lastDateStr[20] = "                  ";

    // Automatische Initialisierung nach längerer Pause (Content-Wechsel)
    unsigned long now = millis();
    bool forceRedraw = false;
    if (now - lastCall > 2000) { // 2 Sekunden ohne Aufruf = vermutlich Content-Wechsel
        forceRedraw = true;
    }
    lastCall = now;

    // Zeit initialisieren
    if (!timeInitialized) {
        configTime(3600, 3600, "pool.ntp.org");
        timeInitialized = true;
    }

    // Nur jede Sekunde aktualisieren
    if (!forceRedraw && millis() - lastUpdate < 1000) return;
    lastUpdate = millis();

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        display->setTextColor(display->color565(255, 255, 255));
        display->setTextSize(1);
        String noTimeText = "Keine Zeit";
        int textWidth = noTimeText.length() * 6;
        int x = (128 - textWidth) / 2;
        int y = (64 - 7) / 2;
        display->setCursor(x, y);
        display->print(noTimeText);
        return;
    }

    // Datum vorbereiten
    char dateStr[20];
    sprintf(dateStr, "%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);

    // Uhrzeit vorbereiten
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    display->setTextSize(1);
    int dateWidth = strlen(dateStr) * 6;
    int dateX = (128 - dateWidth) / 2;
    int dateY = 6;

    display->setTextSize(2);
    int charWidth = 6 * 2;
    int timeWidth = strlen(timeStr) * charWidth;
    int timeX = (128 - timeWidth) / 2;
    int timeY = dateY + 10 + 6;

    // Vollständiger Redraw bei Content-Wechsel
    if (forceRedraw) {
        display->clearScreen();
        // Datum zeichnen
        display->setTextSize(1);
        display->setTextColor(display->color565(255, 255, 255));
        display->setCursor(dateX, dateY);
        display->print(dateStr);
        strcpy(lastDateStr, dateStr);

        // Uhrzeit zeichnen
        display->setTextSize(2);
        display->setTextColor(display->color565(255, 255, 255));
        display->setCursor(timeX, timeY);
        display->print(timeStr);
        strcpy(lastTimeStr, timeStr);

        return;
    }

    // Datum ggf. aktualisieren
    display->setTextSize(1);
    if (strcmp(dateStr, lastDateStr) != 0) {
        display->fillRect(0, dateY, 128, 8, display->color565(0, 0, 0));
        display->setTextColor(display->color565(255, 255, 255));
        display->setCursor(dateX, dateY);
        display->print(dateStr);
        strcpy(lastDateStr, dateStr);
    }

    // Uhrzeit: Nur geänderte Zeichen aktualisieren
    display->setTextSize(2);
    for (int i = 0; i < 8; i++) {
        if (timeStr[i] != lastTimeStr[i]) {
            clearChar(display, timeX + i * charWidth, timeY, 2);
            display->setTextColor(display->color565(255, 255, 255));
            display->setCursor(timeX + i * charWidth, timeY);
            display->print(timeStr[i]);
        }
    }
    strcpy(lastTimeStr, timeStr);
}

#endif


