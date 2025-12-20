#include "apps/news.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>
#include <Preferences.h>

void NewsApp::init() {
    hasData = false;
    loading = false;
    errorMsg[0] = '\0';
    currentIndex = 0;
    headlineCount = 0;
}

void NewsApp::update() {
    // Auto-refresh every 10 minutes
    if (hasData && millis() - lastFetch > 600000) {
        fetchNews();
    }
}

void NewsApp::fetchNews() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    // Get API key (use default from config if NVS is empty)
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    char apiKey[48] = {0};
    prefs.getString("news_key", apiKey, sizeof(apiKey));
    prefs.end();

    if (strlen(apiKey) == 0) {
        // Use default from config.h
        strncpy(apiKey, NEWS_API_KEY, sizeof(apiKey) - 1);
    }

    if (strlen(apiKey) == 0) {
        strcpy(errorMsg, "No API key");
        return;
    }

    loading = true;

    char url[256];
    snprintf(url, sizeof(url),
        "https://newsapi.org/v2/top-headlines?country=us&pageSize=%d&apiKey=%s",
        MAX_HEADLINES, apiKey);

    String response = WiFiManager::httpGet(url);
    loading = false;

    if (response.length() == 0) {
        strcpy(errorMsg, "Network error");
        return;
    }

    // Parse JSON
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        strcpy(errorMsg, "Parse error");
        return;
    }

    if (doc["status"] != "ok") {
        strcpy(errorMsg, "API error");
        return;
    }

    // Extract headlines
    JsonArray articles = doc["articles"];
    headlineCount = 0;

    for (JsonObject article : articles) {
        if (headlineCount >= MAX_HEADLINES) break;

        const char* title = article["title"] | "";
        const char* source = article["source"]["name"] | "";

        strncpy(headlines[headlineCount], title, sizeof(headlines[0]) - 1);
        strncpy(sources[headlineCount], source, sizeof(sources[0]) - 1);
        headlineCount++;
    }

    hasData = headlineCount > 0;
    if (!hasData) {
        strcpy(errorMsg, "No headlines");
    } else {
        errorMsg[0] = '\0';
    }

    currentIndex = 0;
    lastFetch = millis();
}

void NewsApp::render() {
    UI::clear();
    UI::drawTitleBar("News");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to fetch");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(35, "Press A to fetch");
    } else {
        // Show current headline
        UI::setSmallFont();

        // Source
        u8g2.drawStr(2, 20, sources[currentIndex]);

        // Headline (wrapped)
        UI::drawTextWrapped(2, 30, 124, headlines[currentIndex]);

        // Navigation indicator
        char nav[16];
        snprintf(nav, sizeof(nav), "%d/%d", currentIndex + 1, headlineCount);
        u8g2.drawStr(55, 52, nav);

        UI::setNormalFont();
    }

    UI::drawStatusBar("U/D:Scroll", "B:Back");
    UI::flush();
}

void NewsApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    switch (btn) {
        case BTN_UP:
            if (hasData && currentIndex > 0) {
                currentIndex--;
                UI::beep(2500, 20);
            }
            break;

        case BTN_DOWN:
            if (hasData && currentIndex < headlineCount - 1) {
                currentIndex++;
                UI::beep(2500, 20);
            }
            break;

        case BTN_A:
        case BTN_C:
            fetchNews();
            UI::beep();
            break;

        case BTN_B:
        case BTN_D:
            wantsToExit = true;
            break;
    }
}

const uint8_t* NewsApp::getIcon() {
    return icon_news;
}
