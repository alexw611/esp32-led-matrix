// setup_page.h
#ifndef SETUP_PAGE_H
#define SETUP_PAGE_H
String getSetupPage() {
  String html = "<!DOCTYPE html><html><head><title>LED Matrix WiFi Setup</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta charset=\"UTF-8\">";
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }";
  html += ".container { max-width: 480px; margin: 0 auto; background: rgba(255,255,255,0.95); backdrop-filter: blur(20px); border-radius: 20px; padding: 40px 30px; box-shadow: 0 20px 40px rgba(0,0,0,0.1); }";
  html += "h1 { color: #1d1d1f; font-size: 28px; font-weight: 600; text-align: center; margin-bottom: 10px; }";
  html += ".subtitle { color: #86868b; font-size: 16px; text-align: center; margin-bottom: 40px; }";
  html += ".form-group { margin-bottom: 24px; }";
  html += "label { display: block; color: #1d1d1f; font-size: 16px; font-weight: 500; margin-bottom: 8px; }";
  html += "input[type='text'], input[type='password'] { width: 100%; padding: 16px; border: 2px solid #f5f5f7; border-radius: 12px; font-size: 16px; background: #fbfbfd; transition: all 0.3s ease; }";
  html += "input[type='text']:focus, input[type='password']:focus { outline: none; border-color: #007aff; background: white; box-shadow: 0 0 0 4px rgba(0,122,255,0.1); }";
  html += ".submit-btn { width: 100%; padding: 16px; background: linear-gradient(135deg, #007aff 0%, #0051d5 100%); color: white; border: none; border-radius: 12px; font-size: 16px; font-weight: 600; cursor: pointer; transition: all 0.3s ease; margin-top: 8px; }";
  html += ".submit-btn:hover { transform: translateY(-2px); box-shadow: 0 10px 25px rgba(0,122,255,0.3); }";
  html += ".submit-btn:active { transform: translateY(0); }";
  html += ".divider { height: 1px; background: linear-gradient(90deg, transparent, #d2d2d7, transparent); margin: 32px 0; }";
  html += ".instructions { background: #f5f5f7; border-radius: 16px; padding: 24px; }";
  html += ".instructions h3 { color: #1d1d1f; font-size: 18px; font-weight: 600; margin-bottom: 16px; }";
  html += ".step { display: flex; align-items: flex-start; margin-bottom: 12px; color: #424245; font-size: 15px; line-height: 1.4; }";
  html += ".step-number { background: #007aff; color: white; border-radius: 50%; width: 24px; height: 24px; display: flex; align-items: center; justify-content: center; font-size: 12px; font-weight: 600; margin-right: 12px; flex-shrink: 0; }";
  html += ".note { background: #fff3cd; border: 1px solid #ffeaa7; border-radius: 12px; padding: 16px; margin-top: 24px; color: #856404; font-size: 14px; line-height: 1.4; }";
  html += "@media (max-width: 480px) { .container { margin: 10px; padding: 30px 20px; } h1 { font-size: 24px; } }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>LED Matrix Setup</h1>";
  html += "<p class='subtitle'>Verbinden Sie Ihre LED Matrix mit dem WiFi</p>";
  html += "<form action='/save' method='POST'>";
  html += "<div class='form-group'>";
  html += "<label for='ssid'>WiFi Netzwerk (SSID)</label>";
  html += "<input type='text' id='ssid' name='ssid' required placeholder='Ihr WiFi Netzwerkname'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='password'>WiFi Passwort</label>";
  html += "<input type='password' id='password' name='password' required placeholder='Ihr WiFi Passwort'>";
  html += "</div>";
  html += "<button type='submit' class='submit-btn'>Speichern und Verbinden</button>";
  html += "</form>";
  html += "<div class='divider'></div>";
  html += "<div class='instructions'>";
  html += "<h3>Setup Anleitung</h3>";
  html += "<div class='step'><div class='step-number'>1</div><div>SSID (Netzwerkname) Ihres WiFi-Netzwerks eingeben</div></div>";
  html += "<div class='step'><div class='step-number'>2</div><div>WiFi-Passwort eingeben</div></div>";
  html += "<div class='step'><div class='step-number'>3</div><div>Auf \"Speichern und Verbinden\" klicken</div></div>";
  html += "<div class='step'><div class='step-number'>4</div><div>ESP32 startet neu und verbindet sich</div></div>";
  html += "<div class='step'><div class='step-number'>5</div><div>Auf LED Matrix wird IP-Adresse angezeigt</div></div>";
  html += "</div>";
  html += "<div class='note'>💡 <strong>Hinweis:</strong> Bei Verbindungsproblemen startet der ESP32 automatisch wieder in den Setup-Modus.</div>";
  html += "</div>";
  html += "</body></html>";
  return html;
}
#endif