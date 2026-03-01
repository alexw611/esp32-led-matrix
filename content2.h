// content2.h
// Tagesschau RRS feed

// in psram laden 
// falls die verbindung nicht klappt nochmal probieren hinzufügen machnmal gehts manchmal nicht

#ifndef CONTENT2_H
#define CONTENT2_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Tagesschau RSS-Feed (funktioniert zuverlässig)
const char* RSS_URL = "https://www.tagesschau.de/infoservices/alle-meldungen-100~rss2.xml";
const unsigned long RSS_UPDATE_INTERVAL = 300000; // 5 Minuten

String rssTickerText = "Lade Nachrichten...";
int textX = 128; // Start rechts
unsigned long lastUpdateTime = 0;
unsigned long lastRSSUpdate = 0;
bool rssLoadInProgress = false;

// Verbesserte Zeichenbereinigung
String cleanText(String input) {
    // HTML-Entities
    input.replace("&amp;", "&");
    input.replace("&quot;", "\"");
    input.replace("&lt;", "<");
    input.replace("&gt;", ">");
    input.replace("&#039;", "'");
    input.replace("&apos;", "'");
    input.replace("&nbsp;", " ");
    
    // Deutsche Umlaute
    input.replace("ä", "ae");
    input.replace("ö", "oe");
    input.replace("ü", "ue");
    input.replace("Ä", "Ae");
    input.replace("Ö", "Oe");
    input.replace("Ü", "Ue");
    input.replace("ß", "ss");
    
    // Unicode-Zeichen
    input.replace("\u2019", "'");     // Rechtes einfaches Anführungszeichen
    input.replace("\u201E", "\"");    // Deutsches Anführungszeichen unten
    input.replace("\u201C", "\"");    // Linkes doppeltes Anführungszeichen
    input.replace("\u201D", "\"");    // Rechtes doppeltes Anführungszeichen
    input.replace("\u2013", "-");     // En dash
    input.replace("\u2014", "-");     // Em dash
    input.replace("\u2026", "...");   // Ellipsis
    input.replace("–", "-");
    input.replace("—", "-");
    input.replace("…", "...");
    
    // Unerwünschte Zeichen entfernen
    input.replace("+", "");
    input.replace("\n", " ");
    input.replace("\r", "");
    input.replace("\t", " ");
    
    // Mehrfache Leerzeichen reduzieren
    while (input.indexOf("  ") != -1) {
        input.replace("  ", " ");
    }
    
    input.trim();
    return input;
}

// CDATA und HTML-Tags komplett entfernen
String extractCleanTitle(String input) {
    String result = input;
    
    // CDATA entfernen
    int cdataStart = result.indexOf("<![CDATA[");
    if (cdataStart != -1) {
        cdataStart += 9; // Länge von "<![CDATA["
        int cdataEnd = result.indexOf("]]>", cdataStart);
        if (cdataEnd != -1) {
            result = result.substring(cdataStart, cdataEnd);
        }
    }
    
    // Alle HTML-Tags entfernen
    while (true) {
        int tagStart = result.indexOf('<');
        if (tagStart == -1) break;
        int tagEnd = result.indexOf('>', tagStart);
        if (tagEnd == -1) break;
        result = result.substring(0, tagStart) + result.substring(tagEnd + 1);
    }
    
    // Text bereinigen
    result = cleanText(result);
    
    return result;
}

// Verbesserte RSS-Parsing-Funktion
void fetchRSSFeed() {
    if (rssLoadInProgress) {
        Serial.println("RSS-Laden bereits in Bearbeitung, überspringe...");
        return;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Keine WLAN-Verbindung – RSS wird nicht geladen");
        rssTickerText = "Keine WLAN-Verbindung...";
        return;
    }

    rssLoadInProgress = true;
    Serial.println("Starte RSS-Download von Tagesschau...");

    HTTPClient http;
    http.begin(RSS_URL);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000);
    http.addHeader("User-Agent", "ESP32NewsDisplay/1.0");
    http.addHeader("Accept", "application/rss+xml, text/xml");

    int httpCode = http.GET();
    Serial.printf("HTTP-Code: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("Payload-Größe: %d Bytes\n", payload.length());
        
        if (payload.length() < 100) {
            Serial.println("Payload zu klein, möglicherweise ungültig");
            rssTickerText = "RSS-Feed fehlerhaft...";
            http.end();
            rssLoadInProgress = false;
            return;
        }
        
        // Nach <item> Tags suchen statt nur <title>
        int startPos = 0;
        String tempTickerText = "";
        int titleCount = 0;

        while (titleCount < 8) { // Maximal 8 Titel laden
            // Suche nach <item>
            int itemStart = payload.indexOf("<item>", startPos);
            if (itemStart == -1) break;
            
            // Suche nach </item>
            int itemEnd = payload.indexOf("</item>", itemStart);
            if (itemEnd == -1) break;
            
            // Extrahiere den Item-Inhalt
            String itemContent = payload.substring(itemStart, itemEnd);
            
            // Suche nach <title> innerhalb des Items
            int titleStart = itemContent.indexOf("<title>");
            if (titleStart != -1) {
                titleStart += 7;
                int titleEnd = itemContent.indexOf("</title>", titleStart);
                if (titleEnd != -1) {
                    String title = itemContent.substring(titleStart, titleEnd);
                    
                    // Saubere Titel-Extraktion mit HTML-Tag-Entfernung
                    title = extractCleanTitle(title);
                    
                    // Nur gültige Titel verwenden (keine Domain-Namen etc.)
                    if (title.length() > 10 && title.length() < 150 && 
                        title.indexOf("cnn.com") == -1 && 
                        title.indexOf("CNN.com") == -1 &&
                        title.indexOf("http") == -1) {
                        
                        tempTickerText += title + "   +++   ";
                        titleCount++;
                        Serial.println("Titel " + String(titleCount) + ": " + title);
                    } else {
                        Serial.println("Titel übersprungen: " + title);
                    }
                }
            }
            
            startPos = itemEnd + 7;
        }

        if (tempTickerText.length() > 0) {
            rssTickerText = tempTickerText;
            Serial.println("RSS erfolgreich geladen");
            Serial.println("Anzahl Titel: " + String(titleCount));
        } else {
            rssTickerText = "Keine gültigen Nachrichten gefunden...";
            Serial.println("Keine gültigen Titel gefunden");
        }

    } else {
        Serial.printf("RSS Fehler: HTTP-Code %d\n", httpCode);
        if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
            rssTickerText = "Verbindung verweigert...";
        } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
            rssTickerText = "Timeout beim Laden...";
        } else if (httpCode == 403) {
            rssTickerText = "Zugriff verweigert...";
        } else {
            rssTickerText = "Fehler beim Laden (Code: " + String(httpCode) + ")...";
        }
    }

    http.end();
    rssLoadInProgress = false;
}

// Debug-Funktion für Titel-Extraktion
void debugTitleExtraction(String payload) {
    Serial.println("=== DEBUG: Titel-Extraktion ===");
    int itemStart = payload.indexOf("<item>");
    if (itemStart != -1) {
        int itemEnd = payload.indexOf("</item>", itemStart);
        if (itemEnd != -1) {
            String firstItem = payload.substring(itemStart, itemEnd + 7);
            Serial.println("Erstes Item:");
            Serial.println(firstItem.substring(0, min(500, (int)firstItem.length())));
        }
    }
    Serial.println("===============================");
}

// RSS-Status prüfen (für Debugging)
void printRSSStatus() {
    Serial.println("=== RSS Status ===");
    Serial.println("Feed URL: " + String(RSS_URL));
    Serial.println("WLAN Status: " + String(WiFi.status() == WL_CONNECTED ? "Verbunden" : "Nicht verbunden"));
    Serial.println("Letztes RSS Update: " + String((millis() - lastRSSUpdate) / 1000) + " Sekunden her");
    Serial.println("Ticker Text Länge: " + String(rssTickerText.length()));
    Serial.println("Aktueller Text: " + rssTickerText.substring(0, min(100, (int)rssTickerText.length())));
    Serial.println("Load in Progress: " + String(rssLoadInProgress ? "Ja" : "Nein"));
    Serial.println("==================");
}

// Anzeige-Funktion
void showContent2(MatrixPanel_I2S_DMA* display) {
    unsigned long now = millis();

    // RSS bei Bedarf laden
    if (now - lastRSSUpdate > RSS_UPDATE_INTERVAL || rssTickerText == "Lade Nachrichten...") {
        fetchRSSFeed();
        lastRSSUpdate = now;
        textX = display->width(); // Neustart von rechts
    }

    // Scroll alle 50ms
    if (now - lastUpdateTime < 50) return;
    lastUpdateTime = now;

    display->clearScreen();

    int charWidth = 6;       // Zeichenbreite Font 1
    int charHeight = 7;      // Zeichenhöhe Font 1

    // Texthöhe = 7, Displayhöhe = 64 → vertikal mittig platzieren
    int textY = (display->height() - charHeight) / 2;

    // Einzelne sichtbare Zeichen zeichnen
    for (int i = 0; i < rssTickerText.length(); i++) {
        int x = textX + i * charWidth;
        if (x < -charWidth || x >= display->width()) continue;

        display->drawChar(x, textY, rssTickerText.charAt(i), display->color565(255, 255, 255), 0, 1);
    }

    textX -= 2;

    int textPixelWidth = rssTickerText.length() * charWidth;
    if (textX < -textPixelWidth) {
        textX = display->width();
    }
}

#endif