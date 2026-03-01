<p align="center">
  <h1 align="center">🖥️ ESP32 LED Matrix Display</h1>
  <p align="center">
    <strong>Web-controlled RGB LED matrix display with ESP32 for flexible digital signage content</strong>
  </p>
  <p align="center">
    <img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++">
    <img src="https://img.shields.io/badge/Arduino-00878F?style=for-the-badge&logo=arduino&logoColor=white" alt="Arduino">
    <img src="https://img.shields.io/badge/ESP32-Wrover-E7352C?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP32">
    <img src="https://img.shields.io/badge/HUB75-128×64-333333?style=for-the-badge" alt="HUB75">
    <img src="https://img.shields.io/badge/WiFi-WebUI-4285F4?style=for-the-badge&logo=wifi&logoColor=white" alt="WiFi">
  </p>
</p>

---

## About

A modular digital signage system built around an ESP32 Wrover and two daisy-chained 64×64 HUB75 RGB LED panels (128×64 pixels). Content is controlled via a responsive web interface — from news feeds and animations to live weather data. Images and GIFs can be swapped directly through GitHub without reflashing the microcontroller.

<p align="center">
  <table>
    <tr>
      <td><img src="images/IMG_5259.png" width="400"/></td>
      <td><img src="images/clock_display.jpg" alt="Clock Display" width="400"/></td>
    </tr>
    <tr>
      <td><img src="images/weather_display.jpg" alt="Weather Display" width="400"/></td>
      <td><img src="images/web_interface.jpg" alt="Web Interface" width="400"/></td>
    </tr>
  </table>
</p>

## Features

- **16 Content Modules** — switchable via web interface
- **Responsive Control** — works on phone, tablet and desktop
- **Automatic WiFi Setup** — access point with setup page on first boot
- **Modular Architecture** — add new content as a simple `.h` file
- **External Content via GitHub** — swap images (XBM) and GIFs without code changes
- **Brightness Control** — stepless adjustment via web interface (0–255)

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C++ (Arduino Framework) |
| IDE | Arduino IDE |
| Microcontroller | ESP32 Wrover (4 MB Flash + 4 MB PSRAM) |
| Display | 2× 64×64 HUB75 RGB LED Panels (128×64 pixels) |
| Web Server | ESPAsyncWebServer (non-blocking) |
| Data Formats | JSON (ArduinoJson), RSS/XML, XBM, GIF |
| APIs | Open-Meteo, Tagesschau RSS, witzapi.de, Yahoo Finance |

## Available Content

| # | Content | Description | Data Source |
|---|---------|------------|-------------|
| 0 | ℹ️ System Info | IP address, SSID, status | Local |
| 1 | 🕐 Digital Clock | Date & time with seconds | NTP (`pool.ntp.org`) |
| 2 | 📰 News Ticker | Scrolling headlines | [Tagesschau RSS](https://www.tagesschau.de/infoservices/alle-meldungen-100~rss2.xml) |
| 3 | 🌈 Rainbow Waves | Animated water waves with color gradient | Procedural |
| 4 | 🌤️ Weather | Location, temperature, min/max, feels like | [Open-Meteo](https://open-meteo.com/) + IP geolocation |
| 5 | 🖼️ XBM Image | Black & white image from GitHub | [GitHub Raw](https://github.com/alexw611/esp32_projekt) |
| 6 | 🎨 Local Bitmap | RGB565 pixel art stored in code | Local (PROGMEM) |
| 7 | 💬 Custom Text | Scrolling user-defined text | Local |
| 8 | 🐱 GIF Player | Animated GIFs from GitHub | [GitHub Raw](https://github.com/alexw611/esp32_projekt) (PSRAM) |
| 9 | 🎆 Fireworks | Particle-based firework animation | Procedural |
| 10 | 🧩 Tetris | AI-controlled Tetris animation | Procedural |
| 11 | ✌️ Pixel Art Signal | Greeting for friendly coworkers | Local (Bitmap) |
| 12 | ⚡ Strobe Light | Fast color changes & strobe effects | Procedural |
| 13 | 💼 Company Logo (Color) | Local RGB565 logo | Local (PROGMEM) |
| 14 | 📈 Stock Prices | Rotating display of 15 stocks | Yahoo Finance / Demo |
| 15 | 🤪 Jokes | Random German jokes | [witzapi.de](https://witzapi.de) |

## System Overview

```
 ┌──────────────┐
 │   5V / 8A    │
 │   PSU        │
 └──────┬───────┘
        │ DC 5V
 ┌──────┴───────┐
 │   Power      │
 │   Adapter    │
 │   Board      │
 └┬──────┬────┬─┘
  │ 5V   │ 5V │ 3.3V
  │      │    │
  │      │  ┌─┴───────────┐        ┌──────────────┐
  │      │  │  ESP32       │  WiFi  │              │
  │      │  │  Wrover      ◄────────┤  Browser     │
  │      │  │              │  HTTP  │  (Phone/PC)  │
  │      │  └──┬───────────┘        └──────────────┘
  │      │     │ HUB75 OUT
  │      │  ┌──┴──────┐
  │      │  │ 64×64   │
  │      └──► Panel R  │
  │         │ (IN)     │
  │         └──┬──────┘
  │            │ HUB75 OUT → IN
  │         ┌──┴──────┐
  │         │ 64×64   │
  └─────────► Panel L  │
            │ (IN)     │
            └─────────┘
```

> The panels are daisy-chained via HUB75: **ESP32 → Panel R (IN) → Panel R (OUT) → Panel L (IN)**. Power (5V) is supplied in parallel through the Power Adapter Board.

## Hardware

| Component | Description |
|---|---|
| **Microcontroller** | ESP32 Wrover (4 MB Flash + 4 MB PSRAM) |
| **LED Panels** | 2× 64×64 RGB P3 HUB75 → **128×64 pixels** |
| **Power Supply** | 5V / 8A (40W) switching PSU |
| **Power Adapter** | Coolwell RGB Matrix Power Adapter Board |
| **Enclosure** | Custom 3D-printed mount |

### Pin Mapping (ESP32 → HUB75)

| Signal | GPIO | Signal | GPIO |
|--------|------|--------|------|
| R1 | 25 | G1 | 26 |
| B1 | 27 | R2 | 14 |
| G2 | 12 | B2 | 13 |
| A | 23 | B | 22 |
| C | 21 | D | 19 |
| E | 18 | LAT | 5 |
| OE | 4 | CLK | 15 |

## Libraries & Versions

| Library | Version | Purpose |
|---|---|---|
| ESP Core | 2.0.7 | ESP32 base support |
| ESP32 HUB75 MatrixPanel DMA | 2.0.7 | LED panel control via DMA |
| Adafruit GFX Library | 1.10.14 | Graphics primitives (text, shapes) |
| Adafruit BusIO | 1.17.2 | I2C/SPI abstraction |
| AnimatedGIF | 2.2.0 | GIF decoding |
| ArduinoJson | 6.21.3 | JSON parsing (weather, stocks, jokes) |
| AsyncTCP | 1.1.4 | Asynchronous TCP connections |
| ESPAsyncWebServer | 3.1.0 | Non-blocking web server |

> ⚠️ **Important:** These libraries only work reliably together in the exact versions listed above. Other combinations may cause compile errors or instability.

## WiFi Setup Flow

```
ESP32                                    User
  │                                        │
  ├── Starts Access Point ────────────────►│
  │   SSID: "LED_Matrix_Setup"             │
  │                                        ├── Connects to AP
  │                                        ├── Opens 192.168.4.1
  │◄── Setup Page (HTML) ─────────────────┤
  │                                        │
  │◄── POST /save { ssid, password } ─────┤
  │                                        │
  ├── Saves credentials (Preferences)      │
  ├── Reboot                               │
  ├── Connects to WiFi ──────────────────►│
  │                                        │
  ├── Shows IP address on display          │
  │                                        ├── Opens IP in browser
  │◄── GET / ─────────────────────────────┤
  ├── Control interface (HTML) ───────────►│
  │                                        │
  │◄── GET /content?id=3 ─────────────────┤  (switch content)
  │◄── GET /brightness?value=200 ─────────┤  (adjust brightness)
```

## Getting Started

### Prerequisites

- Arduino IDE
- ESP32 Wrover board
- 2× 64×64 HUB75 LED panels + Power Adapter Board + 5V/8A PSU

### Installation

```bash
# Clone the repository
git clone https://github.com/alexw611/esp32-led-matrix.git
cd esp32-led-matrix
```

1. Open `v1.ino` in Arduino IDE
2. Select board: **ESP32 Wrover Kit (all versions)**
3. Install all libraries in the exact versions listed above
4. Upload to ESP32

### First-Time Setup

1. Power on the ESP32 → display shows setup instructions
2. Connect to WiFi network **`LED_Matrix_Setup`**
3. Open **`192.168.4.1`** in your browser → enter SSID & password
4. ESP32 reboots, connects, and displays its IP address
5. Open the IP in your browser → control interface

## Project Structure

```
esp32-led-matrix/
├── v1.ino              # Main: WiFi, web server, content routing
├── setup_page.h        # HTML/CSS WiFi setup page (access point)
├── control.h           # HTML/CSS/JS control interface
├── content1.h          # 🕐 NTP digital clock
├── content2.h          # 📰 Tagesschau RSS feed ticker
├── content3.h          # 🌈 Rainbow water waves
├── content4.h          # 🌤️ Weather (Open-Meteo + IP geolocation)
├── content5.h          # 🖼️ XBM image from GitHub
├── content6.h          # 🎨 Local RGB565 bitmap (PROGMEM)
├── content7.h          # 💬 Custom scrolling text
├── content8.h          # 🐱 GIF player from GitHub (PSRAM)
├── content9.h          # 🎆 Firework particle animation
├── content10.h         # 🧩 AI Tetris animation
├── content11.h         # ✌️ Pixel art signal
├── content12.h         # ⚡ Strobe light effects
├── content13.h         # 💼 Company logo (RGB565, local)
├── content14.h         # 📈 Stock prices (Yahoo Finance / demo)
├── content15.h         # 🤪 Jokes from witzapi.de
├── images/             # Photos
├── LICENSE             # MIT License
└── README.md
```

## Adding Custom Content

The system is modular. New content in 4 steps:

**1. Create a header file**

```cpp
// content_new.h
#ifndef CONTENT_NEW_H
#define CONTENT_NEW_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

void showContentNew(MatrixPanel_I2S_DMA* display) {
    // display->drawPixel(x, y, color);
    // display->print("Text");
}
#endif
```

**2. Include in `v1.ino`**
```cpp
#include "content_new.h"
```

**3. Register in `switch(currentContent)`**
```cpp
case 16:
    showContentNew(dma_display);
    break;
```

**4. Add button in `control.h`**
```cpp
html += "<button onclick='setContent(16)' class='content-btn'>";
html += "<span class='emoji'>🆕</span>My New Content</button>";
```

## Swapping External Content (No Reflash)

Some content modules load data directly from GitHub:

| Content | File | Format | Max Size |
|---|---|---|---|
| XBM Image | `cotive.xbm` | XBM (128×64, B&W) | ~1 KB |
| GIF | `gif4.gif` | GIF (128×64) | ~80 KB |

Simply replace the file in the [GitHub repository](https://github.com/alexw611/esp32_projekt) — the new content will be displayed on the next load.

## Known Limitations

- **PSRAM & GIF** — The AnimatedGIF library only uses DRAM internally. GIFs are therefore limited to ~80 KB despite 4 MB of PSRAM being available.
- **RSS Feed** — The Tagesschau feed is occasionally unavailable server-side.
- **Stock Prices** — Currently using demo data in part, as the Yahoo Finance API doesn't always respond reliably.
- **Library Versions** — Only the combination listed above has been tested and is stable.

## Planned Features

- [ ] Interactive games with custom control interface (e.g. Snake)
- [ ] Live sports scores
- [ ] Public transit departure boards
- [ ] Traffic information
- [ ] Reliable stock & crypto API integration
- [ ] Adapt GIF library for PSRAM operation
- [ ] Migrate all content modules to PSRAM
- [ ] QR code on display for quick access to web interface

## License

This project is licensed under the [MIT License](LICENSE).

---
