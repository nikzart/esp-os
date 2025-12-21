#include "apps/bulb.h"
#include "ui.h"
#include "wifi_manager.h"
#include "keyboard.h"
#include "icons.h"
#include "config.h"
#include <ArduinoJson.h>

void BulbApp::init() {
    mode = Mode::MAIN;
    menuIndex = 0;
    colorIndex = 0;
    presetIndex = 0;
    loading = false;
    hasConnection = false;
    errorMsg[0] = '\0';
    editingIP = false;

    loadSettings();
    loadPresets();

    // Try to fetch initial state if IP is configured
    if (strlen(bulbIP) > 0) {
        fetchState();
    }
}

void BulbApp::update() {
    // Handle keyboard input for IP configuration
    if (editingIP) {
        if (Keyboard::isConfirmed()) {
            strncpy(bulbIP, Keyboard::getText(), sizeof(bulbIP) - 1);
            bulbIP[sizeof(bulbIP) - 1] = '\0';
            saveSettings();
            editingIP = false;
            fetchState();
        } else if (Keyboard::isCancelled()) {
            editingIP = false;
        }
        return;
    }

    // Auto-refresh state every 30 seconds if connected
    if (hasConnection && strlen(bulbIP) > 0 && millis() - lastFetch > 30000) {
        fetchState();
    }
}

void BulbApp::render() {
    if (Keyboard::isActive()) {
        Keyboard::render();
        return;
    }

    UI::clear();
    UI::drawTitleBar("Bulb Control");

    switch (mode) {
        case Mode::MAIN:
            renderMain();
            break;
        case Mode::BRIGHTNESS:
            renderBrightness();
            break;
        case Mode::COLOR:
            renderColor();
            break;
        case Mode::CT:
            renderCT();
            break;
        case Mode::PRESETS:
            renderPresets();
            break;
        case Mode::SETTINGS:
            renderSettings();
            break;
    }

    UI::flush();
}

void BulbApp::renderMain() {
    if (loading) {
        UI::drawCentered(35, "Connecting...");
        UI::drawStatusBar("", "B:Back");
        return;
    }

    if (strlen(bulbIP) == 0) {
        UI::drawCentered(28, "No bulb configured");
        UI::setSmallFont();
        UI::drawCentered(42, "Press A to configure");
        UI::setNormalFont();
        UI::drawStatusBar("A:Settings", "B:Back");
        return;
    }

    if (strlen(errorMsg) > 0) {
        UI::drawCentered(28, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(42, "A:Retry C:Settings");
        UI::setNormalFont();
        UI::drawStatusBar("A:Retry", "B:Back");
        return;
    }

    // Power status - clear visual indicator
    if (power) {
        UI::drawCentered(20, "[ * ]");
        UI::drawCentered(32, "Bulb ON");
    } else {
        UI::drawCentered(20, "[   ]");
        UI::drawCentered(32, "Bulb OFF");
    }

    // Menu items
    const char* items[] = {"Brightness", "Color", "Temperature", "Presets", "Settings"};
    int count = 5;

    // Show current selection indicator
    char buf[32];
    snprintf(buf, sizeof(buf), "%s >", items[menuIndex]);
    UI::setSmallFont();
    UI::drawCentered(42, buf);

    // Show current value hint
    switch (menuIndex) {
        case 0:
            snprintf(buf, sizeof(buf), "%d%%", brightness);
            break;
        case 1:
            snprintf(buf, sizeof(buf), "%s", getColorName());
            break;
        case 2:
            snprintf(buf, sizeof(buf), "%dK", ctToKelvin(colorTemp));
            break;
        case 3:
            snprintf(buf, sizeof(buf), "4 slots");
            break;
        case 4:
            snprintf(buf, sizeof(buf), "%s", bulbIP);
            break;
    }
    UI::drawCentered(52, buf);
    UI::setNormalFont();

    UI::drawStatusBar("A:Sel C:Pwr", "B:Back");
}

void BulbApp::renderBrightness() {
    UI::drawCentered(22, "Brightness");

    // Progress bar
    int barWidth = 100;
    int barX = (SCREEN_WIDTH - barWidth) / 2;
    UI::drawProgressBar(barX, 30, barWidth, 10, brightness);

    // Percentage
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", brightness);
    UI::drawCentered(48, buf);

    UI::drawStatusBar("L/R:Adj A:Apply", "B:Back");
}

void BulbApp::renderColor() {
    UI::drawCentered(18, "Color");

    // Hue slider
    char buf[24];
    int barWidth = 80;
    int barX = 24;

    // Hue
    u8g2.drawStr(4, 30, "H:");
    if (colorIndex == 0) {
        u8g2.drawFrame(barX - 2, 22, barWidth + 4, 10);
    }
    UI::drawProgressBar(barX, 24, barWidth, 6, hue * 100 / 360);
    snprintf(buf, sizeof(buf), "%d", hue);
    u8g2.drawStr(108, 30, buf);

    // Saturation
    u8g2.drawStr(4, 44, "S:");
    if (colorIndex == 1) {
        u8g2.drawFrame(barX - 2, 36, barWidth + 4, 10);
    }
    UI::drawProgressBar(barX, 38, barWidth, 6, saturation);
    snprintf(buf, sizeof(buf), "%d", saturation);
    u8g2.drawStr(108, 44, buf);

    // Show color name
    UI::drawCentered(52, getColorName());

    UI::drawStatusBar("U/D:Sel L/R:Adj A:Set", "B:Back");
}

void BulbApp::renderCT() {
    UI::drawCentered(20, "Color Temperature");

    // Temperature bar
    int barWidth = 100;
    int barX = (SCREEN_WIDTH - barWidth) / 2;

    // CT range: 153 (cool/6500K) to 500 (warm/2000K)
    int percent = (colorTemp - 153) * 100 / (500 - 153);
    UI::drawProgressBar(barX, 32, barWidth, 10, percent);

    // Labels
    UI::setSmallFont();
    u8g2.drawStr(barX - 4, 30, "Cool");
    u8g2.drawStr(barX + barWidth - 16, 30, "Warm");
    UI::setNormalFont();

    // Kelvin value
    char buf[16];
    snprintf(buf, sizeof(buf), "%dK", ctToKelvin(colorTemp));
    UI::drawCentered(52, buf);

    UI::drawStatusBar("L/R:Adj A:Apply", "B:Back");
}

void BulbApp::renderPresets() {
    // Show preset list
    for (int i = 0; i < PRESET_COUNT; i++) {
        char buf[24];
        if (presets[i].valid) {
            snprintf(buf, sizeof(buf), "%d. %s", i + 1, presets[i].name);
        } else {
            snprintf(buf, sizeof(buf), "%d. [Empty]", i + 1);
        }
        UI::drawMenuItem(20 + i * 10, buf, i == presetIndex);
    }

    UI::drawStatusBar("A:Load C:Save", "B:Back");
}

void BulbApp::renderSettings() {
    UI::drawCentered(24, "Bulb IP Address");

    // Show current IP
    if (strlen(bulbIP) > 0) {
        UI::drawCentered(38, bulbIP);
    } else {
        UI::setSmallFont();
        UI::drawCentered(38, "Not configured");
        UI::setNormalFont();
    }

    UI::drawStatusBar("A:Edit", "B:Back");
}

void BulbApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (Keyboard::isActive()) {
        Keyboard::onButton(btn, pressed);
        return;
    }

    UI::beep();

    switch (mode) {
        case Mode::MAIN:
            if (strlen(bulbIP) == 0) {
                // No IP configured - A goes to settings
                if (btn == BTN_A) {
                    mode = Mode::SETTINGS;
                } else if (btn == BTN_B || btn == BTN_D) {
                    wantsToExit = true;
                }
                return;
            }

            if (strlen(errorMsg) > 0) {
                // Error state
                if (btn == BTN_A) {
                    fetchState();
                } else if (btn == BTN_C) {
                    mode = Mode::SETTINGS;
                } else if (btn == BTN_B || btn == BTN_D) {
                    wantsToExit = true;
                }
                return;
            }

            // Normal main screen - navigate with UP/DOWN or LEFT/RIGHT
            if ((btn == BTN_UP || btn == BTN_LEFT) && menuIndex > 0) {
                menuIndex--;
            } else if ((btn == BTN_DOWN || btn == BTN_RIGHT) && menuIndex < 4) {
                menuIndex++;
            } else if (btn == BTN_A) {
                if (menuIndex == 0) {
                    mode = Mode::BRIGHTNESS;
                } else if (menuIndex == 1) {
                    mode = Mode::COLOR;
                    colorIndex = 0;
                } else if (menuIndex == 2) {
                    mode = Mode::CT;
                } else if (menuIndex == 3) {
                    mode = Mode::PRESETS;
                    presetIndex = 0;
                } else if (menuIndex == 4) {
                    mode = Mode::SETTINGS;
                }
            } else if (btn == BTN_C) {
                // Quick toggle power
                togglePower();
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;

        case Mode::BRIGHTNESS:
            if (btn == BTN_LEFT && brightness > 0) {
                brightness = max(0, brightness - 10);
            } else if (btn == BTN_RIGHT && brightness < 100) {
                brightness = min(100, brightness + 10);
            } else if (btn == BTN_A) {
                applyBrightness();
                mode = Mode::MAIN;
            } else if (btn == BTN_B) {
                mode = Mode::MAIN;
            }
            break;

        case Mode::COLOR:
            if (btn == BTN_UP) {
                colorIndex = 0; // Hue
            } else if (btn == BTN_DOWN) {
                colorIndex = 1; // Saturation
            } else if (btn == BTN_LEFT) {
                if (colorIndex == 0) {
                    hue = (hue - 15 + 360) % 360;
                } else {
                    saturation = max(0, saturation - 10);
                }
            } else if (btn == BTN_RIGHT) {
                if (colorIndex == 0) {
                    hue = (hue + 15) % 360;
                } else {
                    saturation = min(100, saturation + 10);
                }
            } else if (btn == BTN_A) {
                applyColor();
                mode = Mode::MAIN;
            } else if (btn == BTN_B) {
                mode = Mode::MAIN;
            }
            break;

        case Mode::CT:
            if (btn == BTN_LEFT && colorTemp > 153) {
                colorTemp = max(153, colorTemp - 25);
            } else if (btn == BTN_RIGHT && colorTemp < 500) {
                colorTemp = min(500, colorTemp + 25);
            } else if (btn == BTN_A) {
                applyColorTemp();
                mode = Mode::MAIN;
            } else if (btn == BTN_B) {
                mode = Mode::MAIN;
            }
            break;

        case Mode::PRESETS:
            if (btn == BTN_UP && presetIndex > 0) {
                presetIndex--;
            } else if (btn == BTN_DOWN && presetIndex < PRESET_COUNT - 1) {
                presetIndex++;
            } else if (btn == BTN_A) {
                loadPreset(presetIndex);
            } else if (btn == BTN_C) {
                savePreset(presetIndex);
            } else if (btn == BTN_B) {
                mode = Mode::MAIN;
            }
            break;

        case Mode::SETTINGS:
            if (btn == BTN_A) {
                editingIP = true;
                strncpy(ipBuffer, bulbIP, sizeof(ipBuffer));
                Keyboard::show("Bulb IP:", ipBuffer, sizeof(ipBuffer));
            } else if (btn == BTN_B) {
                mode = Mode::MAIN;
            }
            break;
    }
}

// HTTP/Tasmota methods
void BulbApp::sendCommand(const char* cmd) {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    if (strlen(bulbIP) == 0) {
        strcpy(errorMsg, "No IP set");
        return;
    }

    char url[128];
    snprintf(url, sizeof(url), "http://%s/cm?cmnd=%s", bulbIP, cmd);

    // URL encode spaces as %20
    String urlStr = url;
    urlStr.replace(" ", "%20");

    WiFiManager::httpGet(urlStr.c_str());
}

void BulbApp::fetchState() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        hasConnection = false;
        return;
    }

    loading = true;
    errorMsg[0] = '\0';

    char url[128];
    snprintf(url, sizeof(url), "http://%s/cm?cmnd=State", bulbIP);

    String response = WiFiManager::httpGet(url);
    loading = false;

    if (response.length() == 0) {
        strcpy(errorMsg, "Connection failed");
        hasConnection = false;
        return;
    }

    // Parse JSON response (Tasmota State response can be 1-2KB)
    StaticJsonDocument<1536> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        strcpy(errorMsg, "Parse error");
        hasConnection = false;
        return;
    }

    // Extract state - check both POWER and POWER1 keys (single vs multi-relay)
    const char* powerStr = "OFF";
    if (doc.containsKey("POWER")) {
        powerStr = doc["POWER"].as<const char*>();
    } else if (doc.containsKey("POWER1")) {
        powerStr = doc["POWER1"].as<const char*>();
    }
    power = (strcmp(powerStr, "ON") == 0);

    brightness = doc["Dimmer"] | 100;

    // Parse HSBColor if present (format: "hue,sat,bright")
    const char* hsbStr = doc["HSBColor"] | "";
    if (strlen(hsbStr) > 0) {
        int h, s, b;
        if (sscanf(hsbStr, "%d,%d,%d", &h, &s, &b) == 3) {
            hue = h;
            saturation = s;
            // brightness already set from Dimmer
        }
    }

    // Color temperature
    colorTemp = doc["CT"] | 326;

    hasConnection = true;
    lastFetch = millis();
}

void BulbApp::setPower(bool on) {
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "Power%%20%s", on ? "ON" : "OFF");
    sendCommand(cmd);
    power = on;
}

void BulbApp::togglePower() {
    sendCommand("Power%20TOGGLE");
    power = !power;
}

void BulbApp::applyBrightness() {
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "Dimmer%%20%d", brightness);
    sendCommand(cmd);
}

void BulbApp::applyColor() {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "HSBColor%%20%d,%d,%d", hue, saturation, brightness);
    sendCommand(cmd);
}

void BulbApp::applyColorTemp() {
    // First set saturation to 0 for white mode
    sendCommand("HSBColor2%200");
    delay(100);

    char cmd[24];
    snprintf(cmd, sizeof(cmd), "CT%%20%d", colorTemp);
    sendCommand(cmd);
}

// Preset methods
void BulbApp::loadPreset(int idx) {
    if (idx < 0 || idx >= PRESET_COUNT || !presets[idx].valid) {
        return;
    }

    hue = presets[idx].hue;
    saturation = presets[idx].saturation;
    brightness = presets[idx].brightness;
    colorTemp = presets[idx].colorTemp;

    // Apply to bulb
    if (!power) {
        setPower(true);
        delay(100);
    }

    if (saturation > 0) {
        applyColor();
    } else {
        applyBrightness();
        delay(100);
        applyColorTemp();
    }
}

void BulbApp::savePreset(int idx) {
    if (idx < 0 || idx >= PRESET_COUNT) {
        return;
    }

    presets[idx].hue = hue;
    presets[idx].saturation = saturation;
    presets[idx].brightness = brightness;
    presets[idx].colorTemp = colorTemp;
    presets[idx].valid = true;

    // Generate name based on settings
    if (saturation < 20) {
        snprintf(presets[idx].name, sizeof(presets[idx].name), "%dK %d%%",
                 ctToKelvin(colorTemp), brightness);
    } else {
        snprintf(presets[idx].name, sizeof(presets[idx].name), "H%d S%d",
                 hue, saturation);
    }

    savePresets();
    UI::beep(1500, 50);
}

// Storage methods
void BulbApp::loadSettings() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    prefs.getString("bulb_ip", bulbIP, sizeof(bulbIP));
    prefs.end();
}

void BulbApp::saveSettings() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString("bulb_ip", bulbIP);
    prefs.end();
}

void BulbApp::loadPresets() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);

    for (int i = 0; i < PRESET_COUNT; i++) {
        char key[16];
        snprintf(key, sizeof(key), "bulb_pre_%d", i);

        presets[i].valid = prefs.getBool(key, false);
        if (presets[i].valid) {
            snprintf(key, sizeof(key), "bulb_pre_n%d", i);
            prefs.getString(key, presets[i].name, sizeof(presets[i].name));

            snprintf(key, sizeof(key), "bulb_pre_h%d", i);
            presets[i].hue = prefs.getInt(key, 0);

            snprintf(key, sizeof(key), "bulb_pre_s%d", i);
            presets[i].saturation = prefs.getInt(key, 0);

            snprintf(key, sizeof(key), "bulb_pre_b%d", i);
            presets[i].brightness = prefs.getInt(key, 100);

            snprintf(key, sizeof(key), "bulb_pre_c%d", i);
            presets[i].colorTemp = prefs.getInt(key, 326);
        }
    }

    prefs.end();
}

void BulbApp::savePresets() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);

    for (int i = 0; i < PRESET_COUNT; i++) {
        char key[16];
        snprintf(key, sizeof(key), "bulb_pre_%d", i);
        prefs.putBool(key, presets[i].valid);

        if (presets[i].valid) {
            snprintf(key, sizeof(key), "bulb_pre_n%d", i);
            prefs.putString(key, presets[i].name);

            snprintf(key, sizeof(key), "bulb_pre_h%d", i);
            prefs.putInt(key, presets[i].hue);

            snprintf(key, sizeof(key), "bulb_pre_s%d", i);
            prefs.putInt(key, presets[i].saturation);

            snprintf(key, sizeof(key), "bulb_pre_b%d", i);
            prefs.putInt(key, presets[i].brightness);

            snprintf(key, sizeof(key), "bulb_pre_c%d", i);
            prefs.putInt(key, presets[i].colorTemp);
        }
    }

    prefs.end();
}

// Helper methods
const char* BulbApp::getColorName() {
    if (saturation < 20) {
        if (colorTemp < 250) return "Cool White";
        if (colorTemp < 380) return "Neutral";
        return "Warm White";
    }

    // Color based on hue
    if (hue < 30) return "Red";
    if (hue < 60) return "Orange";
    if (hue < 90) return "Yellow";
    if (hue < 150) return "Green";
    if (hue < 210) return "Cyan";
    if (hue < 270) return "Blue";
    if (hue < 330) return "Purple";
    return "Red";
}

int BulbApp::ctToKelvin(int ct) {
    // CT in mireds to Kelvin: K = 1,000,000 / CT
    // Range: 153 (6536K) to 500 (2000K)
    if (ct <= 0) ct = 326;
    return 1000000 / ct;
}

const uint8_t* BulbApp::getIcon() {
    return icon_bulb;
}
