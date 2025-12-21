#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "ui.h"
#include "input.h"
#include "app.h"
#include "keyboard.h"
#include "wifi_manager.h"
#include "homescreen.h"

// App includes
#include "apps/launcher.h"
#include "apps/weather.h"
#include "apps/crypto.h"
#include "apps/news.h"
#include "apps/settings.h"
#include "apps/sysinfo.h"
#include "apps/snake.h"
#include "apps/pong.h"
#include "apps/facts.h"
#include "apps/jokes.h"
#include "apps/quotes.h"
#include "apps/iss.h"
#include "apps/trivia.h"
#include "apps/ota.h"
#include "apps/timer.h"
#include "apps/bulb.h"

// Global display (defined in ui.cpp)
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

// App instances
LauncherApp launcher;
WeatherApp weatherApp;
CryptoApp cryptoApp;
NewsApp newsApp;
SettingsApp settingsApp;
SysInfoApp sysInfoApp;
SnakeApp snakeApp;
PongApp pongApp;
FactsApp factsApp;
JokesApp jokesApp;
QuotesApp quotesApp;
ISSApp issApp;
TriviaApp triviaApp;
OTAApp otaApp;
TimerApp timerApp;
BulbApp bulbApp;

// App list (excluding launcher)
App* apps[] = {
    &weatherApp,
    &bulbApp,
    &cryptoApp,
    &newsApp,
    &snakeApp,
    &pongApp,
    &factsApp,
    &jokesApp,
    &quotesApp,
    &issApp,
    &triviaApp,
    &timerApp,
    &settingsApp,
    &sysInfoApp,
    &otaApp
};
const int appCount = sizeof(apps) / sizeof(apps[0]);

// Current state
AppState currentState = AppState::HOMESCREEN;
App* currentApp = nullptr;

// Boot animation helper
void showBootProgress(int percent, const char* status) {
    UI::clear();
    UI::setLargeFont();
    UI::drawCentered(22, "ESP32 OS");
    UI::setNormalFont();
    UI::drawProgressBar(14, 32, 100, 8, percent);
    UI::setSmallFont();
    UI::drawCentered(52, status);
    UI::setNormalFont();
    UI::flush();
}

// Button callback
void onButtonEvent(uint8_t btn, bool pressed) {
    if (Keyboard::isActive()) {
        Keyboard::onButton(btn, pressed);
        return;
    }

    if (currentState == AppState::HOMESCREEN) {
        if (Homescreen::onButton(btn, pressed)) {
            currentState = AppState::LAUNCHER;
            launcher.init();
        }
    } else if (currentState == AppState::LAUNCHER) {
        // B button goes back to homescreen
        if (btn == BTN_B && pressed) {
            UI::beep();
            currentState = AppState::HOMESCREEN;
            return;
        }

        launcher.onButton(btn, pressed);

        // Check if launcher wants to launch an app
        if (launcher.wantsToExit && pressed && btn == BTN_A) {
            launcher.wantsToExit = false;
            int idx = launcher.getSelectedApp();
            if (idx >= 0 && idx < appCount) {
                currentApp = apps[idx];
                currentApp->init();
                currentState = AppState::APP_RUNNING;
            }
        }
    } else if (currentState == AppState::APP_RUNNING && currentApp) {
        currentApp->onButton(btn, pressed);

        // Check if app wants to exit
        if (currentApp->wantsToExit) {
            currentApp->wantsToExit = false;
            currentApp->onClose();
            currentApp = nullptr;
            currentState = AppState::LAUNCHER;
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Mini OS ===");

    // Initialize display first
    UI::init();
    showBootProgress(10, "Display ready");
    delay(100);

    // Load saved brightness
    UI::loadBrightness();
    showBootProgress(20, "Settings loaded");
    delay(100);

    // Initialize input
    Input::init();
    Input::setCallback(onButtonEvent);
    Input::loadSleepTimeout();
    showBootProgress(30, "Input ready");
    delay(100);

    // Initialize keyboard
    Keyboard::init();
    showBootProgress(40, "Keyboard ready");
    delay(100);

    // Initialize WiFi
    WiFiManager::init();
    showBootProgress(50, "Connecting WiFi...");

    // Try auto-connect to WiFi
    Serial.println("Auto-connecting WiFi...");
    WiFiManager::autoConnect();

    if (WiFiManager::isConnected()) {
        showBootProgress(80, "WiFi connected");
    } else {
        showBootProgress(80, "WiFi offline");
    }
    delay(200);

    // Initialize homescreen
    showBootProgress(90, "Loading homescreen...");
    Homescreen::init();
    delay(100);

    // Initialize launcher
    launcher.setApps(apps, appCount);
    showBootProgress(100, "Ready!");
    delay(300);

    // Startup beeps
    UI::beep(1000, 50);
    delay(100);
    UI::beep(1500, 50);

    Serial.println("Ready!");
}

void loop() {
    // Update input
    Input::update();

    // Check for sleep timeout
    unsigned long sleepMs = Input::getSleepTimeoutMs();
    if (sleepMs > 0) {
        unsigned long inactive = millis() - Input::getLastActivity();
        if (inactive >= sleepMs) {
            // Turn off display and enter light sleep
            u8g2.setPowerSave(1);
            delay(100);
            Input::enterSleep();
            // Continues here after wake from light sleep
            u8g2.setPowerSave(0);
        }
    }

    // Update keyboard if active
    if (Keyboard::isActive()) {
        Keyboard::update();
        Keyboard::render();
        return;
    }

    // Update and render based on state
    if (currentState == AppState::HOMESCREEN) {
        Homescreen::update();
        Homescreen::render();
    } else if (currentState == AppState::LAUNCHER) {
        launcher.update();
        launcher.render();
    } else if (currentState == AppState::APP_RUNNING && currentApp) {
        currentApp->update();
        currentApp->render();
    }

    // Small delay to prevent hogging CPU
    delay(10);
}
