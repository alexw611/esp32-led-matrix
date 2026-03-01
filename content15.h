// content15.h
// Witze von witzapi.de anzeigen
#ifndef CONTENT15_H
#define CONTENT15_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Globale Variablen für Content15
static String currentJoke = "";
static unsigned long lastJokeUpdate = 0;
static unsigned long lastScrollUpdate = 0;
static int scrollPosition = 0;
static bool jokeLoaded = false;
static bool isScrolling = false;
static unsigned long lastCall15 = 0;

// Hilfsfunktion: Deutsche Umlaute und Sonderzeichen ersetzen
String replaceGermanChars(String text) {
    // Umlaute ersetzen
    text.replace("ä", "ae");
    text.replace("ö", "oe");
    text.replace("ü", "ue");
    text.replace("Ä", "Ae");
    text.replace("Ö", "Oe");
    text.replace("Ü", "Ue");
    text.replace("ß", "ss");
    
    // Weitere Sonderzeichen
    text.replace("é", "e");
    text.replace("è", "e");
    text.replace("ê", "e");
    text.replace("á", "a");
    text.replace("à", "a");
    text.replace("â", "a");
    text.replace("í", "i");
    text.replace("ì", "i");
    text.replace("î", "i");
    text.replace("ó", "o");
    text.replace("ò", "o");
    text.replace("ô", "o");
    text.replace("ú", "u");
    text.replace("ù", "u");
    text.replace("û", "u");
    
    // Einfache Ersetzung für problematische Zeichen
    // Alle nicht-ASCII Zeichen durch Standard-Zeichen ersetzen
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        if (c < 32 || c > 126) { // Nicht-druckbare oder Nicht-ASCII Zeichen
            if (c == '\n' || c == '\r') continue; // Zeilenumbrüche beibehalten
            text.setCharAt(i, '?'); // Durch ? ersetzen
        }
    }
    
    return text;
}

// Hilfsfunktion: Witz von API abrufen
bool fetchJoke(String &joke) {
    if (WiFi.status() != WL_CONNECTED) {
        joke = "WLAN nicht verbunden";
        return false;
    }
    
    HTTPClient http;
    http.begin("https://witzapi.de/api/joke");
    http.addHeader("User-Agent", "ESP32-Matrix");
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
        String payload = http.getString();
        
        // JSON parsen
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error && doc.is<JsonArray>() && doc.size() > 0) {
            joke = doc[0]["text"].as<String>();
            joke = replaceGermanChars(joke); // Umlaute und Sonderzeichen ersetzen
            http.end();
            return true;
        } else {
            joke = "JSON Fehler";
        }
    } else {
        joke = "API Fehler: " + String(httpResponseCode);
    }
    
    http.end();
    return false;
}

// Hilfsfunktion: Text umbrechen für Display
void wrapText(const String& text, String lines[10], int& lineCount, int maxCharsPerLine) {
    lineCount = 0;
    String currentLine = "";
    String word = "";
    
    for (int i = 0; i < text.length() && lineCount < 10; i++) {
        char c = text.charAt(i);
        
        if (c == ' ' || i == text.length() - 1) {
            if (i == text.length() - 1 && c != ' ') {
                word += c;
            }
            
            if ((currentLine + word).length() <= maxCharsPerLine) {
                currentLine += word;
                if (c == ' ' && i != text.length() - 1) {
                    currentLine += " ";
                }
            } else {
                if (currentLine.length() > 0) {
                    lines[lineCount++] = currentLine;
                    currentLine = word;
                    if (c == ' ' && i != text.length() - 1) {
                        currentLine += " ";
                    }
                } else {
                    // Wort ist zu lang für eine Zeile - aufteilen
                    lines[lineCount++] = word;
                    currentLine = "";
                }
            }
            word = "";
        } else {
            word += c;
        }
    }
    
    if (currentLine.length() > 0 && lineCount < 10) {
        lines[lineCount++] = currentLine;
    }
}

void showContent15(MatrixPanel_I2S_DMA* display) {
    unsigned long now = millis();
    static bool forceRedraw = false;
    
    // Automatische Initialisierung nach längerer Pause (Content-Wechsel)
    if (now - lastCall15 > 2000) { // 2 Sekunden ohne Aufruf = vermutlich Content-Wechsel
        forceRedraw = true;
        jokeLoaded = false;
        scrollPosition = 0;
        isScrolling = false;
        
        // Zusätzlicher vollständiger Display-Reset
        display->clearScreen();
        display->fillScreen(display->color565(0, 0, 0)); // Komplett schwarz füllen
    }
    lastCall15 = now;
    
    // Neuen Witz laden bei jedem Content-Aufruf oder wenn noch keiner geladen ist
    if (!jokeLoaded || forceRedraw) {
        display->clearScreen();
        display->setTextSize(1);
        display->setTextColor(display->color565(255, 255, 0)); // Gelb für "Lade..."
        display->setCursor(5, 28);
        display->print("Lade Witz...");
        
        if (fetchJoke(currentJoke)) {
            jokeLoaded = true;
            scrollPosition = 0;
            isScrolling = false;
            lastScrollUpdate = now;
            forceRedraw = true;
        } else {
            // Fehler anzeigen
            display->clearScreen();
            display->setTextSize(1);
            display->setTextColor(display->color565(255, 0, 0)); // Rot für Fehler
            display->setCursor(5, 20);
            display->print("Fehler beim");
            display->setCursor(5, 30);
            display->print("Laden:");
            display->setCursor(5, 40);
            display->print(currentJoke.substring(0, 18));
            return;
        }
    }
    
    if (!jokeLoaded) return;
    
    // Text in Zeilen aufteilen (ca. 21 Zeichen pro Zeile bei Textgröße 1)
    String lines[10];
    int lineCount = 0;
    wrapText(currentJoke, lines, lineCount, 21);
    
    // Prüfen ob Scrolling nötig ist (mehr als 7 Zeilen passen nicht auf Display)
    bool needsScrolling = (lineCount > 7);
    
    if (needsScrolling) {
        isScrolling = true;
        
        // Scrolling alle 3 Sekunden
        if (now - lastScrollUpdate > 3000) {
            scrollPosition++;
            if (scrollPosition > lineCount - 7) {
                scrollPosition = 0; // Zurück zum Anfang
            }
            lastScrollUpdate = now;
            forceRedraw = true;
        }
    }
    
    // Display aktualisieren
    if (forceRedraw) {
        // Doppeltes Clearing für saubere Darstellung
        display->clearScreen();
        display->fillScreen(display->color565(0, 0, 0));
        delay(10); // Kurze Pause für Display-Reset
        
        display->setTextSize(1);
        display->setTextColor(display->color565(255, 165, 0)); // Orange für Witz
        
        int startLine = isScrolling ? scrollPosition : 0;
        int maxLines = isScrolling ? 7 : min(lineCount, 7);
        
        for (int i = 0; i < maxLines && (startLine + i) < lineCount; i++) {
            int y = 2 + i * 9; // 9 Pixel Abstand zwischen Zeilen
            display->setCursor(2, y);
            display->print(lines[startLine + i]);
        }
        
        // Scroll-Indikator anzeigen
        if (needsScrolling) {
            display->setTextColor(display->color565(100, 100, 100)); // Grau
            display->setCursor(120, 57);
            display->print(String(scrollPosition + 1) + "/" + String(lineCount - 6));
        }
        
        forceRedraw = false;
    }
}

#endif