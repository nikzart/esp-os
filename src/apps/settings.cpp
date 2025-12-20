#include "apps/settings.h"
#include "ui.h"
#include "wifi_manager.h"
#include "keyboard.h"
#include "icons.h"
#include <Preferences.h>

void SettingsApp::init() {
    mode = Mode::MENU;
    menuIndex = 0;
    loadSettings();
}

void SettingsApp::loadSettings() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    prefs.getString("weather_key", weatherApiKey, sizeof(weatherApiKey));
    prefs.getString("news_key", newsApiKey, sizeof(newsApiKey));
    prefs.end();
}

void SettingsApp::saveSettings() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString("weather_key", weatherApiKey);
    prefs.putString("news_key", newsApiKey);
    prefs.end();
}

void SettingsApp::update() {
    if (mode == Mode::WIFI_PASSWORD && Keyboard::isConfirmed()) {
        // Connect with password
        const char* password = Keyboard::getText();
        if (WiFiManager::connect(selectedSSID, password)) {
            WiFiManager::saveNetwork(selectedSSID, password);
            UI::beep(1000, 100);
        } else {
            UI::beep(500, 200);
        }
        mode = Mode::MENU;
    } else if (mode == Mode::WIFI_PASSWORD && Keyboard::isCancelled()) {
        mode = Mode::WIFI_LIST;
    } else if (mode == Mode::EDIT_API_KEY && Keyboard::isConfirmed()) {
        if (apiKeyIndex == 0) {
            strncpy(weatherApiKey, Keyboard::getText(), sizeof(weatherApiKey) - 1);
        } else {
            strncpy(newsApiKey, Keyboard::getText(), sizeof(newsApiKey) - 1);
        }
        saveSettings();
        mode = Mode::API_KEYS;
    } else if (mode == Mode::EDIT_API_KEY && Keyboard::isCancelled()) {
        mode = Mode::API_KEYS;
    }
}

void SettingsApp::render() {
    if (Keyboard::isActive()) {
        Keyboard::render();
        return;
    }

    UI::clear();
    UI::drawTitleBar("Settings");

    switch (mode) {
        case Mode::MENU:
            renderMenu();
            break;
        case Mode::WIFI_LIST:
        case Mode::WIFI_SCAN:
            renderWifiList();
            break;
        case Mode::API_KEYS:
            renderApiKeys();
            break;
        default:
            break;
    }

    UI::drawStatusBar("A:Select", "B:Back");
    UI::flush();
}

void SettingsApp::renderMenu() {
    const char* items[] = {"WiFi", "API Keys", "Sound: ON", "About"};
    int count = 4;

    for (int i = 0; i < count; i++) {
        UI::drawMenuItem(24 + i * 11, items[i], i == menuIndex);
    }
}

void SettingsApp::renderWifiList() {
    if (scanning) {
        UI::drawCentered(35, "Scanning...");
        return;
    }

    int count = WiFiManager::getScanCount();
    WiFiNetwork* networks = WiFiManager::getScanResults();

    if (count == 0) {
        UI::drawCentered(35, "No networks found");
        UI::drawCentered(48, "Press C to scan");
        return;
    }

    // Show networks
    int startIdx = max(0, wifiIndex - 2);
    int endIdx = min(count, startIdx + 4);

    for (int i = startIdx; i < endIdx; i++) {
        int y = 24 + (i - startIdx) * 11;
        char line[32];
        int rssi = networks[i].rssi;
        const char* strength = rssi > -50 ? "***" : (rssi > -70 ? "**" : "*");
        snprintf(line, sizeof(line), "%s %s", networks[i].ssid, strength);
        UI::drawMenuItem(y, line, i == wifiIndex);
    }
}

void SettingsApp::renderApiKeys() {
    const char* items[] = {"Weather API Key", "News API Key"};

    for (int i = 0; i < 2; i++) {
        UI::drawMenuItem(24 + i * 11, items[i], i == apiKeyIndex);
    }

    // Show current values (truncated)
    UI::setSmallFont();
    if (strlen(weatherApiKey) > 0) {
        u8g2.drawStr(4, 50, "Weather: Set");
    } else {
        u8g2.drawStr(4, 50, "Weather: Not set");
    }
    if (strlen(newsApiKey) > 0) {
        u8g2.drawStr(4, 58, "News: Set");
    } else {
        u8g2.drawStr(4, 58, "News: Not set");
    }
    UI::setNormalFont();
}

void SettingsApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (Keyboard::isActive()) {
        Keyboard::onButton(btn, pressed);
        return;
    }

    UI::beep();

    switch (mode) {
        case Mode::MENU:
            if (btn == BTN_UP && menuIndex > 0) menuIndex--;
            else if (btn == BTN_DOWN && menuIndex < 3) menuIndex++;
            else if (btn == BTN_A) {
                if (menuIndex == 0) {
                    mode = Mode::WIFI_LIST;
                    wifiIndex = 0;
                    WiFiManager::scan();
                } else if (menuIndex == 1) {
                    mode = Mode::API_KEYS;
                    apiKeyIndex = 0;
                }
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;

        case Mode::WIFI_LIST:
            if (btn == BTN_UP && wifiIndex > 0) wifiIndex--;
            else if (btn == BTN_DOWN && wifiIndex < WiFiManager::getScanCount() - 1) wifiIndex++;
            else if (btn == BTN_A && WiFiManager::getScanCount() > 0) {
                WiFiNetwork* networks = WiFiManager::getScanResults();
                strncpy(selectedSSID, networks[wifiIndex].ssid, sizeof(selectedSSID) - 1);

                if (networks[wifiIndex].open) {
                    // Open network, connect directly
                    if (WiFiManager::connect(selectedSSID, "")) {
                        WiFiManager::saveNetwork(selectedSSID, "");
                        UI::beep(1000, 100);
                    }
                    mode = Mode::MENU;
                } else {
                    // Need password
                    mode = Mode::WIFI_PASSWORD;
                    Keyboard::show("Password:", keyboardBuffer, sizeof(keyboardBuffer));
                }
            } else if (btn == BTN_C) {
                scanning = true;
                WiFiManager::scan();
                scanning = false;
            } else if (btn == BTN_B) {
                mode = Mode::MENU;
            }
            break;

        case Mode::API_KEYS:
            if (btn == BTN_UP && apiKeyIndex > 0) apiKeyIndex--;
            else if (btn == BTN_DOWN && apiKeyIndex < 1) apiKeyIndex++;
            else if (btn == BTN_A) {
                mode = Mode::EDIT_API_KEY;
                char* currentKey = (apiKeyIndex == 0) ? weatherApiKey : newsApiKey;
                Keyboard::show(apiKeyIndex == 0 ? "Weather Key:" : "News Key:",
                              currentKey, 48);
            } else if (btn == BTN_B) {
                mode = Mode::MENU;
            }
            break;

        default:
            break;
    }
}

const uint8_t* SettingsApp::getIcon() {
    return icon_settings;
}
