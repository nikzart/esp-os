#include "apps/iss.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>

void ISSApp::init() {
    hasData = false;
    loading = false;
    errorMsg[0] = '\0';
    astronautCount = 0;
}

void ISSApp::update() {
    // Auto-refresh every 30 seconds
    if (hasData && millis() - lastFetch > 30000) {
        fetchISS();
    }
}

void ISSApp::fetchISS() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    loading = true;

    // ISS Location API
    String response = WiFiManager::httpGet("http://api.open-notify.org/iss-now.json");

    if (response.length() == 0) {
        loading = false;
        strcpy(errorMsg, "Network error");
        return;
    }

    // Parse JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        loading = false;
        strcpy(errorMsg, "Parse error");
        return;
    }

    latitude = doc["iss_position"]["latitude"].as<float>();
    longitude = doc["iss_position"]["longitude"].as<float>();

    hasData = true;
    errorMsg[0] = '\0';
    lastFetch = millis();

    // Also fetch astronauts (only once)
    if (astronautCount == 0) {
        fetchAstronauts();
    }

    loading = false;
}

void ISSApp::fetchAstronauts() {
    String response = WiFiManager::httpGet("http://api.open-notify.org/astros.json");

    if (response.length() == 0) return;

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);

    if (error) return;

    astronautCount = doc["number"] | 0;

    // Get first few names
    JsonArray people = doc["people"];
    int idx = 0;
    for (JsonObject person : people) {
        if (idx >= 6) break;
        const char* name = person["name"] | "";
        strncpy(astronauts[idx], name, sizeof(astronauts[0]) - 1);
        idx++;
    }
}

void ISSApp::render() {
    UI::clear();
    UI::drawTitleBar("ISS Tracker");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to retry");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(35, "Press A to track");
    } else {
        char buf[48];

        // Location
        snprintf(buf, sizeof(buf), "Lat: %.2f", latitude);
        u8g2.drawStr(4, 22, buf);

        snprintf(buf, sizeof(buf), "Lon: %.2f", longitude);
        u8g2.drawStr(4, 32, buf);

        // Astronaut count
        snprintf(buf, sizeof(buf), "Astronauts: %d", astronautCount);
        u8g2.drawStr(4, 44, buf);

        // Last updated
        UI::setSmallFont();
        unsigned long ago = (millis() - lastFetch) / 1000;
        snprintf(buf, sizeof(buf), "Updated %lus ago", ago);
        u8g2.drawStr(4, 52, buf);
        UI::setNormalFont();
    }

    UI::drawStatusBar("A:Refresh", "B:Back");
    UI::flush();
}

void ISSApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_A || btn == BTN_C) {
        fetchISS();
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* ISSApp::getIcon() {
    return icon_iss;
}
