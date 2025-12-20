#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "ui.h"
#include "input.h"
#include "app.h"
#include "keyboard.h"
#include "wifi_manager.h"

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

// App list (excluding launcher)
App* apps[] = {
    &weatherApp,
    &cryptoApp,
    &newsApp,
    &snakeApp,
    &pongApp,
    &factsApp,
    &jokesApp,
    &quotesApp,
    &issApp,
    &triviaApp,
    &settingsApp,
    &sysInfoApp
};
const int appCount = sizeof(apps) / sizeof(apps[0]);

// Current state
AppState currentState = AppState::LAUNCHER;
App* currentApp = nullptr;

// Button callback
void onButtonEvent(uint8_t btn, bool pressed) {
    if (Keyboard::isActive()) {
        Keyboard::onButton(btn, pressed);
        return;
    }

    if (currentState == AppState::LAUNCHER) {
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

    // Initialize components
    UI::init();
    Input::init();
    Input::setCallback(onButtonEvent);
    Keyboard::init();
    WiFiManager::init();

    // Show splash screen
    UI::clear();
    UI::drawCentered(25, "ESP32 OS");
    UI::setSmallFont();
    UI::drawCentered(40, "Loading...");
    UI::setNormalFont();
    UI::flush();

    // Try auto-connect to WiFi
    Serial.println("Auto-connecting WiFi...");
    WiFiManager::autoConnect();

    delay(1000);

    // Initialize launcher
    launcher.setApps(apps, appCount);
    launcher.init();

    // Startup beep
    UI::beep(1000, 50);
    delay(100);
    UI::beep(1500, 50);

    Serial.println("Ready!");
}

void loop() {
    // Update input
    Input::update();

    // Update keyboard if active
    if (Keyboard::isActive()) {
        Keyboard::update();
        Keyboard::render();
        return;
    }

    // Update and render based on state
    if (currentState == AppState::LAUNCHER) {
        launcher.update();
        launcher.render();
    } else if (currentState == AppState::APP_RUNNING && currentApp) {
        currentApp->update();
        currentApp->render();
    }

    // Small delay to prevent hogging CPU
    delay(10);
}
