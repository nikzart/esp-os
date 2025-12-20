#include "apps/crypto.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>

void CryptoApp::init() {
    hasData = false;
    loading = false;
    errorMsg[0] = '\0';
}

void CryptoApp::update() {
    // Auto-refresh every 60 seconds
    if (hasData && millis() - lastFetch > 60000) {
        fetchPrices();
    }
}

void CryptoApp::fetchPrices() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    loading = true;

    // CoinGecko free API
    const char* url = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum,solana&vs_currencies=usd&include_24hr_change=true";

    String response = WiFiManager::httpGet(url);
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

    btcPrice = doc["bitcoin"]["usd"] | 0.0f;
    btcChange = doc["bitcoin"]["usd_24h_change"] | 0.0f;
    ethPrice = doc["ethereum"]["usd"] | 0.0f;
    ethChange = doc["ethereum"]["usd_24h_change"] | 0.0f;
    solPrice = doc["solana"]["usd"] | 0.0f;
    solChange = doc["solana"]["usd_24h_change"] | 0.0f;

    hasData = true;
    errorMsg[0] = '\0';
    lastFetch = millis();
}

void CryptoApp::render() {
    UI::clear();
    UI::drawTitleBar("Crypto Prices");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to retry");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(35, "Press A to fetch");
    } else {
        char buf[48];
        int y = 22;

        // BTC
        const char* btcArrow = btcChange >= 0 ? "+" : "";
        snprintf(buf, sizeof(buf), "BTC $%.0f %s%.1f%%", btcPrice, btcArrow, btcChange);
        u8g2.drawStr(4, y, buf);
        y += 12;

        // ETH
        const char* ethArrow = ethChange >= 0 ? "+" : "";
        snprintf(buf, sizeof(buf), "ETH $%.0f %s%.1f%%", ethPrice, ethArrow, ethChange);
        u8g2.drawStr(4, y, buf);
        y += 12;

        // SOL
        const char* solArrow = solChange >= 0 ? "+" : "";
        snprintf(buf, sizeof(buf), "SOL $%.1f %s%.1f%%", solPrice, solArrow, solChange);
        u8g2.drawStr(4, y, buf);

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

void CryptoApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_A || btn == BTN_C) {
        fetchPrices();
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* CryptoApp::getIcon() {
    return icon_crypto;
}
