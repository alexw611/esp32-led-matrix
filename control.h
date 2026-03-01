// control.h
#ifndef CONTROL_H
#define CONTROL_H
String getControlPage(int currentContent, int currentBrightness) {
  String html = "<!DOCTYPE html><html><head><title>LED Matrix Control</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta charset=\"UTF-8\">";
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }";
  html += ".container { max-width: 480px; margin: 0 auto; background: rgba(255,255,255,0.95); backdrop-filter: blur(20px); border-radius: 20px; padding: 30px; box-shadow: 0 20px 40px rgba(0,0,0,0.1); }";
  html += "h1 { color: #1d1d1f; font-size: 28px; font-weight: 600; text-align: center; margin-bottom: 30px; }";
  html += ".section { margin-bottom: 32px; }";
  html += ".section-title { color: #1d1d1f; font-size: 20px; font-weight: 600; margin-bottom: 16px; }";
  html += ".content-grid { display: grid; grid-template-columns: 1fr; gap: 12px; }";
  html += ".content-btn { width: 100%; padding: 16px; border: 2px solid #f5f5f7; border-radius: 12px; background: #fbfbfd; color: #1d1d1f; font-size: 15px; font-weight: 500; cursor: pointer; transition: all 0.3s ease; text-align: left; }";
  html += ".content-btn:hover { background: #f5f5f7; transform: translateY(-1px); }";
  html += ".content-btn.active { background: linear-gradient(135deg, #007aff 0%, #0051d5 100%); color: white; border-color: #007aff; box-shadow: 0 8px 20px rgba(0,122,255,0.3); }";
  html += ".content-btn.active:hover { background: linear-gradient(135deg, #0051d5 0%, #003d99 100%); }";
  html += ".brightness-container { background: #f5f5f7; border-radius: 16px; padding: 20px; }";
  html += ".brightness-label { color: #1d1d1f; font-size: 16px; font-weight: 500; margin-bottom: 12px; }";
  html += ".slider-container { position: relative; margin-bottom: 12px; }";
  html += ".brightness-slider { width: 100%; height: 8px; border-radius: 4px; background: #d1d1d6; outline: none; appearance: none; cursor: pointer; }";
  html += ".brightness-slider::-webkit-slider-thumb { appearance: none; width: 24px; height: 24px; border-radius: 50%; background: #007aff; cursor: pointer; box-shadow: 0 2px 8px rgba(0,122,255,0.3); }";
  html += ".brightness-slider::-moz-range-thumb { width: 24px; height: 24px; border-radius: 50%; background: #007aff; cursor: pointer; border: none; box-shadow: 0 2px 8px rgba(0,122,255,0.3); }";
  html += ".brightness-value { color: #86868b; font-size: 14px; text-align: center; }";
  html += ".system-section { background: #fff3cd; border: 1px solid #ffeaa7; border-radius: 16px; padding: 20px; }";
  html += ".reset-btn { width: 100%; padding: 16px; background: linear-gradient(135deg, #ff3b30 0%, #d70015 100%); color: white; border: none; border-radius: 12px; font-size: 16px; font-weight: 600; cursor: pointer; transition: all 0.3s ease; }";
  html += ".reset-btn:hover { transform: translateY(-2px); box-shadow: 0 10px 25px rgba(255,59,48,0.3); }";
  html += ".reset-btn:active { transform: translateY(0); }";
  html += ".divider { height: 1px; background: linear-gradient(90deg, transparent, #d2d2d7, transparent); margin: 24px 0; }";
  html += ".auto-refresh { color: #86868b; font-size: 14px; text-align: center; margin-top: 20px; }";
  html += ".emoji { font-size: 16px; margin-right: 8px; }";
  html += "@media (max-width: 480px) { .container { margin: 10px; padding: 20px; } h1 { font-size: 24px; } .content-grid { grid-template-columns: 1fr; } }";
  html += "@media (min-width: 600px) { .content-grid { grid-template-columns: 1fr 1fr; } }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>LED Matrix Steuerung</h1>";
  
  html += "<div class='section'>";
  html += "<h2 class='section-title'>Content auswählen</h2>";
  html += "<div class='content-grid'>";
  html += "<button onclick='setContent(0)' class='content-btn" + String(currentContent == 0 ? " active" : "") + "'>";
  html += "<span class='emoji'>ℹ️</span>System Info</button>";
  html += "<button onclick='setContent(1)' class='content-btn" + String(currentContent == 1 ? " active" : "") + "'>";
  html += "<span class='emoji'>🕐</span>NTP Digital Uhr</button>";
  html += "<button onclick='setContent(2)' class='content-btn" + String(currentContent == 2 ? " active" : "") + "'>";
  html += "<span class='emoji'>📰</span>RSS Feed Tagesschau</button>";
  html += "<button onclick='setContent(3)' class='content-btn" + String(currentContent == 3 ? " active" : "") + "'>";
  html += "<span class='emoji'>🌈</span>Regenbogen Animation</button>";
  html += "<button onclick='setContent(4)' class='content-btn" + String(currentContent == 4 ? " active" : "") + "'>";
  html += "<span class='emoji'>🌤️</span>Wetter</button>";
  html += "<button onclick='setContent(5)' class='content-btn" + String(currentContent == 5 ? " active" : "") + "'>";
  html += "<span class='emoji'>🖼️</span>Bild.xbm aus GitHub</button>";
  html += "<button onclick='setContent(6)' class='content-btn" + String(currentContent == 6 ? " active" : "") + "'>";
  html += "<span class='emoji'>🎨</span>lokale Bitmap Pixelart</button>";
  html += "<button onclick='setContent(7)' class='content-btn" + String(currentContent == 7 ? " active" : "") + "'>";
  html += "<span class='emoji'>💬</span>lokaler Eigener Text</button>";
  html += "<button onclick='setContent(8)' class='content-btn" + String(currentContent == 8 ? " active" : "") + "'>";
  html += "<span class='emoji'>🐱</span>GIF aus github</button>";
  html += "<button onclick='setContent(9)' class='content-btn" + String(currentContent == 9 ? " active" : "") + "'>";
  html += "<span class='emoji'>🎆</span>Feuerwerk Animation</button>";
  html += "<button onclick='setContent(10)' class='content-btn" + String(currentContent == 10 ? " active" : "") + "'>";
  html += "<span class='emoji'>🧩</span>Tetris Animation</button>";
  html += "<button onclick='setContent(11)' class='content-btn" + String(currentContent == 11 ? " active" : "") + "'>";
  html += "<span class='emoji'>✌️</span>Signal für nette Arbeitskollegen</button>";
  html += "<button onclick='setContent(12)' class='content-btn" + String(currentContent == 12 ? " active" : "") + "'>";
  html += "<span class='emoji'>⚡️</span>Flacker Licht</button>";
  html += "<button onclick='setContent(13)' class='content-btn" + String(currentContent == 13 ? " active" : "") + "'>";
  html += "<span class='emoji'>💼</span>Cotive Logo Farbe lokal</button>";
  html += "<button onclick='setContent(14)' class='content-btn" + String(currentContent == 14 ? " active" : "") + "'>";
  html += "<span class='emoji'>📈</span>Live Aktienkurse</button>";
  html += "<button onclick='setContent(15)' class='content-btn" + String(currentContent == 15 ? " active" : "") + "'>";
  html += "<span class='emoji'>🤪</span>Random Joke</button>";
  html += "</div></div>";
  
  html += "<div class='divider'></div>";
  
  html += "<div class='section'>";
  html += "<div class='brightness-container'>";
  html += "<div class='brightness-label'>💡 Helligkeit</div>";
  html += "<div class='slider-container'>";
  html += "<input type='range' id='brightness' class='brightness-slider' min='1' max='255' value='" + String(currentBrightness) + "'";
  html += " onchange='setBrightness(this.value)' oninput='updateBrightnessDisplay(this.value)'>";
  html += "</div>";
  html += "<div class='brightness-value'>Aktuell: <span id='brightnessValue'>" + String(currentBrightness) + "</span></div>";
  html += "</div></div>";
  
  html += "<div class='divider'></div>";
  
  html += "<div class='section'>";
  html += "<div class='system-section'>";
  html += "<h2 class='section-title'>⚙️ System</h2>";
  html += "<button onclick='resetWiFi()' class='reset-btn'>WiFi Einstellungen zurücksetzen</button>";
  html += "</div></div>";
  
  html += "<div class='auto-refresh'>🔄 Seite aktualisiert sich alle 10 Sekunden automatisch</div>";
  
  html += "<script>";
  html += "function setContent(id) {";
  html += "  const buttons = document.querySelectorAll('.content-btn');";
  html += "  buttons.forEach(btn => btn.classList.remove('active'));";
  html += "  buttons[id].classList.add('active');";
  html += "  fetch('/content?id=' + id).then(response => response.text()).then(data => {";
  html += "    if(data !== 'OK') { location.reload(); }";
  html += "  });";
  html += "}";
  html += "function setBrightness(value) {";
  html += "  fetch('/brightness?value=' + value);";
  html += "}";
  html += "function updateBrightnessDisplay(value) {";
  html += "  document.getElementById('brightnessValue').innerHTML = value;";
  html += "}";
  html += "function resetWiFi() {";
  html += "  if(confirm('WiFi-Einstellungen zurücksetzen? ESP32 startet neu!')) {";
  html += "    fetch('/reset');";
  html += "  }";
  html += "}";
  html += "setTimeout(function() { location.reload(); }, 10000);";
  html += "</script>";
  
  html += "</div>";
  html += "</body></html>";
  return html;
}
#endif