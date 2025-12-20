# ESP32 Mini OS - App-Based System

A complete handheld operating system for ESP32 with OLED display, featuring 12 apps, QWERTY keyboard, WiFi management, and persistent storage.

## Hardware

- **MCU**: ESP32 WROOM32 DevKit (CP2102)
- **Display**: SH1106 128x64 I2C OLED
- **Buttons**: 8 push buttons (D-pad + A/B/C/D)
- **Sensors**: DHT11 Temperature/Humidity
- **Audio**: Passive Buzzer
- **Power**: TP4056 + LiPo battery

---

## Wiring

### Display (SH1106 I2C)
| Display | ESP32 |
|---------|-------|
| VCC     | 3.3V  |
| GND     | GND   |
| SDA     | GPIO 21 |
| SCK     | GPIO 22 |

### Buttons (Active LOW - GPIO to GND)
| Button | GPIO |
|--------|------|
| LEFT   | 32   |
| RIGHT  | 33   |
| UP     | 25   |
| DOWN   | 26   |
| A      | 27   |
| B      | 14   |
| C      | 13   |
| D      | 4    |

### Other Components
| Component | GPIO |
|-----------|------|
| Buzzer (+) | 15  |
| DHT11 Data | 17  |

---

## Apps (12 Total)

### Utility Apps
| App | Description | API |
|-----|-------------|-----|
| Weather | Temperature, humidity, conditions | OpenWeatherMap (key required) |
| Crypto | BTC, ETH, SOL prices | CoinGecko (free) |
| News | Top headlines | NewsAPI (key required) |
| Settings | WiFi, API keys, sound | - |
| System | WiFi status, DHT11, button test | - |

### Games
| App | Description |
|-----|-------------|
| Snake | Classic snake game with high scores |
| Pong | Single player vs AI |

### Fun API Apps (All Free, No Keys)
| App | Description | API |
|-----|-------------|-----|
| Facts | Random fun facts | uselessfacts.jsph.pl |
| Jokes | Programming & general jokes | JokeAPI |
| Quotes | Inspirational quotes | quotable.io |
| ISS | Space station tracker | open-notify.org |
| Trivia | Multiple choice quiz | OpenTDB |

---

## Button Controls

| Button | Function |
|--------|----------|
| D-pad  | Navigate menus/games |
| A      | Select / Confirm |
| B      | Back / Cancel |
| C      | Secondary action (refresh, toggle) |
| D      | Home / Exit to launcher |

### Keyboard Controls
| Button | Function |
|--------|----------|
| D-pad  | Move cursor |
| A      | Type character / Select action |
| B      | Backspace |
| C      | Toggle CAPS / Symbols |
| D      | Cancel input |

---

## Project Structure

```
esp/
├── platformio.ini
├── PROJECT.md
├── include/
│   ├── config.h
│   ├── app.h
│   ├── ui.h
│   ├── input.h
│   ├── keyboard.h
│   ├── wifi_manager.h
│   ├── icons.h
│   └── apps/
│       ├── launcher.h
│       ├── weather.h
│       ├── crypto.h
│       ├── news.h
│       ├── settings.h
│       ├── sysinfo.h
│       ├── snake.h
│       ├── pong.h
│       ├── facts.h
│       ├── jokes.h
│       ├── quotes.h
│       ├── iss.h
│       └── trivia.h
└── src/
    ├── main.cpp
    ├── ui.cpp
    ├── input.cpp
    ├── keyboard.cpp
    ├── wifi_manager.cpp
    └── apps/
        ├── launcher.cpp
        ├── weather.cpp
        ├── crypto.cpp
        ├── news.cpp
        ├── settings.cpp
        ├── sysinfo.cpp
        ├── snake.cpp
        ├── pong.cpp
        ├── facts.cpp
        ├── jokes.cpp
        ├── quotes.cpp
        ├── iss.cpp
        └── trivia.cpp
```

---

## Build & Upload

```bash
cd /Users/nikzart/Developer/esp

# Build
~/.platformio/penv/bin/pio run

# Upload
~/.platformio/penv/bin/pio run -t upload

# Monitor
~/.platformio/penv/bin/pio device monitor
```

---

## First-Time Setup

1. **Build and upload** the firmware
2. **Boot** the device - it will show "ESP32 OS Loading..."
3. **Go to Settings** (select with D-pad, press A)
4. **Configure WiFi**:
   - Select "WiFi"
   - Press C to scan
   - Select your network
   - Enter password with keyboard
   - Credentials are saved automatically
5. **Add API Keys** (for Weather & News):
   - Select "API Keys"
   - Enter your keys using keyboard

---

## API Keys

### Required for some apps:
1. **OpenWeatherMap** (Weather app)
   - https://openweathermap.org/api
   - Free tier: 1000 calls/day

2. **NewsAPI** (News app)
   - https://newsapi.org
   - Free tier: 100 requests/day

### Not required (free public APIs):
- CoinGecko (Crypto)
- uselessfacts.jsph.pl (Facts)
- JokeAPI (Jokes)
- quotable.io (Quotes)
- open-notify.org (ISS)
- OpenTDB (Trivia)

---

## NVS Storage

The following data is persisted in ESP32 flash:
- WiFi credentials (up to 3 networks)
- API keys (Weather, News)
- Snake high score
- City name for weather

---

## Libraries Used

```ini
lib_deps =
    olikraus/U8g2@^2.35.9
    adafruit/DHT sensor library@^1.4.6
    adafruit/Adafruit Unified Sensor@^1.1.14
    bblanchon/ArduinoJson@^6.21.3
```

---

## Troubleshooting

### Display garbled
- Ensure you have SH1106 (not SSD1306)
- Check I2C wiring: SDA→21, SCK→22

### WiFi won't connect
- Check password (case sensitive)
- Ensure network is 2.4GHz (ESP32 doesn't support 5GHz)

### API apps show errors
- Check WiFi is connected (indicator in launcher)
- Verify API keys are entered in Settings
- Some APIs have rate limits

### Buttons not working
- Buttons should connect GPIO to GND
- Internal pull-ups are enabled in code

### DHT11 shows "--"
- Check wiring: Data→GPIO17, VCC→3.3V
- Some modules need 10K pull-up on data line
