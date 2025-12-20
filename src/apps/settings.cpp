#include "apps/settings.h"
#include "ui.h"
#include "wifi_manager.h"
#include "keyboard.h"
#include "icons.h"
#include "input.h"
#include "homescreen.h"
#include <Preferences.h>

// Timezone data
static const char* timezoneNames[] = {
    "UTC", "IST +5:30", "EST -5", "CST -6", "MST -7", "PST -8", "CET +1", "JST +9"
};
static const long timezoneOffsets[] = {
    0, 19800, -18000, -21600, -25200, -28800, 3600, 32400
};
static const int timezoneCount = 8;

void SettingsApp::init() {
    mode = Mode::MENU;
    menuIndex = 0;
    brightness = UI::getBrightness();
    sleepTimeoutIndex = Input::getSleepTimeoutIndex();
    loadSettings();
}

void SettingsApp::loadSettings() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    prefs.getString("weather_key", weatherApiKey, sizeof(weatherApiKey));
    prefs.getString("news_key", newsApiKey, sizeof(newsApiKey));
    timezoneIndex = prefs.getInt("timezone_idx", 1);  // Default IST
    use24Hour = prefs.getBool("use_24hour", true);
    if (timezoneIndex < 0 || timezoneIndex >= timezoneCount) timezoneIndex = 1;
    prefs.end();
}

void SettingsApp::saveSettings() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString("weather_key", weatherApiKey);
    prefs.putString("news_key", newsApiKey);
    prefs.putInt("timezone_idx", timezoneIndex);
    prefs.putBool("use_24hour", use24Hour);
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

    // Render content and get appropriate status bar text
    const char* statusLeft = "A:Select";
    const char* statusRight = "B:Back";

    switch (mode) {
        case Mode::MENU:
            renderMenu();
            break;
        case Mode::WIFI_LIST:
        case Mode::WIFI_SCAN:
            renderWifiList();
            statusLeft = "A:Connect C:Scan";
            break;
        case Mode::API_KEYS:
            renderApiKeys();
            statusLeft = "A:Edit";
            break;
        case Mode::BRIGHTNESS:
            renderBrightness();
            statusLeft = "L/R:Adj A:Save";
            break;
        case Mode::SLEEP_TIMEOUT:
            renderSleepTimeout();
            break;
        case Mode::TIME_SETTINGS:
            renderTimeSettings();
            statusLeft = "L/R:Adj A:Save";
            break;
        default:
            break;
    }

    UI::drawStatusBar(statusLeft, statusRight);
    UI::flush();
}

void SettingsApp::renderMenu() {
    const char* items[] = {"WiFi", "API Keys", "Brightness", "Sleep", "Time", "Restart"};
    int count = 6;
    int visibleCount = 3;  // Max items that fit before status bar

    // Calculate scroll offset to keep selection visible
    int scrollOffset = 0;
    if (menuIndex >= visibleCount) {
        scrollOffset = menuIndex - visibleCount + 1;
    }

    // Render visible items
    for (int i = 0; i < visibleCount && (i + scrollOffset) < count; i++) {
        int itemIndex = i + scrollOffset;
        UI::drawMenuItem(24 + i * 11, items[itemIndex], itemIndex == menuIndex);
    }

    // Draw scrollbar if needed
    if (count > visibleCount) {
        UI::drawScrollbar(125, 14, 38, scrollOffset, count, visibleCount);
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

    // Show current values (moved up to avoid status bar)
    UI::setSmallFont();
    if (strlen(weatherApiKey) > 0) {
        u8g2.drawStr(4, 46, "Weather: Set");
    } else {
        u8g2.drawStr(4, 46, "Weather: Not set");
    }
    if (strlen(newsApiKey) > 0) {
        u8g2.drawStr(64, 46, "News: Set");
    } else {
        u8g2.drawStr(64, 46, "News: Not set");
    }
    UI::setNormalFont();
}

void SettingsApp::renderBrightness() {
    UI::drawCentered(28, "Brightness");

    // Draw brightness bar
    int barWidth = 100;
    int barX = (SCREEN_WIDTH - barWidth) / 2;
    UI::drawProgressBar(barX, 36, barWidth, 10, brightness * 100 / 255);

    // Show percentage
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", brightness * 100 / 255);
    UI::drawCentered(52, buf);
    // Hints moved to status bar
}

void SettingsApp::renderSleepTimeout() {
    const char* options[] = {"Off", "30 sec", "1 min", "2 min", "5 min"};
    int count = 5;
    int visibleCount = 3;

    UI::drawCentered(20, "Sleep Timeout");

    // Calculate scroll offset
    int scrollOffset = 0;
    if (sleepTimeoutIndex >= visibleCount) {
        scrollOffset = sleepTimeoutIndex - visibleCount + 1;
    }

    // Render visible items
    for (int i = 0; i < visibleCount && (i + scrollOffset) < count; i++) {
        int itemIndex = i + scrollOffset;
        int y = 32 + i * 10;
        if (itemIndex == sleepTimeoutIndex) {
            u8g2.drawBox(20, y - 8, 88, 10);
            u8g2.setDrawColor(0);
        }
        int w = u8g2.getStrWidth(options[itemIndex]);
        u8g2.drawStr((SCREEN_WIDTH - w) / 2, y, options[itemIndex]);
        u8g2.setDrawColor(1);
    }

    // Draw scrollbar
    if (count > visibleCount) {
        UI::drawScrollbar(120, 24, 28, scrollOffset, count, visibleCount);
    }
}

void SettingsApp::renderTimeSettings() {
    UI::drawCentered(18, "Time Settings");

    // Timezone selector
    char tzLabel[24];
    snprintf(tzLabel, sizeof(tzLabel), "Zone: %s", timezoneNames[timezoneIndex]);
    UI::drawMenuItem(30, tzLabel, timeSettingIndex == 0);

    // 12/24 hour toggle
    const char* formatStr = use24Hour ? "Format: 24-hour" : "Format: 12-hour";
    UI::drawMenuItem(42, formatStr, timeSettingIndex == 1);
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
            else if (btn == BTN_DOWN && menuIndex < 5) menuIndex++;
            else if (btn == BTN_A) {
                if (menuIndex == 0) {
                    mode = Mode::WIFI_LIST;
                    wifiIndex = 0;
                    WiFiManager::scan();
                } else if (menuIndex == 1) {
                    mode = Mode::API_KEYS;
                    apiKeyIndex = 0;
                } else if (menuIndex == 2) {
                    mode = Mode::BRIGHTNESS;
                    brightness = UI::getBrightness();
                } else if (menuIndex == 3) {
                    mode = Mode::SLEEP_TIMEOUT;
                } else if (menuIndex == 4) {
                    mode = Mode::TIME_SETTINGS;
                    timeSettingIndex = 0;
                } else if (menuIndex == 5) {
                    // Restart device
                    ESP.restart();
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

        case Mode::BRIGHTNESS:
            if (btn == BTN_LEFT && brightness > 15) {
                brightness -= 16;
                UI::setBrightness(brightness);
            } else if (btn == BTN_RIGHT && brightness < 255) {
                brightness = min(255, brightness + 16);
                UI::setBrightness(brightness);
            } else if (btn == BTN_A) {
                UI::saveBrightness();
                mode = Mode::MENU;
            } else if (btn == BTN_B) {
                brightness = UI::getBrightness();  // Restore original
                UI::setBrightness(brightness);
                mode = Mode::MENU;
            }
            break;

        case Mode::SLEEP_TIMEOUT:
            if (btn == BTN_UP && sleepTimeoutIndex > 0) sleepTimeoutIndex--;
            else if (btn == BTN_DOWN && sleepTimeoutIndex < 4) sleepTimeoutIndex++;
            else if (btn == BTN_A) {
                Input::saveSleepTimeout(sleepTimeoutIndex);
                Input::resetActivity();  // Reset timer after saving
                mode = Mode::MENU;
            } else if (btn == BTN_B) {
                sleepTimeoutIndex = Input::getSleepTimeoutIndex();  // Restore original
                mode = Mode::MENU;
            }
            break;

        case Mode::TIME_SETTINGS:
            if (btn == BTN_UP && timeSettingIndex > 0) timeSettingIndex--;
            else if (btn == BTN_DOWN && timeSettingIndex < 1) timeSettingIndex++;
            else if (btn == BTN_LEFT) {
                if (timeSettingIndex == 0) {
                    // Decrease timezone
                    timezoneIndex = (timezoneIndex - 1 + timezoneCount) % timezoneCount;
                } else {
                    // Toggle time format
                    use24Hour = !use24Hour;
                }
            } else if (btn == BTN_RIGHT) {
                if (timeSettingIndex == 0) {
                    // Increase timezone
                    timezoneIndex = (timezoneIndex + 1) % timezoneCount;
                } else {
                    // Toggle time format
                    use24Hour = !use24Hour;
                }
            } else if (btn == BTN_A) {
                saveSettings();
                Homescreen::loadTimeSettings();
                Homescreen::syncTime();
                mode = Mode::MENU;
            } else if (btn == BTN_B) {
                loadSettings();  // Restore original
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
