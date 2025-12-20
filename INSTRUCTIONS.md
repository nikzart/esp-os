# ESP-OS Complete Instructions

A comprehensive guide to building, setting up, and using ESP-OS - a handheld operating system for ESP32 with OLED display.

---

## Table of Contents

1. [Hardware Requirements](#1-hardware-requirements)
2. [Hardware Assembly](#2-hardware-assembly)
3. [Software Setup](#3-software-setup)
4. [Configuration](#4-configuration)
5. [Building & Uploading](#5-building--uploading)
6. [Using the Device](#6-using-the-device)
7. [App Guide](#7-app-guide)
8. [OTA Updates](#8-ota-updates)
9. [Customization](#9-customization)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. Hardware Requirements

### 1.1 Components List

| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| ESP32 WROOM32 | DevKit v1 (30-pin or 38-pin) | 1 | Any ESP32 dev board works |
| OLED Display | SH1106 128x64 I2C | 1 | 4-pin version (GND, VCC, SCL, SDA) |
| Push Buttons | 6x6mm Tactile | 8 | Momentary, normally open |
| Buzzer | Active Buzzer 5V | 1 | 3.3V compatible, or use passive with resistor |
| Resistors | 10kΩ | 8 | Optional - ESP32 has internal pull-ups |
| Breadboard | Full-size or half-size | 1 | For prototyping |
| Jumper Wires | Male-to-male | ~30 | Various colors recommended |
| USB Cable | Micro-USB or USB-C | 1 | Depends on your ESP32 board |

### 1.2 Optional Components

| Component | Purpose |
|-----------|---------|
| TP4056 Module | LiPo battery charging |
| 3.7V LiPo Battery | Portable power (1000mAh+ recommended) |
| Slide Switch | Power on/off |
| 3D Printed Case | Enclosure |
| Perfboard | Permanent assembly |

### 1.3 Tools Needed

- Soldering iron (for permanent assembly)
- Wire strippers
- Multimeter (for debugging)
- Computer with USB port

---

## 2. Hardware Assembly

### 2.1 Pin Mapping

#### Display (I2C)

| Display Pin | ESP32 Pin | Wire Color (suggested) |
|-------------|-----------|------------------------|
| GND | GND | Black |
| VCC | 3.3V | Red |
| SCL | GPIO 22 | Yellow |
| SDA | GPIO 21 | Blue |

#### D-Pad Buttons

| Button | ESP32 Pin | Function |
|--------|-----------|----------|
| UP | GPIO 25 | Navigate up / Scroll up |
| DOWN | GPIO 26 | Navigate down / Scroll down |
| LEFT | GPIO 32 | Navigate left / Previous |
| RIGHT | GPIO 33 | Navigate right / Next |

#### Action Buttons

| Button | ESP32 Pin | Function |
|--------|-----------|----------|
| A | GPIO 27 | Select / Confirm / Action |
| B | GPIO 14 | Back / Cancel / Exit |
| C | GPIO 13 | Secondary action / Toggle |
| D | GPIO 4 | Menu / Options |

#### Buzzer

| Buzzer Pin | ESP32 Pin |
|------------|-----------|
| + (Positive) | GPIO 15 |
| - (Negative) | GND |

### 2.2 Wiring Instructions

#### Step 1: Connect the Display

1. Place the OLED display on your breadboard
2. Connect **GND** on display to any **GND** pin on ESP32
3. Connect **VCC** on display to **3.3V** on ESP32 (NOT 5V!)
4. Connect **SCL** on display to **GPIO 22** on ESP32
5. Connect **SDA** on display to **GPIO 21** on ESP32

> **Warning**: The SH1106 display typically runs on 3.3V. While some modules have onboard regulators for 5V, check your module's specifications first.

#### Step 2: Connect the Buttons

Each button should be wired between the GPIO pin and GND. The ESP32's internal pull-up resistors will be enabled in software.

```
GPIO Pin ----[BUTTON]---- GND
```

Wire each button according to the pin mapping above:

1. **D-Pad buttons**: Wire UP to GPIO25, DOWN to GPIO26, LEFT to GPIO32, RIGHT to GPIO33
2. **Action buttons**: Wire A to GPIO27, B to GPIO14, C to GPIO13, D to GPIO4

> **Tip**: Use different colored wires for each button group to make debugging easier.

#### Step 3: Connect the Buzzer

1. Connect the **positive** leg (longer leg or marked with +) to **GPIO 15**
2. Connect the **negative** leg to **GND**

> **Note**: If using a passive buzzer, you may need a current-limiting resistor (220Ω-1kΩ).

### 2.3 Power Options

#### Option A: USB Power (Development)

Simply connect the ESP32 to your computer via USB. The onboard regulator provides stable 3.3V.

#### Option B: Battery Power (Portable)

Using a TP4056 charging module with a LiPo battery:

```
LiPo Battery (+) -----> B+ on TP4056
LiPo Battery (-) -----> B- on TP4056
TP4056 OUT+ -----> VIN (or 5V) on ESP32
TP4056 OUT- -----> GND on ESP32
USB Power -----> TP4056 USB port (for charging)
```

> **Safety Warning**: LiPo batteries can be dangerous if mishandled. Never short-circuit, puncture, or overcharge them.

### 2.4 Wiring Diagram (ASCII)

```
                    ESP32 WROOM32
                   +--------------+
                   |              |
    [UP Button]----| GPIO25       |
  [DOWN Button]----| GPIO26       |
  [LEFT Button]----| GPIO32       |
 [RIGHT Button]----| GPIO33       |
                   |              |
     [A Button]----| GPIO27       |
     [B Button]----| GPIO14       |
     [C Button]----| GPIO13       |
     [D Button]----| GPIO4        |
                   |              |
   [OLED SCL]------| GPIO22       |
   [OLED SDA]------| GPIO21       |
                   |              |
   [Buzzer +]------| GPIO15       |
                   |              |
                   | 3.3V |----------- [OLED VCC]
                   | GND  |-+--------- [OLED GND]
                   |      | +--------- [All Button GNDs]
                   |      | +--------- [Buzzer -]
                   +--------------+
```

---

## 3. Software Setup

### 3.1 Install Visual Studio Code

1. Go to https://code.visualstudio.com/
2. Download the installer for your operating system
3. Run the installer and follow the prompts
4. Launch VS Code after installation

### 3.2 Install PlatformIO Extension

1. Open VS Code
2. Click the **Extensions** icon in the left sidebar (or press `Ctrl+Shift+X`)
3. Search for "**PlatformIO IDE**"
4. Click **Install** on the PlatformIO IDE extension
5. Wait for installation to complete (this may take a few minutes)
6. Restart VS Code when prompted

### 3.3 Clone the Repository

#### Using Git (Recommended)

Open a terminal and run:

```bash
cd ~/Developer  # or your preferred directory
git clone https://github.com/nikzart/esp-os.git
cd esp-os
```

#### Manual Download

1. Go to https://github.com/nikzart/esp-os
2. Click the green "**Code**" button
3. Select "**Download ZIP**"
4. Extract the ZIP file to your preferred location

### 3.4 Open Project in VS Code

1. Open VS Code
2. Click **File** → **Open Folder**
3. Navigate to the `esp-os` folder
4. Click **Open**
5. PlatformIO will automatically detect the project and install dependencies

> **Note**: First-time setup may take several minutes as PlatformIO downloads the ESP32 toolchain and libraries.

---

## 4. Configuration

### 4.1 API Keys Setup

The Weather and News apps require API keys. Edit `include/config.h`:

#### OpenWeatherMap API Key (for Weather app)

1. Go to https://openweathermap.org/api
2. Sign up for a free account
3. Go to "My API Keys" in your account
4. Copy your API key
5. Edit `include/config.h` and replace the key:

```c
#define OPENWEATHER_API_KEY "your_api_key_here"
```

#### NewsAPI Key (for News app)

1. Go to https://newsapi.org/
2. Sign up for a free account
3. Copy your API key from the dashboard
4. Edit `include/config.h` and replace the key:

```c
#define NEWS_API_KEY "your_api_key_here"
```

#### Default City

Set your city for the Weather app:

```c
#define DEFAULT_CITY "YourCity"
```

### 4.2 Pin Configuration

If you need to use different GPIO pins, edit `include/config.h`:

```c
// Button GPIO Pin Definitions
#define BTN_PIN_LEFT  32
#define BTN_PIN_RIGHT 33
#define BTN_PIN_UP    25
#define BTN_PIN_DOWN  26
#define BTN_PIN_A     27
#define BTN_PIN_B     14
#define BTN_PIN_C     13
#define BTN_PIN_D     4

// I2C Pins
#define I2C_SDA 21
#define I2C_SCL 22

// Buzzer
#define BUZZER_PIN 15
```

### 4.3 Display Configuration

If using a different display address:

```c
#define SCREEN_ADDRESS 0x3C  // Common addresses: 0x3C or 0x3D
```

---

## 5. Building & Uploading

### 5.1 Connect Your ESP32

1. Connect your ESP32 to your computer using a USB cable
2. The computer should recognize it as a serial port
3. If not detected, you may need to install USB drivers:
   - **CP2102**: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
   - **CH340**: http://www.wch-ic.com/downloads/CH341SER_ZIP.html

### 5.2 Build the Firmware

#### Using VS Code/PlatformIO GUI

1. Click the **PlatformIO** icon in the left sidebar (alien head icon)
2. Under "PROJECT TASKS" → "esp32dev", click **Build**
3. Wait for the build to complete
4. Check the terminal for "SUCCESS" message

#### Using Command Line

```bash
# Navigate to project directory
cd ~/Developer/esp-os

# Build the project
~/.platformio/penv/bin/pio run
```

### 5.3 Upload to ESP32

#### Using VS Code/PlatformIO GUI

1. Click the **PlatformIO** icon
2. Under "PROJECT TASKS" → "esp32dev", click **Upload**
3. Wait for upload to complete

#### Using Command Line

```bash
~/.platformio/penv/bin/pio run -t upload
```

### 5.4 Monitor Serial Output

To see debug messages and logs:

#### Using VS Code/PlatformIO GUI

1. Click **Upload and Monitor** under PROJECT TASKS

#### Using Command Line

```bash
~/.platformio/penv/bin/pio device monitor
```

Press `Ctrl+C` to exit the monitor.

### 5.5 Common Build Issues

| Error | Solution |
|-------|----------|
| "No such file or directory" | Run `pio run` to install dependencies |
| "Port not found" | Check USB connection, install drivers |
| "Permission denied" (Linux) | Add user to dialout group: `sudo usermod -a -G dialout $USER` |
| "Upload failed" | Hold BOOT button during upload |

---

## 6. Using the Device

### 6.1 First Boot

1. After successful upload, the device will restart automatically
2. You'll see "ESP32 OS Loading..." on the display
3. A startup beep confirms the buzzer is working
4. The **Homescreen** will appear showing time, date, and weather

### 6.2 Homescreen

The homescreen is the main screen shown when the device starts.

**Displays:**
- Current time (synced via NTP when WiFi connected)
- Current date
- WiFi connection status
- Weather for saved city (temperature and condition)

**Controls:**
- **Press any button**: Open the app launcher

**Auto-refresh:**
- Time updates every second
- Weather updates every 10 minutes

> **Note**: Time and weather require WiFi connection. Without WiFi, time shows "00:00" and weather shows "No WiFi".

### 6.4 Button Controls

| Button | Global Function | In Apps |
|--------|-----------------|---------|
| **D-Pad UP** | Move selection up | Scroll up / Navigate |
| **D-Pad DOWN** | Move selection down | Scroll down / Navigate |
| **D-Pad LEFT** | Move selection left | Previous item |
| **D-Pad RIGHT** | Move selection right | Next item |
| **A** | Select / Confirm | Primary action |
| **B** | Back / Exit | Cancel / Go back |
| **C** | Secondary action | Varies by app |
| **D** | Menu / Options | Varies by app |

### 6.5 Launcher Navigation

The launcher displays apps in a 4x4 grid:

```
+---+---+---+---+
| ☀ | ₿ | 📰 | 🐍 |  Row 1: Weather, Crypto, News, Snake
+---+---+---+---+
| 🏓 | 💡 | 😂 | 💬 |  Row 2: Pong, Facts, Jokes, Quotes
+---+---+---+---+
| 🛰 | ❓ | ⚙ | 📊 |  Row 3: ISS, Trivia, Settings, System
+---+---+---+---+
| ⬇ |   |   |   |  Row 4: OTA Update
+---+---+---+---+
```

- Use **D-Pad** to move the selection cursor
- Press **A** to open the selected app
- Press **B** to return to homescreen
- The status bar shows WiFi status and current time

### 6.6 Connecting to WiFi

1. Navigate to **Settings** app
2. Select **WiFi**
3. Wait for network scan to complete
4. Select your network from the list
5. Enter password using the on-screen keyboard:
   - **D-Pad**: Move cursor on keyboard
   - **A**: Select character
   - **B**: Backspace
   - **C**: Confirm/Submit
   - **D**: Cancel
6. Wait for connection (status shown on screen)

---

## 7. App Guide

### 7.1 Weather App ☀

Shows current weather for any city with search and memory.

**Features:**
- Current temperature (Celsius)
- Weather condition (Clear, Cloudy, Rain, etc.)
- Humidity percentage
- Search for any city
- Remembers last searched city (persists across reboots)
- Default city: Kollam

**Controls:**
- **A**: Fetch/refresh weather data
- **B**: Exit to launcher
- **C**: Change city (opens keyboard)

**How to Change City:**
1. Press **C** to open the keyboard
2. Current city is pre-filled for editing
3. Type new city name using on-screen keyboard
4. Press **C** on keyboard to confirm
5. Weather automatically fetches for new city
6. City is saved and remembered for next time

**Requirements:**
- WiFi connection
- Valid OpenWeatherMap API key

---

### 7.2 Crypto App ₿

Displays real-time cryptocurrency prices.

**Features:**
- Bitcoin (BTC) price in USD
- Ethereum (ETH) price in USD
- 24-hour price change percentage

**Controls:**
- **A**: Refresh prices
- **B**: Exit to launcher

**Requirements:**
- WiFi connection
- Uses free CoinGecko API (no key needed)

---

### 7.3 News App 📰

Shows latest news headlines.

**Features:**
- Top headlines from NewsAPI
- Scrollable list of articles
- Article titles displayed

**Controls:**
- **UP/DOWN**: Scroll through headlines
- **A**: (Future: Read full article)
- **B**: Exit to launcher

**Requirements:**
- WiFi connection
- Valid NewsAPI key

---

### 7.4 Snake Game 🐍

Classic snake game.

**Features:**
- Growing snake gameplay
- Score tracking
- Increasing difficulty

**Controls:**
- **D-Pad**: Control snake direction
- **A**: Start game / Restart after game over
- **B**: Exit to launcher

---

### 7.5 Pong Game 🏓

Single-player Pong against AI.

**Features:**
- Paddle vs AI gameplay
- Score tracking
- Ball speed increases over time

**Controls:**
- **UP/DOWN**: Move paddle
- **A**: Start game / Restart
- **B**: Exit to launcher

---

### 7.6 Facts App 💡

Displays random interesting facts.

**Features:**
- Random facts from API
- New fact on each request

**Controls:**
- **A**: Get new fact
- **B**: Exit to launcher

**Requirements:**
- WiFi connection

---

### 7.7 Jokes App 😂

Shows random jokes.

**Features:**
- Programming jokes
- General jokes
- Setup + Punchline format

**Controls:**
- **A**: Get new joke
- **B**: Exit to launcher

**Requirements:**
- WiFi connection

---

### 7.8 Quotes App 💬

Inspirational quotes.

**Features:**
- Random quotes from famous people
- Author attribution

**Controls:**
- **A**: Get new quote
- **B**: Exit to launcher

**Requirements:**
- WiFi connection

---

### 7.9 ISS Tracker 🛰

Track the International Space Station.

**Features:**
- Current ISS latitude/longitude
- Number of people in space
- Real-time position updates

**Controls:**
- **A**: Refresh position
- **B**: Exit to launcher

**Requirements:**
- WiFi connection

---

### 7.10 Trivia App ❓

Quiz game with multiple choice questions.

**Features:**
- Random trivia questions
- Multiple choice answers
- Score tracking

**Controls:**
- **UP/DOWN**: Select answer
- **A**: Confirm answer
- **B**: Exit to launcher

**Requirements:**
- WiFi connection

---

### 7.11 Settings App ⚙

Configure device settings.

**Menu Options:**
1. **WiFi** - Connect to wireless networks
2. **API Keys** - Configure Weather/News API keys
3. **Display** - Brightness, contrast settings
4. **Sound** - Enable/disable buzzer
5. **About** - Device information
6. **Restart** - Reboot the device

**Controls:**
- **UP/DOWN**: Navigate menu
- **A**: Select option
- **B**: Exit to launcher

---

### 7.12 System Info App 📊

System diagnostics and button tester.

**Pages:**
1. **Info Page** (default):
   - WiFi status and IP address
   - Signal strength (RSSI)
   - Free heap memory
   - Uptime

2. **Button Test Page**:
   - Visual button state display
   - All 8 buttons shown
   - Filled = pressed, Empty = released

**Controls:**
- **C**: Toggle between Info and Button Test
- **B**: Exit to launcher

---

### 7.13 OTA Update App ⬇

Wirelessly update firmware.

**How to Use:**
1. Connect to WiFi first (via Settings)
2. Open OTA Update app
3. Note the IP address shown on screen
4. On your computer/phone, open a web browser
5. Go to `http://<IP-ADDRESS>` (e.g., http://192.168.1.100)
6. Click "Choose File" and select your `.bin` firmware file
7. Click "Upload & Update"
8. Wait for upload to complete
9. Device will automatically reboot with new firmware

**Finding the Firmware File:**
After building, the firmware is located at:
```
.pio/build/esp32dev/firmware.bin
```

**Controls:**
- **B**: Exit (only when not uploading)

**Requirements:**
- WiFi connection
- Web browser on same network

---

## 8. OTA Updates

### 8.1 Why Use OTA?

- Update firmware without USB cable
- Update remotely over WiFi
- Faster iteration during development
- Update deployed devices

### 8.2 Step-by-Step OTA Update

1. **Build new firmware:**
   ```bash
   ~/.platformio/penv/bin/pio run
   ```

2. **Locate firmware file:**
   ```
   .pio/build/esp32dev/firmware.bin
   ```

3. **Connect device to WiFi:**
   - Open Settings → WiFi
   - Connect to your network

4. **Open OTA Update app:**
   - Navigate to OTA Update in launcher
   - Press A to open

5. **Note the IP address:**
   - Screen shows: `http://192.168.x.x`

6. **Open browser on computer/phone:**
   - Must be on same WiFi network
   - Go to the IP address shown

7. **Upload firmware:**
   - Click "Choose File"
   - Select `firmware.bin`
   - Click "Upload & Update"

8. **Wait for completion:**
   - Progress bar shows upload status
   - Device shows "Update successful!"
   - Automatic reboot occurs

### 8.3 OTA Troubleshooting

| Problem | Solution |
|---------|----------|
| Can't connect to IP | Ensure same WiFi network |
| Upload fails | Check file is .bin format |
| Device not responding | Wait 30 seconds, then power cycle |
| "Update failed" message | Verify firmware is valid ESP32 binary |

---

## 9. Customization

### 9.1 Adding a New App

#### Step 1: Create Header File

Create `include/apps/myapp.h`:

```cpp
#ifndef MYAPP_H
#define MYAPP_H

#include "app.h"

class MyApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "My App"; }
    const uint8_t* getIcon() override;

private:
    // Your app's state variables
};

#endif
```

#### Step 2: Create Source File

Create `src/apps/myapp.cpp`:

```cpp
#include "apps/myapp.h"
#include "ui.h"
#include "config.h"
#include "icons.h"

void MyApp::init() {
    // Initialize app state
}

void MyApp::update() {
    // Called every frame - update logic here
}

void MyApp::render() {
    UI::clear();
    UI::drawTitleBar("My App");
    
    // Draw your app content
    u8g2.drawStr(10, 30, "Hello World!");
    
    UI::drawStatusBar("A:Action", "B:Back");
    UI::flush();
}

void MyApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;
    
    if (btn == BTN_B) {
        wantsToExit = true;
    }
    // Handle other buttons
}

const uint8_t* MyApp::getIcon() {
    return icon_facts;  // Use existing icon or create new one
}
```

#### Step 3: Register App in main.cpp

```cpp
// Add include
#include "apps/myapp.h"

// Add instance
MyApp myApp;

// Add to apps array
App* apps[] = {
    // ... existing apps ...
    &myApp
};
```

#### Step 4: Update App Count

In `include/config.h`, increment `NUM_APPS`.

### 9.2 Creating Custom Icons

Icons are 16x16 pixel XBM format. Use an online XBM editor or create manually:

```cpp
// Example: Simple smiley face
static const uint8_t icon_myicon[] PROGMEM = {
    0xE0, 0x07, 0xF8, 0x1F, 0x1C, 0x38, 0x06, 0x60,
    0x62, 0x46, 0xF2, 0x4F, 0x02, 0x40, 0x02, 0x40,
    0x02, 0x40, 0x06, 0x60, 0x8C, 0x31, 0xF8, 0x1F,
    0xF0, 0x0F, 0xF8, 0x1F, 0xE0, 0x07, 0x00, 0x00
};
```

Add to `include/icons.h`.

### 9.3 Changing Button Pins

Edit `include/config.h`:

```cpp
#define BTN_PIN_UP    25  // Change to your GPIO
#define BTN_PIN_DOWN  26
// ... etc
```

### 9.4 Using Different Display

For **SSD1306** instead of SH1106, edit `src/ui.cpp`:

```cpp
// Change this line:
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// To:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
```

---

## 10. Troubleshooting

### 10.1 Display Issues

#### Display shows nothing
1. Check wiring (VCC, GND, SCL, SDA)
2. Verify display address (0x3C or 0x3D)
3. Ensure using 3.3V not 5V (unless module has regulator)
4. Try swapping SCL and SDA

#### Display shows garbled output
1. Wrong display driver - SH1106 vs SSD1306
2. Edit `src/ui.cpp` to match your display chip
3. Check I2C connection quality

#### Display is dim
1. Check VCC voltage (should be 3.3V)
2. Some displays have adjustable contrast in software

### 10.2 WiFi Issues

#### Won't connect to WiFi
1. Check password is correct
2. Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
3. Router may be blocking new devices
4. Try moving closer to router

#### Disconnects frequently
1. Check signal strength in System Info
2. Move closer to router
3. Avoid interference from microwaves, Bluetooth

#### "No WiFi" in apps
1. Connect to WiFi first via Settings app
2. Check if network requires captive portal (not supported)

### 10.3 Button Issues

#### Button not responding
1. Check wiring to correct GPIO
2. Verify button is connected to GND
3. Use Button Test in System Info to diagnose
4. Check for short circuits

#### Button triggers multiple times
1. Debouncing issue - increase `DEBOUNCE_MS` in config.h
2. Check button for physical damage
3. Verify good ground connection

#### Wrong button mapping
1. Check pin definitions in config.h
2. Use Button Test to identify which GPIO each button triggers
3. Update config.h to match your wiring

### 10.4 Build Errors

#### "fatal error: file not found"
```bash
# Clean and rebuild
~/.platformio/penv/bin/pio run -t clean
~/.platformio/penv/bin/pio run
```

#### "multiple definition" errors
- Check for duplicate includes
- Ensure header guards are correct

#### "undefined reference"
- Missing implementation file
- Check all .cpp files are in src/ folder

### 10.5 Upload Issues

#### "No serial port detected"
1. Try different USB cable (data cable, not charge-only)
2. Install USB drivers for your ESP32's USB chip
3. Check Device Manager (Windows) or `ls /dev/tty*` (Mac/Linux)

#### "Upload failed"
1. Hold BOOT button while uploading
2. Try lower upload speed in platformio.ini:
   ```ini
   upload_speed = 115200
   ```
3. Press EN (reset) button after upload starts

#### "Timed out waiting for packet header"
1. Hold BOOT button during entire upload
2. Check USB cable quality
3. Try different USB port

### 10.6 API Issues

#### "Weather/News not loading"
1. Check WiFi connection
2. Verify API key is correct in config.h
3. API rate limit may be exceeded (wait and retry)
4. Check serial monitor for error messages

#### "API key invalid"
1. Regenerate key from provider website
2. Check for extra spaces in config.h
3. Some APIs take time to activate new keys

---

## Quick Reference Card

```
+----------------------------------+
|        ESP-OS CONTROLS           |
+----------------------------------+
|                                  |
|    [UP]        NAVIGATION        |
|  [L]  [R]      D-Pad moves       |
|   [DOWN]       cursor/selection  |
|                                  |
|  [A] - Select/Confirm            |
|  [B] - Back/Exit                 |
|  [C] - Toggle/Secondary          |
|  [D] - Menu/Options              |
|                                  |
+----------------------------------+
|        ON-SCREEN KEYBOARD        |
+----------------------------------+
|  D-Pad: Move cursor              |
|  A: Type character               |
|  B: Backspace                    |
|  C: Confirm input                |
|  D: Cancel                       |
+----------------------------------+
```

---

## Support

For issues and feature requests, visit:
https://github.com/nikzart/esp-os/issues

---

*Last updated: December 2024*
