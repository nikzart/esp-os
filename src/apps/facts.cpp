#include "apps/facts.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>

void FactsApp::init() {
    hasData = false;
    loading = false;
    errorMsg[0] = '\0';
}

void FactsApp::update() {
    // No auto-refresh
}

void FactsApp::fetchFact() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    loading = true;

    // Useless facts API
    String response = WiFiManager::httpGet("https://uselessfacts.jsph.pl/api/v2/facts/random?language=en");
    loading = false;

    if (response.length() == 0) {
        strcpy(errorMsg, "Network error");
        return;
    }

    // Parse JSON
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        strcpy(errorMsg, "Parse error");
        return;
    }

    const char* text = doc["text"] | "";
    strncpy(fact, text, sizeof(fact) - 1);

    hasData = strlen(fact) > 0;
    if (!hasData) {
        strcpy(errorMsg, "No fact received");
    } else {
        errorMsg[0] = '\0';
    }
}

void FactsApp::render() {
    UI::clear();
    UI::drawTitleBar("Random Fact");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to retry");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(35, "Press A for a fact");
    } else {
        UI::setSmallFont();
        UI::drawTextWrapped(2, 20, 124, fact);
        UI::setNormalFont();
    }

    UI::drawStatusBar("A:Next", "B:Back");
    UI::flush();
}

void FactsApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_A || btn == BTN_C) {
        fetchFact();
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* FactsApp::getIcon() {
    return icon_facts;
}
