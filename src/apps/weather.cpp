#include "apps/weather.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include "config.h"
#include "keyboard.h"
#include <ArduinoJson.h>
#include <Preferences.h>

void WeatherApp::init() {
    hasData = false;
    loading = false;
    searching = false;
    errorMsg[0] = '\0';
    loadCity();
}

void WeatherApp::loadCity() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    prefs.getString("weather_city", city, sizeof(city));
    if (strlen(city) == 0) strcpy(city, DEFAULT_CITY);
    prefs.end();
}

void WeatherApp::saveCity() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString("weather_city", city);
    prefs.end();
}

void WeatherApp::update() {
    // Check for keyboard input completion
    if (searching) {
        if (Keyboard::isConfirmed()) {
            strncpy(city, Keyboard::getText(), sizeof(city) - 1);
            city[sizeof(city) - 1] = '\0';
            saveCity();
            searching = false;
            hasData = false;
            fetchWeather();
        } else if (Keyboard::isCancelled()) {
            searching = false;
        }
        return;
    }

    // Auto-refresh every 5 minutes
    if (hasData && millis() - lastFetch > 300000) {
        fetchWeather();
    }
}

void WeatherApp::fetchWeather() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    // Get API key (use default from config if NVS is empty)
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    char apiKey[48] = {0};
    prefs.getString("weather_key", apiKey, sizeof(apiKey));
    prefs.end();

    if (strlen(apiKey) == 0) {
        // Use default from config.h
        strncpy(apiKey, OPENWEATHER_API_KEY, sizeof(apiKey) - 1);
    }

    if (strlen(apiKey) == 0) {
        strcpy(errorMsg, "No API key");
        return;
    }

    loading = true;

    char url[256];
    snprintf(url, sizeof(url),
        "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric",
        city, apiKey);

    String response = WiFiManager::httpGet(url);
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

    if (doc.containsKey("cod") && doc["cod"] != 200) {
        strcpy(errorMsg, "City not found");
        return;
    }

    // Extract data
    temp = doc["main"]["temp"] | 0.0f;
    humidity = doc["main"]["humidity"] | 0;
    const char* desc = doc["weather"][0]["description"] | "";
    strncpy(description, desc, sizeof(description) - 1);

    hasData = true;
    errorMsg[0] = '\0';
    lastFetch = millis();
}

void WeatherApp::render() {
    UI::clear();
    UI::drawTitleBar("Weather");

    if (loading) {
        UI::drawCentered(35, "Loading...");
    } else if (strlen(errorMsg) > 0) {
        UI::drawCentered(30, errorMsg);
        UI::setSmallFont();
        UI::drawCentered(45, "Press A to retry");
        UI::setNormalFont();
    } else if (!hasData) {
        UI::drawCentered(30, "Press A to fetch");
        UI::setSmallFont();
        UI::drawCentered(45, city);
        UI::setNormalFont();
    } else {
        // Show weather data
        char buf[32];

        // City
        UI::drawCentered(22, city);

        // Large temperature
        UI::setLargeFont();
        snprintf(buf, sizeof(buf), "%.1f C", temp);
        UI::drawCentered(38, buf);

        UI::setNormalFont();

        // Humidity
        snprintf(buf, sizeof(buf), "Humidity: %d%%", humidity);
        UI::drawCentered(50, buf);
    }

    UI::drawStatusBar("A:Fetch C:City", "B:Back");
    UI::flush();
}

void WeatherApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_A) {
        fetchWeather();
        UI::beep();
    } else if (btn == BTN_C) {
        // Open keyboard to search city
        searching = true;
        strcpy(searchBuffer, city);
        Keyboard::show("Enter city:", searchBuffer, sizeof(searchBuffer));
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* WeatherApp::getIcon() {
    return icon_weather;
}
