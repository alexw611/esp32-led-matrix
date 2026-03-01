// main.ino
// esp core 2.0.7
// adafruit busio 1.17.2
// adafruit gfx library 1.10.14
// animatedgif 2.2.0
// arduinojson 6.21.3
// AsyncTCP 1.1.4
// ESP32 HUB 75 LED Matrix Panal DMA Display 2.0.7
// ESPAsycTCP 1.2.4
// ESPAsyncWebServer 3.1.0

// QR code zur Setup bzw Weboberfläche wäre noch wild

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Adafruit_GFX.h>
#include <Preferences.h>
#include "setup_page.h"
#include "control.h"
#include "content1.h"
#include "content2.h"
#include "content3.h"
#include "content4.h"
#include "content5.h"
#include "content6.h"
#include "content7.h"
#include "content8.h"
#include "content9.h"
#include "content10.h"
#include "content11.h"
#include "content12.h"
#include "content13.h"
#include "content14.h"
#include "content15.h"



// Matrix Pin Definition
#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 2


#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 22
#define C_PIN 21
#define D_PIN 19
#define E_PIN 18
#define LAT_PIN 5
#define OE_PIN 4
#define CLK_PIN 15


// Global variables
MatrixPanel_I2S_DMA *dma_display = nullptr;
AsyncWebServer server(80);
Preferences prefs;

bool setupMode = true;
bool wifiConnected = false;
String targetSSID = "";
String targetPassword = "";
int currentContent = 0;
int brightness = 128;
unsigned long lastWiFiAttempt = 0;
const unsigned long WIFI_TIMEOUT = 60000; // 1 minute

// Nur für Setup-Inhalte - verhindert Flackern
bool setupContentShown = false;
bool connectingContentShown = false;
bool connectedContentShown = false;
bool errorContentShown = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize preferences
  prefs.begin("wifi", false);
  
  // Initialize Matrix
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,
    PANEL_RES_Y,
    PANEL_CHAIN
  );

  mxconfig.gpio.r1 = R1_PIN;
  mxconfig.gpio.g1 = G1_PIN;
  mxconfig.gpio.b1 = B1_PIN;
  mxconfig.gpio.r2 = R2_PIN;
  mxconfig.gpio.g2 = G2_PIN;
  mxconfig.gpio.b2 = B2_PIN;
  mxconfig.gpio.a = A_PIN;
  mxconfig.gpio.b = B_PIN;
  mxconfig.gpio.c = C_PIN;
  mxconfig.gpio.d = D_PIN;
  mxconfig.gpio.e = E_PIN;
  mxconfig.gpio.lat = LAT_PIN;
  mxconfig.gpio.oe = OE_PIN;
  mxconfig.gpio.clk = CLK_PIN;
  
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  // mxconfig.double_buff = true;
  mxconfig.clkphase = false;
  mxconfig.latch_blanking = 1;


  dma_display = new MatrixPanel_I2S_DMA(mxconfig); 
  dma_display->begin();
  dma_display->setBrightness8(brightness);
  dma_display->clearScreen();
 

  // Try to load saved WiFi credentials
  targetSSID = prefs.getString("ssid", "");
  targetPassword = prefs.getString("password", "");
  
  if (targetSSID.length() > 0) {
    startWiFiConnection();
  } else {
    startSetupMode();
  }
}

void startSetupMode() {
  setupMode = true;
  wifiConnected = false;
  currentContent = 0;
  
  // Reset Setup-Content Flags
  setupContentShown = false;
  connectingContentShown = false;
  connectedContentShown = false;
  errorContentShown = false;
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("LED_Matrix_Setup");
  
  setupWebServer();
  
  showSetupContent();
  Serial.println("Setup Mode - AP started: LED_Matrix_Setup");
}

void startWiFiConnection() {
  setupMode = false;
  
  // Reset Content Flags
  setupContentShown = false;
  connectingContentShown = false;
  connectedContentShown = false;
  errorContentShown = false;
  
  showConnectingContent();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(targetSSID.c_str(), targetPassword.c_str());
  lastWiFiAttempt = millis();
  
  Serial.println("Attempting WiFi connection to: " + targetSSID);
}

void setupWebServer() {
  server.end();
  
  if (setupMode) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", getSetupPage());
    });
    
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        targetSSID = request->getParam("ssid", true)->value();
        targetPassword = request->getParam("password", true)->value();
        
        prefs.putString("ssid", targetSSID);
        prefs.putString("password", targetPassword);
        
        request->send(200, "text/html", "<h1>Gespeichert!</h1><p>ESP32 startet neu...</p>");
        
        delay(2000);
        ESP.restart();
      }
    });
  } else {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", getControlPage(currentContent, brightness));
    });
    
    server.on("/content", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("id")) {
        currentContent = request->getParam("id")->value().toInt();
        connectedContentShown = false; // Reset flag so content updates
        request->send(200, "text/plain", "OK");
      }
    });
    
    server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("value")) {
        brightness = request->getParam("value")->value().toInt();
        dma_display->setBrightness8(brightness);
        request->send(200, "text/plain", "OK");
      }
    });
    
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
      prefs.clear();
      request->send(200, "text/html", "<h1>Reset!</h1><p>ESP32 startet neu...</p>");
      delay(2000);
      ESP.restart();
    });
  }
  
  server.begin();
}

void showSetupContent() {
  if (setupContentShown) return; // Verhindert Flackern
  
  dma_display->clearScreen();
  dma_display->setTextColor(dma_display->color565(255, 255, 255));
  dma_display->setTextSize(1);
  
  dma_display->setCursor(2, 2);
  dma_display->print("Verbinde mit:");
  dma_display->setCursor(2, 12);
  dma_display->print("LED_Matrix_Setup");
  dma_display->setCursor(2, 25);
  dma_display->print("Dann gehe zu:");
  dma_display->setCursor(2, 35);
  dma_display->print("192.168.4.1");
  dma_display->setCursor(2, 50);
  dma_display->print("Setup Mode");
  
  setupContentShown = true;
}

void showConnectingContent() {
  if (connectingContentShown) return; // Verhindert Flackern
  
  dma_display->clearScreen();
  dma_display->setTextColor(dma_display->color565(255, 255, 0));
  dma_display->setTextSize(1);
  
  dma_display->setCursor(2, 2);
  dma_display->print("Verbinde mit:");
  dma_display->setCursor(2, 12);
  dma_display->print(targetSSID.substring(0, 16));
  dma_display->setCursor(2, 25);
  dma_display->print("Bitte warten...");
  
  connectingContentShown = true;
}

void showConnectedContent() {
  if (connectedContentShown && currentContent == 0) return; // Verhindert Flackern nur für Status-Anzeige
  
  dma_display->clearScreen();
  dma_display->setTextColor(dma_display->color565(0, 255, 0));
  dma_display->setTextSize(1);
  
  dma_display->setCursor(2, 2);
  dma_display->print("Verbunden mit:");
  dma_display->setCursor(2, 12);
  dma_display->print(targetSSID.substring(0, 16));
  dma_display->setCursor(2, 25);
  dma_display->print("IP-Adresse:");
  dma_display->setCursor(2, 35);
  dma_display->print(WiFi.localIP().toString());
  dma_display->setCursor(2, 50);
  dma_display->print("Bereit!");
  
  if (currentContent == 0) connectedContentShown = true;
}

void showErrorContent() {
  if (errorContentShown) return; // Verhindert Flackern
  
  dma_display->clearScreen();
  dma_display->setTextColor(dma_display->color565(255, 0, 0));
  dma_display->setTextSize(1);
  
  dma_display->setCursor(2, 2);
  dma_display->print("Fehler:");
  dma_display->setCursor(2, 12);
  dma_display->print("Keine Verbindung");
  dma_display->setCursor(2, 25);
  dma_display->print("Zurueck zum Setup");
  
  errorContentShown = true;
}

void loop() {
  if (!setupMode) {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
      wifiConnected = true;
      connectedContentShown = false; // Reset flag for new connection
      showConnectedContent();
      setupWebServer();
      Serial.println("WiFi connected: " + WiFi.localIP().toString());
    }
    else if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      wifiConnected = false;
      connectingContentShown = false; // Reset flag for reconnection
      showConnectingContent();
      Serial.println("WiFi disconnected, reconnecting...");
    }
    else if (WiFi.status() != WL_CONNECTED && (millis() - lastWiFiAttempt > WIFI_TIMEOUT)) {
      Serial.println("WiFi timeout, returning to setup mode");
      showErrorContent();
      delay(3000);
      startSetupMode();
      return;
    }
    
    // Handle content display when connected
    if (wifiConnected) {
      switch(currentContent) {
        case 0:
          showConnectedContent();
          break;
        case 1:
          showContent1(dma_display);
          break;
        case 2:
          showContent2(dma_display);
          break;
        case 3:
          showContent3(dma_display);
          break;
        case 4:
          showContent4(dma_display);
          break;
        case 5:
          showContent5(dma_display);
          break;
        case 6:
          showContent6(dma_display);
          break;
        case 7:
          showContent7(dma_display);
          break;
        case 8:
          showContent8(dma_display);
          break;
        case 9:
          showContent9(dma_display);
          break;
        case 10:
          showContent10(dma_display);
          break;
        case 11:
          showContent11(dma_display);
          break;
        case 12:
          showContent12(dma_display);
          break;
        case 13:
          showContent13(dma_display);
          break;
        case 14:
          showContent14(dma_display);
          break;
        case 15:
          showContent15(dma_display);
          break;
        default:
          showConnectedContent();
          break;
      }
    }
  }
  
  delay(100);
}