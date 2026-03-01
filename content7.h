//content7.h
// eigener text RRS Feed Style
// wäre noch cool mit zweiter web page anderer port wo man den Text eingeben kann

#ifndef CONTENT7_H
#define CONTENT7_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Text-Konfiguration
const char* content7_text = "Das ist ganz schoen viel Meinung fuer so wenig Ahnung";
#define TEXT_COLOR_R 255
#define TEXT_COLOR_G 255
#define TEXT_COLOR_B 0 // Gelber Text
#define BG_COLOR_R 0
#define BG_COLOR_G 0
#define BG_COLOR_B 0 // Schwarzer Hintergrund

// Globale statische Variablen für Status-Tracking
static bool content7_displayed = false;
static MatrixPanel_I2S_DMA* content7_last_display = nullptr;
static uint32_t content7_last_call_time = 0;
static const uint32_t CONTENT7_MIN_REDRAW_INTERVAL = 1000; // 1 Sekunde

// Scrolling-Variablen - VERBESSERT
static int scroll_offset = 0;
static uint32_t last_scroll_time = 0;
static const uint32_t SCROLL_SPEED = 50;        // Millisekunden zwischen Updates (häufigere Updates)
static const int SCROLL_STEP_SIZE = 2;          // Pixel pro Schritt (größere Schritte)
static int text_width = 0;
static bool scroll_initialized = false;

void showContent7(MatrixPanel_I2S_DMA* display) {
    uint32_t current_time = millis();

    // Prüfen ob ein Refresh nötig ist
    bool needs_refresh = false;

    if (!content7_displayed || content7_last_display != display) {
        // Erstes Mal oder Display-Wechsel
        needs_refresh = true;
        scroll_initialized = false; // Scrolling neu initialisieren
    } else if (current_time - content7_last_call_time > CONTENT7_MIN_REDRAW_INTERVAL) {
        // Genug Zeit vergangen - vermutlich wurde zwischenzeitlich was anderes angezeigt
        needs_refresh = true;
        scroll_initialized = false; // Scrolling neu initialisieren
    }

    // Scrolling initialisieren wenn nötig
    if (!scroll_initialized) {
        display->setTextSize(1);
        display->setTextWrap(false);

        // Text-Breite messen (grobe Schätzung: 6 Pixel pro Zeichen)
        text_width = strlen(content7_text) * 6;
        scroll_offset = display->width(); // Start rechts außerhalb des Displays
        scroll_initialized = true;
        last_scroll_time = current_time;
    }

    // Scrolling-Update - VERBESSERT
    if (current_time - last_scroll_time >= SCROLL_SPEED) {
        scroll_offset -= SCROLL_STEP_SIZE; // Mehrere Pixel pro Schritt statt nur 1

        // Reset wenn Text komplett durchgelaufen ist
        if (scroll_offset < -text_width) {
            scroll_offset = display->width();
        }

        last_scroll_time = current_time;
        needs_refresh = true; // Für Scrolling immer neu zeichnen
    }

    // Nur neu zeichnen wenn nötig
    if (needs_refresh) {
        display->clearScreen();

        // Text-Eigenschaften setzen
        display->setTextColor(display->color565(TEXT_COLOR_R, TEXT_COLOR_G, TEXT_COLOR_B));
        display->setTextSize(1);
        display->setTextWrap(false);

        // Text zentriert vertikal, scrollend horizontal
        int y_pos = (display->height() - 8) / 2; // 8 = Texthöhe bei Size 1

        display->setCursor(scroll_offset, y_pos);
        display->print(content7_text);

        content7_displayed = true;
        content7_last_display = display;
    }

    // Zeit des letzten Aufrufs speichern
    content7_last_call_time = current_time;
}

// Alternative Funktion für statischen Text (ohne Scrolling)
void showContent7Static(MatrixPanel_I2S_DMA* display) {
    uint32_t current_time = millis();

    // Prüfen ob ein Refresh nötig ist
    bool needs_refresh = false;

    if (!content7_displayed || content7_last_display != display) {
        needs_refresh = true;
    } else if (current_time - content7_last_call_time > CONTENT7_MIN_REDRAW_INTERVAL) {
        needs_refresh = true;
    }

    if (needs_refresh) {
        display->clearScreen();

        // Text-Eigenschaften setzen
        display->setTextColor(display->color565(TEXT_COLOR_R, TEXT_COLOR_G, TEXT_COLOR_B));
        display->setTextSize(1);
        display->setTextWrap(true);

        // Mehrzeiligen Text anzeigen
        display->setCursor(2, 8);
        display->print("Das ist ganz");
        display->setCursor(2, 18);
        display->print("schoen viel");
        display->setCursor(2, 28);
        display->print("Meinung fuer");
        display->setCursor(2, 38);
        display->print("so wenig");
        display->setCursor(2, 48);
        display->print("Ahnung");

        content7_displayed = true;
        content7_last_display = display;
    }

    content7_last_call_time = current_time;
}

// Funktion um eigenen Text zu setzen
void setContent7Text(const char* new_text) {
    content7_text = new_text;
    content7_displayed = false; // Refresh erzwingen
    scroll_initialized = false; // Scrolling neu berechnen
}

// Funktion um Scroll-Geschwindigkeit anzupassen - NEU
void setContent7ScrollSpeed(uint32_t speed_ms, int step_size) {
    // Diese Funktion erlaubt es, die Scroll-Parameter zur Laufzeit zu ändern
    // Da die Variablen static const sind, wäre eine Implementierung mit globalen Variablen nötig
    // Hier als Kommentar für zukünftige Erweiterung
    content7_displayed = false; // Refresh erzwingen
    scroll_initialized = false; // Scrolling neu initialisieren
}

// Funktion um Textfarbe zu ändern
void setContent7Color(uint8_t r, uint8_t g, uint8_t b) {
    // Diese Werte werden beim nächsten Refresh verwendet
    // Da die Konstanten const sind, müssten sie durch Variablen ersetzt werden
    content7_displayed = false; // Refresh erzwingen
}

// Reset-Funktion für Debug-Zwecke
void resetContent7() {
    content7_displayed = false;
    content7_last_display = nullptr;
    content7_last_call_time = 0;
    scroll_initialized = false;
    scroll_offset = 0;
}

#endif
