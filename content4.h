// content4.h - Wetter mit Open-Meteo API

#ifndef CONTENT4_H
#define CONTENT4_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Erweiterte Wetterdaten-Struktur
struct WeatherData {
    String city = "";
    String country = "";
    float currentTemp = 0.0;
    float maxTemp = 0.0;
    float minTemp = 0.0;
    float feelsLike = 0.0;
    int humidity = 0;
    float windSpeed = 0.0;
    float pressure = 0.0;
    int uvIndex = 0;
    int weatherCode = 0;
    String description = "";
    bool valid = false;
};

// Cache-Variablen (alle wichtigen Funktionen beibehalten)
static WeatherData currentWeather;
static WeatherData lastWeather;
static bool lastDisplayed = false;
static unsigned long lastDraw = 0;
static unsigned long lastWeatherUpdate = 0;
static unsigned long lastLocationUpdate = 0;
static unsigned long lastRotation = 0;
static String weatherSummary = "Wetter wird geladen...";
static bool weatherDataFetched = false;
static float latitude = 0.0;
static float longitude = 0.0;
static bool locationValid = false;
static int rotationState = 0; // 0=Luftfeuchtigkeit, 1=Gefühlte Temp, 2=Wind, 3=Beschreibung

const unsigned long UPDATE_INTERVAL = 10 * 60 * 1000; // 10 Minuten
const unsigned long LOCATION_UPDATE_INTERVAL = 24 * 60 * 60 * 1000; // 24 Stunden
const unsigned long ROTATION_INTERVAL = 5 * 1000; // 5 Sekunden

// Wetterbeschreibung basierend auf WMO-Codes
String getWeatherDescription(int weatherCode) {
    if (weatherCode == 0) return "Klar";
    else if (weatherCode <= 3) return "Bewoelkt";
    else if (weatherCode <= 48) return "Nebelig";
    else if (weatherCode <= 57) return "Niesel";
    else if (weatherCode <= 67) return "Regen";
    else if (weatherCode <= 77) return "Schnee";
    else if (weatherCode <= 82) return "Schauer";
    else if (weatherCode <= 86) return "Schneeschauer";
    else if (weatherCode <= 99) return "Gewitter";
    else return "Unbekannt";
}

// Frankfurt Koordinaten als Fallback (beibehaltene Funktion)
void setFrankfurtLocation() {
    currentWeather.city = "Frankfurt";
    currentWeather.country = "Germany";
    latitude = 50.1109;
    longitude = 8.6821;
    locationValid = true;
    lastLocationUpdate = millis();
}

// Manuelle Standort-Einstellung für bessere Genauigkeit
void setManualLocation() {
    // Hier kannst du deine genauen Koordinaten eintragen
    // Beispiel für Frankfurt Umgebung - anpassen für deinen genauen Standort:
    currentWeather.city = "Dreieich";  // Deine Stadt
    currentWeather.country = "Germany";
    latitude = 50.0136;  // Deine genauen Koordinaten
    longitude = 8.7784;  // Deine genauen Koordinaten  
    locationValid = true;
    lastLocationUpdate = millis();
}

// Erweiterte IP-Geolocation mit mehreren Fallback-Services und manueller Option
bool fetchLocation() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    // Option 1: Manuelle Koordinaten verwenden (auskommentieren zum Aktivieren)
    // setManualLocation();
    // return true;
    
    HTTPClient http;
    WiFiClient client;
    bool foundLocation = false;
    
    // Erst ip-api.com versuchen (kostenlos, präzise)
    if (http.begin(client, "http://ip-api.com/json/?fields=status,city,country,lat,lon,regionName")) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error && doc["status"] == "success") {
                String detectedCity = doc["city"].as<String>();
                String region = doc["regionName"].as<String>();
                
                // Plausibilitätsprüfung für Hessen/Frankfurt Region
                if (region.indexOf("Hessen") != -1 || region.indexOf("Hesse") != -1 || 
                    detectedCity.indexOf("Frankfurt") != -1 || detectedCity.indexOf("Mainz") != -1 ||
                    detectedCity.indexOf("Wiesbaden") != -1 || detectedCity.indexOf("Darmstadt") != -1) {
                    
                    currentWeather.city = detectedCity;
                    currentWeather.country = doc["country"].as<String>();
                    latitude = doc["lat"];
                    longitude = doc["lon"];
                    locationValid = true;
                    lastLocationUpdate = millis();
                    foundLocation = true;
                } else {
                    // IP-Location scheint ungenau - verwende Frankfurt als Näherung
                    Serial.println("IP-Location ungenau: " + detectedCity + ", " + region + " - verwende Frankfurt");
                }
            }
        }
        http.end();
    }
    
    if (!foundLocation) {
        // Fallback: ipapi.co versuchen
        if (http.begin(client, "http://ipapi.co/json/")) {
            int httpCode = http.GET();
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                
                DynamicJsonDocument doc(1024);
                DeserializationError error = deserializeJson(doc, payload);
                
                if (!error && doc["city"]) {
                    String detectedCity = doc["city"].as<String>();
                    String region = doc["region"].as<String>();
                    
                    // Wieder Plausibilitätsprüfung
                    if (region.indexOf("Hessen") != -1 || region.indexOf("Hesse") != -1 ||
                        detectedCity.indexOf("Frankfurt") != -1) {
                        
                        currentWeather.city = detectedCity;
                        currentWeather.country = doc["country_name"].as<String>();
                        latitude = doc["latitude"];
                        longitude = doc["longitude"];
                        locationValid = true;
                        lastLocationUpdate = millis();
                        foundLocation = true;
                    }
                }
            }
            http.end();
        }
    }
    
    // Letzter Fallback auf Frankfurt wenn IP-Location unbrauchbar
    if (!foundLocation) {
        setFrankfurtLocation();
        foundLocation = true;
    }
    
    return foundLocation;
}

// Erweiterte Wetterdaten von Open-Meteo holen
bool fetchWeatherData() {
    if (WiFi.status() != WL_CONNECTED || !locationValid) return false;
    
    HTTPClient http;
    WiFiClient client;
    
    // Erweiterte API-Anfrage mit mehr Parametern
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" + 
                 String(latitude, 4) + "&longitude=" + String(longitude, 4) + 
                 "&current_weather=true&hourly=apparent_temperature,relativehumidity_2m,windspeed_10m,surface_pressure,uv_index" +
                 "&daily=temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=1";
    
    if (!http.begin(client, url)) {
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(4096); // Größerer Buffer für mehr Daten
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            // Aktuelle Wetterdaten
            if (doc["current_weather"]["temperature"]) {
                currentWeather.currentTemp = doc["current_weather"]["temperature"];
            }
            
            if (doc["current_weather"]["weathercode"]) {
                currentWeather.weatherCode = doc["current_weather"]["weathercode"];
                currentWeather.description = getWeatherDescription(currentWeather.weatherCode);
            }
            
            if (doc["current_weather"]["windspeed"]) {
                currentWeather.windSpeed = doc["current_weather"]["windspeed"];
            }
            
            // Tages-Min/Max
            if (doc["daily"]["temperature_2m_max"][0]) {
                currentWeather.maxTemp = doc["daily"]["temperature_2m_max"][0];
            }
            if (doc["daily"]["temperature_2m_min"][0]) {
                currentWeather.minTemp = doc["daily"]["temperature_2m_min"][0];
            }
            
            // Stündliche Daten für aktuelle Stunde (Index 0)
            if (doc["hourly"]["apparent_temperature"][0]) {
                currentWeather.feelsLike = doc["hourly"]["apparent_temperature"][0];
            }
            
            if (doc["hourly"]["relativehumidity_2m"][0]) {
                currentWeather.humidity = doc["hourly"]["relativehumidity_2m"][0];
            }
            
            if (doc["hourly"]["surface_pressure"][0]) {
                currentWeather.pressure = doc["hourly"]["surface_pressure"][0];
            }
            
            if (doc["hourly"]["uv_index"][0]) {
                currentWeather.uvIndex = doc["hourly"]["uv_index"][0];
            }
            
            currentWeather.valid = true;
            weatherDataFetched = true;
            lastWeatherUpdate = millis();
            
            http.end();
            return true;
        }
    }
    
    http.end();
    return false;
}

// Beibehaltene Fallback-Funktionen
void parseWeatherString(const String& raw, String& city, String& country, String& temperature, String& wind) {
    int colonIndex = raw.indexOf(':');
    String location = (colonIndex != -1) ? raw.substring(0, colonIndex) : raw;
    location.trim();

    int commaIndex = location.indexOf(',');
    if (commaIndex != -1) {
        city = location.substring(0, commaIndex);
        country = location.substring(commaIndex + 1);
    } else {
        city = location;
        country = "";
    }
    city.trim();
    country.trim();

    String rest = (colonIndex != -1) ? raw.substring(colonIndex + 1) : "";

    int tempStart = rest.indexOf('+');
    if (tempStart == -1) tempStart = rest.indexOf('-');
    int tempEnd = rest.indexOf('C', tempStart);
    if (tempStart != -1 && tempEnd != -1 && tempEnd > tempStart) {
        temperature = rest.substring(tempStart, tempEnd + 1);
        temperature.replace("°", "");
    } else {
        temperature = "N/A";
    }

    int windIndex = rest.indexOf("km/h");
    if (windIndex != -1) {
        int numStart = windIndex - 1;
        while (numStart > 0 && isDigit(rest[numStart - 1])) numStart--;
        wind = rest.substring(numStart, windIndex + 4);
        wind.replace("→", "");
        wind.replace("←", "");
        wind.replace("↑", "");
        wind.replace("↓", "");
        wind.replace("-", "");
        wind.trim();
    } else {
        wind = "N/A";
    }
}

void fetchWeatherSummary() {
    if (WiFi.status() != WL_CONNECTED) {
        weatherSummary = "Kein WLAN";
        weatherDataFetched = false;
        return;
    }
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, "https://wttr.in/?format=4")) {
        weatherSummary = "Wetter Fehler (begin)";
        weatherDataFetched = false;
        return;
    }
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String raw = http.getString();
        raw.trim();
        weatherSummary = raw;
        weatherDataFetched = true;
    } else {
        weatherSummary = "Wetter Fehler (" + String(httpCode) + ")";
        weatherDataFetched = false;
    }
    http.end();
}

// Beibehaltene Textbreiten-Funktion
int getTextWidth(const String& text) {
    int width = 0;
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        if (c == 'i' || c == 'l' || c == '!' || c == ':' || c == '.' || c == ',') {
            width += 2;
        } else if (c == 'm' || c == 'w' || c == 'M' || c == 'W') {
            width += 6;
        } else if (c == ' ') {
            width += 3;
        } else {
            width += 5;
        }
        if (i < text.length() - 1) width += 1;
    }
    return width;
}

// Optimierte Hauptfunktion mit rotierenden Nebendaten
void showContent4(MatrixPanel_I2S_DMA* display) {
    static unsigned long lastCall = 0;
    unsigned long now = millis();
    bool forceRedraw = false;
    
    if (now - lastCall > 2000) forceRedraw = true; // Content-Wechsel erkannt
    lastCall = now;

    // Standort bei erstem Aufruf oder alle 24h aktualisieren
    if (!locationValid || now - lastLocationUpdate > LOCATION_UPDATE_INTERVAL) {
        if (fetchLocation()) {
            forceRedraw = true;
        }
    }

    // Wetterdaten aktualisieren
    if (!weatherDataFetched || now - lastWeatherUpdate > UPDATE_INTERVAL || forceRedraw) {
        bool success = false;
        
        if (locationValid) {
            success = fetchWeatherData();
        }
        
        // Fallback auf alte Methode wenn Open-Meteo nicht funktioniert
        if (!success) {
            fetchWeatherSummary();
        }
        
        if (success || weatherDataFetched) {
            forceRedraw = true;
        }
    }

    // Rotation alle 5 Sekunden
    if (now - lastRotation > ROTATION_INTERVAL) {
        lastRotation = now;
        rotationState = (rotationState + 1) % 4;
        forceRedraw = true; // Neuzeichnung für Rotation
    }

    // Prüfen ob Neuzeichnung nötig ist
    bool changed = forceRedraw ||
        currentWeather.city != lastWeather.city ||
        currentWeather.country != lastWeather.country ||
        abs(currentWeather.currentTemp - lastWeather.currentTemp) > 0.1 ||
        abs(currentWeather.maxTemp - lastWeather.maxTemp) > 0.1 ||
        abs(currentWeather.minTemp - lastWeather.minTemp) > 0.1 ||
        currentWeather.weatherCode != lastWeather.weatherCode ||
        currentWeather.valid != lastWeather.valid;

    if (!changed && lastDisplayed) return; // Anti-Flicker
    
    // Cache aktualisieren
    lastWeather = currentWeather;
    lastDraw = now;

    // Display clearing (beibehaltene Methode)
    for (int clear = 0; clear < 3; clear++) {
        display->clearScreen();
        display->fillScreen(0);
        delayMicroseconds(100);
    }
    
    display->setTextSize(1);
    display->setTextColor(display->color565(255, 255, 255), 0);
    display->setTextWrap(false);
    
    if (currentWeather.valid && locationValid) {
        // Optimierte 4-Zeilen Darstellung für 128x64 Display
        const int DISPLAY_WIDTH = 128;
        const int DISPLAY_HEIGHT = 64;
        const int CENTER_X = DISPLAY_WIDTH / 2;
        
        // Zeile 1: Stadt (zentriert, oben)
        String cityText = currentWeather.city;
        if (cityText.length() > 18) cityText = cityText.substring(0, 18);
        
        int cityWidth = getTextWidth(cityText);
        int cityX = CENTER_X - (cityWidth / 2);
        int cityY = 8;
        
        display->fillRect(0, cityY - 2, DISPLAY_WIDTH, 12, 0);
        display->setCursor(cityX, cityY);
        display->setTextColor(display->color565(100, 150, 255), 0);
        display->print(cityText);
        
        // Zeile 2: Aktuelle Temperatur (zentriert) - mit Komma-Darstellung
        String currentLine = String(currentWeather.currentTemp, 1) + "C";
        int currentWidth = getTextWidth(currentLine);
        int currentX = CENTER_X - (currentWidth / 2);
        int currentY = 22;
        
        display->fillRect(0, currentY - 2, DISPLAY_WIDTH, 12, 0);
        display->setCursor(currentX, currentY);
        display->setTextColor(display->color565(255, 255, 100), 0);
        display->print(currentLine);
        
        // Zeile 3: Min/Max Temperatur (zentriert) - mit Komma-Darstellung
        String minMaxLine = String(currentWeather.minTemp, 1) + "/" + 
                           String(currentWeather.maxTemp, 1) + "C";
        int minMaxWidth = getTextWidth(minMaxLine);
        int minMaxX = CENTER_X - (minMaxWidth / 2);
        int minMaxY = 36;
        
        display->fillRect(0, minMaxY - 2, DISPLAY_WIDTH, 12, 0);
        display->setCursor(minMaxX, minMaxY);
        display->setTextColor(display->color565(255, 200, 100), 0);
        display->print(minMaxLine);
        
        // Zeile 4: Rotierende Nebendaten (zentriert)
        String extraText = "";
        switch (rotationState) {
            case 0:
                extraText = "Feuchte: " + String(currentWeather.humidity) + "%";
                break;
            case 1:
                extraText = "Gefuehlt: " + String(currentWeather.feelsLike, 1) + "C";
                break;
            case 2:
                extraText = "Wind: " + String(currentWeather.windSpeed, 1) + " km/h";
                break;
            case 3:
                if (currentWeather.description.length() > 0) {
                    extraText = currentWeather.description;
                } else {
                    extraText = "UV: " + String(currentWeather.uvIndex);
                }
                break;
        }
        
        if (extraText.length() > 0) {
            int extraWidth = getTextWidth(extraText);
            int extraX = CENTER_X - (extraWidth / 2);
            int extraY = 50;
            
            display->fillRect(0, extraY - 2, DISPLAY_WIDTH, 12, 0);
            display->setCursor(extraX, extraY);
            display->setTextColor(display->color565(200, 200, 200), 0);
            display->print(extraText);
        }
        
    } else {
        // Beibehaltene Fallback-Darstellung
        const int DISPLAY_WIDTH = 128;
        const int DISPLAY_HEIGHT = 64;
        
        String lines[4];
        
        if (weatherDataFetched && !currentWeather.valid) {
            String city, country, temperature, wind;
            parseWeatherString(weatherSummary, city, country, temperature, wind);
            
            lines[0] = city;
            lines[1] = country;
            lines[2] = "Temp: " + temperature;
            lines[3] = "Wind: " + wind;
        } else {
            lines[0] = weatherSummary;
            lines[1] = "";
            lines[2] = "";
            lines[3] = "";
        }

        int lineHeight = 8;
        int lineSpacing = 4;
        int numLines = 4;
        int totalHeight = numLines * lineHeight + (numLines - 1) * lineSpacing;
        int y0 = (DISPLAY_HEIGHT - totalHeight) / 2;

        for (int i = 0; i < numLines; i++) {
            if (lines[i].length() > 0) {
                int textWidth = getTextWidth(lines[i]);
                int x = (DISPLAY_WIDTH - textWidth) / 2;
                if (x < 0) x = 0;
                
                int y = y0 + i * (lineHeight + lineSpacing);
                
                display->fillRect(0, y - 2, DISPLAY_WIDTH, lineHeight + 4, 0);
                display->setCursor(x, y);
                display->setTextColor(display->color565(255, 255, 255), 0);
                display->print(lines[i]);
                
                delayMicroseconds(50);
            }
        }
    }
    
    lastDisplayed = true;
}

#endif