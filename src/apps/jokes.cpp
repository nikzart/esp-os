#include "apps/jokes.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>

void JokesApp::init() {
    hasData = false;
    loading = false;
    showPunchline = false;
    errorMsg[0] = '\0';
}

void JokesApp::update() {
    // No auto-refresh
}

void JokesApp::fetchJoke() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    loading = true;
    showPunchline = false;

    // JokeAPI
    String response = WiFiManager::httpGet("https://v2.jokeapi.dev/joke/Any?safe-mode");
    loading = false;

    if (response.length() == 0) {
        strcpy(errorMsg, "Network error");
        return;
    }

    // Parse JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        strcpy(errorMsg, "Parse error");
        return;
    }

    if (doc.containsKey("error") && doc["error"] == true) {
        strcpy(errorMsg, "API error");
        return;
    }

    const char* type = doc["type"] | "single";
    isSingleJoke = (strcmp(type, "single") == 0);

    if (isSingleJoke) {
        const char* joke = doc["joke"] | "";
        strncpy(setup, joke, sizeof(setup) - 1);
        delivery[0] = '\0';
    } else {
        const char* s = doc["setup"] | "";
        const char* d = doc["delivery"] | "";
        strncpy(setup, s, sizeof(setup) - 1);
        strncpy(delivery, d, sizeof(delivery) - 1);
    }

    hasData = strlen(setup) > 0;
    if (!hasData) {
        strcpy(errorMsg, "No joke received");
    } else {
        errorMsg[0] = '\0';
    }
}

void JokesApp::render() {
    UI::clear();
    UI::drawTitleBar("Jokes");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to retry");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(35, "Press A for a joke");
    } else {
        UI::setSmallFont();

        if (isSingleJoke) {
            // Single joke - show it all
            UI::drawTextWrapped(2, 20, 124, setup);
        } else {
            // Two-part joke
            UI::drawTextWrapped(2, 20, 124, setup);

            if (showPunchline) {
                // Draw punchline
                u8g2.drawLine(0, 38, 127, 38);
                UI::drawTextWrapped(2, 46, 124, delivery);
            } else {
                UI::drawCentered(52, "[Press A for punchline]");
            }
        }

        UI::setNormalFont();
    }

    UI::drawStatusBar(hasData && !isSingleJoke && !showPunchline ? "A:Reveal" : "A:Next", "B:Back");
    UI::flush();
}

void JokesApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_A) {
        if (hasData && !isSingleJoke && !showPunchline) {
            // Reveal punchline
            showPunchline = true;
            UI::beep(1500, 50);
        } else {
            // Get new joke
            fetchJoke();
            UI::beep();
        }
    } else if (btn == BTN_C) {
        fetchJoke();
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* JokesApp::getIcon() {
    return icon_jokes;
}
