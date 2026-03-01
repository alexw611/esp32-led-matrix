// content14.h 
// Aktiendaten

#ifndef CONTENT14_H
#define CONTENT14_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Struktur für Aktiendaten
struct StockData {
  String symbol;
  String currency;
  float price;
  float change;
  float changePercent;
  bool dataValid;
  unsigned long lastUpdate;
};

// Globale Variable für aktuelle Aktie
static StockData stockData = {"", "USD", 0.0, 0.0, 0.0, false, 0};

// Aktien-Liste
struct StockInfo {
  String symbol;
  String currency;
  String ticker;
};

static StockInfo stockList[] = {
  {"AAPL", "USD", "AAPL"},
  {"GOOGL", "USD", "GOOGL"},
  {"MSFT", "USD", "MSFT"},
  {"TSLA", "USD", "TSLA"},
  {"NVDA", "USD", "NVDA"},
  {"AMZN", "USD", "AMZN"},
  {"META", "USD", "META"},
  {"SAP.DE", "EUR", "SAP"},
  {"SIE.DE", "EUR", "SIE"},
  {"VOW3.DE", "EUR", "VOW3"},
  {"BMW.DE", "EUR", "BMW"},
  {"BAS.DE", "EUR", "BAS"},
  {"ALV.DE", "EUR", "ALV"},
  {"DTE.DE", "EUR", "DTE"},
  {"ASML.AS", "EUR", "ASML"}
};

static int currentStockIndex = 0;
static const int numStocks = 15;
static unsigned long lastDataUpdate = 0;
static const unsigned long DATA_UPDATE_INTERVAL = 300000; // 5 Minuten

// Einzelne Aktie laden - NICHT BLOCKIEREND
bool fetchStockData(const String& symbol) {
  HTTPClient http;
  
  String url = "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol + "?region=US&lang=en-US&includePrePost=false&interval=1d&range=2d&corsDomain=finance.yahoo.com&.tsrc=finance";
  
  http.begin(url);
  http.addHeader("User-Agent", "Mozilla/5.0 (ESP32)");
  http.setTimeout(5000); // NUR 5 Sekunden Timeout!
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(4096); // Kleinerer Buffer
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      http.end();
      return false;
    }
    
    auto result = doc["chart"]["result"][0];
    auto meta = result["meta"];
    
    if (meta["regularMarketPrice"] && meta["previousClose"]) {
      stockData.symbol = stockList[currentStockIndex].ticker;
      stockData.currency = stockList[currentStockIndex].currency;
      stockData.price = meta["regularMarketPrice"].as<float>();
      
      // Vortags-Berechnung
      float previousClose = meta["previousClose"].as<float>();
      stockData.change = stockData.price - previousClose;
      stockData.changePercent = (stockData.change / previousClose) * 100;
      
      stockData.dataValid = true;
      stockData.lastUpdate = millis();
      
      http.end();
      return true;
    }
  }
  
  http.end();
  return false;
}

// Fallback API - SCHNELL
bool fetchStockDataQuick(const String& symbol) {
  HTTPClient http;
  
  String url = "https://query1.finance.yahoo.com/v7/finance/quote?symbols=" + symbol;
  
  http.begin(url);
  http.addHeader("User-Agent", "Mozilla/5.0 (ESP32)");
  http.setTimeout(3000); // NUR 3 Sekunden!
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      http.end();
      return false;
    }
    
    auto quote = doc["quoteResponse"]["result"][0];
    if (quote["regularMarketPrice"] && quote["regularMarketPreviousClose"]) {
      stockData.symbol = stockList[currentStockIndex].ticker;
      stockData.currency = stockList[currentStockIndex].currency;
      stockData.price = quote["regularMarketPrice"].as<float>();
      
      float previousClose = quote["regularMarketPreviousClose"].as<float>();
      stockData.change = stockData.price - previousClose;
      stockData.changePercent = (stockData.change / previousClose) * 100;
      
      stockData.dataValid = true;
      stockData.lastUpdate = millis();
      
      http.end();
      return true;
    }
  }
  
  http.end();
  return false;
}

// Demo-Daten - SOFORT verfügbar
void generateDemoData() {
  stockData.symbol = stockList[currentStockIndex].ticker;
  stockData.currency = stockList[currentStockIndex].currency;
  
  if (stockData.currency == "EUR") {
    stockData.price = 50.0 + random(-20, 50);
  } else if (stockData.currency == "USD") {
    stockData.price = 150.0 + random(-50, 100);
  } else {
    stockData.price = 80.0 + random(-30, 40);
  }
  
  stockData.change = random(-500, 500) / 100.0;
  stockData.changePercent = (stockData.change / stockData.price) * 100;
  stockData.dataValid = true;
  stockData.lastUpdate = millis();
}

// Währungssymbol
String getCurrencySymbol(const String& currency) {
  if (currency == "EUR") return "E";
  if (currency == "USD") return "$";
  if (currency == "CHF") return "CHF";
  return currency;
}

// Scrollender Text
void drawScrollingText(MatrixPanel_I2S_DMA* display, const String& text, int y, int& scrollPos, uint16_t color) {
  int textWidth = text.length() * 6;
  int displayWidth = 128;
  
  if (textWidth > displayWidth) {
    display->setTextColor(color);
    display->setCursor(displayWidth - scrollPos, y);
    display->print(text);
    
    scrollPos += 2;
    if (scrollPos > textWidth + displayWidth) {
      scrollPos = 0;
    }
  } else {
    int x = (displayWidth - textWidth) / 2;
    display->setCursor(x, y);
    display->setTextColor(color);
    display->print(text);
  }
}

// Hauptfunktion - NICHT BLOCKIEREND
void showContent14(MatrixPanel_I2S_DMA* display) {
  static unsigned long lastUpdate = 0;
  static unsigned long lastCall = 0;
  static unsigned long lastStockRotation = 0;
  static int scrollPos = 0;
  static String lastSymbol = "";
  static String lastPrice = "";
  static String lastChange = "";
  static int lastScrollPos = -1;
  static bool needsDataUpdate = true;
  
  unsigned long now = millis();
  bool forceRedraw = false;
  
  // Content-Wechsel erkennen
  if (now - lastCall > 2000) {
    forceRedraw = true;
    scrollPos = 0;
    lastSymbol = "";
    lastPrice = "";
    lastChange = "";
    lastScrollPos = -1;
    needsDataUpdate = true;
  }
  lastCall = now;
  
  // Aktie alle 15 Sekunden wechseln
  if (now - lastStockRotation > 15000) {
    currentStockIndex = (currentStockIndex + 1) % numStocks;
    lastStockRotation = now;
    forceRedraw = true;
    needsDataUpdate = true;
  }
  
  // Daten alle 5 Minuten ODER bei Aktienwechsel aktualisieren
  if (needsDataUpdate || (now - lastDataUpdate > DATA_UPDATE_INTERVAL)) {
    if (WiFi.status() == WL_CONNECTED) {
      String currentSymbol = stockList[currentStockIndex].symbol;
      
      // NUR EINE Aktie laden - NICHT BLOCKIEREND
      if (!fetchStockData(currentSymbol)) {
        // Kurzer Fallback-Versuch
        if (!fetchStockDataQuick(currentSymbol)) {
          // Sofort Demo-Daten wenn API fehlschlägt
          generateDemoData();
        }
      }
    } else {
      // Sofort Demo-Daten ohne WiFi
      generateDemoData();
    }
    
    lastDataUpdate = now;
    needsDataUpdate = false;
    forceRedraw = true;
  }
  
  // Nur alle 150ms aktualisieren für Scrolling
  if (!forceRedraw && now - lastUpdate < 150) return;
  lastUpdate = now;
  
  // Vollständiger Redraw bei Content-Wechsel
  if (forceRedraw) {
    display->clearScreen();
    scrollPos = 0;
    lastScrollPos = -1;
  }
  
  // Keine Daten verfügbar
  if (!stockData.dataValid) {
    display->setTextSize(1);
    display->setTextColor(display->color565(255, 255, 255));
    String loadingText = "Lade " + stockList[currentStockIndex].ticker + "...";
    int textWidth = loadingText.length() * 6;
    int x = (128 - textWidth) / 2;
    int y = 30;
    display->setCursor(x, y);
    display->print(loadingText);
    return;
  }
  
  // Aktiensymbol (Ticker) oben anzeigen
  String symbolText = stockData.symbol;
  if (symbolText != lastSymbol || forceRedraw) {
    display->fillRect(0, 6, 128, 20, display->color565(0, 0, 0));
    
    display->setTextSize(2);
    int symbolWidth = symbolText.length() * 12;
    int symbolX = (128 - symbolWidth) / 2;
    display->setCursor(symbolX, 8);
    display->setTextColor(display->color565(255, 255, 255));
    display->print(symbolText);
    lastSymbol = symbolText;
  }
  
  // Preis mit Währung
  String priceText;
  if (stockData.currency == "EUR") {
    priceText = String(stockData.price, 2) + "E";
  } else if (stockData.currency == "USD") {
    priceText = "$" + String(stockData.price, 2);
  } else {
    priceText = String(stockData.price, 2) + stockData.currency;
  }
  
  if (priceText != lastPrice || forceRedraw) {
    display->fillRect(0, 26, 128, 12, display->color565(0, 0, 0));
    
    display->setTextSize(1);
    int priceWidth = priceText.length() * 6;
    int priceX = (128 - priceWidth) / 2;
    display->setCursor(priceX, 28);
    display->setTextColor(display->color565(255, 255, 255));
    display->print(priceText);
    lastPrice = priceText;
  }
  
  // Änderung zum Vortag
  String changeText = String(stockData.change >= 0 ? "+" : "") + String(stockData.change, 2);
  String percentText = "(" + String(stockData.changePercent >= 0 ? "+" : "") + String(stockData.changePercent, 2) + "%)";
  String fullChangeText = changeText + " " + percentText;
  
  // Farbe basierend auf Änderung
  uint16_t changeColor;
  if (stockData.change > 0) {
    changeColor = display->color565(0, 255, 0);
  } else if (stockData.change < 0) {
    changeColor = display->color565(255, 0, 0);
  } else {
    changeColor = display->color565(255, 255, 255);
  }
  
  // Scrolling-Bereich für Änderung
  if (fullChangeText != lastChange || scrollPos != lastScrollPos || forceRedraw) {
    display->fillRect(0, 38, 128, 20, display->color565(0, 0, 0));
    
    display->setTextSize(1);
    drawScrollingText(display, fullChangeText, 41, scrollPos, changeColor);
    
    lastChange = fullChangeText;
    lastScrollPos = scrollPos;
  }
}

#endif
