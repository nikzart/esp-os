#include "apps/quotes.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>

void QuotesApp::init() {
    hasData = false;
    loading = false;
    errorMsg[0] = '\0';
}

void QuotesApp::update() {
    // No auto-refresh
}

void QuotesApp::fetchQuote() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    loading = true;

    // Quotable API
    String response = WiFiManager::httpGet("https://api.quotable.io/random?maxLength=150");
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

    const char* content = doc["content"] | "";
    const char* auth = doc["author"] | "Unknown";

    strncpy(quote, content, sizeof(quote) - 1);
    strncpy(author, auth, sizeof(author) - 1);

    hasData = strlen(quote) > 0;
    if (!hasData) {
        strcpy(errorMsg, "No quote received");
    } else {
        errorMsg[0] = '\0';
    }
}

void QuotesApp::render() {
    UI::clear();
    UI::drawTitleBar("Quotes");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to retry");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(35, "Press A for a quote");
    } else {
        UI::setSmallFont();

        // Quote with quotes
        char quotedText[210];
        snprintf(quotedText, sizeof(quotedText), "\"%s\"", quote);
        UI::drawTextWrapped(2, 18, 124, quotedText);

        // Author at bottom
        char authorLine[56];
        snprintf(authorLine, sizeof(authorLine), "- %s", author);
        u8g2.drawStr(4, 52, authorLine);

        UI::setNormalFont();
    }

    UI::drawStatusBar("A:Next", "B:Back");
    UI::flush();
}

void QuotesApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_A || btn == BTN_C) {
        fetchQuote();
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* QuotesApp::getIcon() {
    return icon_quotes;
}
