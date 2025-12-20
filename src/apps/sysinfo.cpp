#include "apps/sysinfo.h"
#include "ui.h"
#include "input.h"
#include "wifi_manager.h"
#include "icons.h"

void SysInfoApp::init() {
    currentPage = Page::INFO;
}

void SysInfoApp::update() {
    // Nothing to update periodically
}

void SysInfoApp::render() {
    UI::clear();

    if (currentPage == Page::INFO) {
        renderInfo();
    } else {
        renderButtonTest();
    }

    UI::drawStatusBar("C:Toggle", "B:Back");
    UI::flush();
}

void SysInfoApp::renderInfo() {
    UI::drawTitleBar("System Info");
    UI::setSmallFont();

    int y = 22;
    char buf[48];

    // WiFi Status
    if (WiFiManager::isConnected()) {
        snprintf(buf, sizeof(buf), "WiFi: %s", WiFiManager::getSSID());
        u8g2.drawStr(2, y, buf);
        y += 9;

        IPAddress ip = WiFiManager::getIP();
        snprintf(buf, sizeof(buf), "IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        u8g2.drawStr(2, y, buf);
        y += 9;

        snprintf(buf, sizeof(buf), "Signal: %d dBm", WiFiManager::getRSSI());
        u8g2.drawStr(2, y, buf);
        y += 9;
    } else {
        u8g2.drawStr(2, y, "WiFi: Not connected");
        y += 9;
    }

    // Memory
    snprintf(buf, sizeof(buf), "Free Heap: %d KB", ESP.getFreeHeap() / 1024);
    u8g2.drawStr(2, y, buf);
    y += 9;

    // Uptime
    unsigned long uptime = millis() / 1000;
    int hours = uptime / 3600;
    int mins = (uptime % 3600) / 60;
    int secs = uptime % 60;
    snprintf(buf, sizeof(buf), "Uptime: %02d:%02d:%02d", hours, mins, secs);
    u8g2.drawStr(2, y, buf);

    UI::setNormalFont();
}

void SysInfoApp::renderButtonTest() {
    UI::drawTitleBar("Button Test");

    // D-pad
    int cx = 32, cy = 35;

    // UP
    if (Input::isPressed(BTN_UP)) {
        u8g2.drawBox(cx - 5, cy - 18, 10, 10);
    } else {
        u8g2.drawFrame(cx - 5, cy - 18, 10, 10);
    }

    // DOWN
    if (Input::isPressed(BTN_DOWN)) {
        u8g2.drawBox(cx - 5, cy + 8, 10, 10);
    } else {
        u8g2.drawFrame(cx - 5, cy + 8, 10, 10);
    }

    // LEFT
    if (Input::isPressed(BTN_LEFT)) {
        u8g2.drawBox(cx - 18, cy - 5, 10, 10);
    } else {
        u8g2.drawFrame(cx - 18, cy - 5, 10, 10);
    }

    // RIGHT
    if (Input::isPressed(BTN_RIGHT)) {
        u8g2.drawBox(cx + 8, cy - 5, 10, 10);
    } else {
        u8g2.drawFrame(cx + 8, cy - 5, 10, 10);
    }

    // Action buttons
    const char* labels[] = {"A", "B", "C", "D"};
    int ax = 88, ay = 24;

    for (int i = 0; i < 4; i++) {
        int x = ax + (i % 2) * 20;
        int y = ay + (i / 2) * 20;

        if (Input::isPressed(BTN_A + i)) {
            u8g2.drawDisc(x, y, 8);
            u8g2.setDrawColor(0);
            u8g2.drawStr(x - 3, y + 4, labels[i]);
            u8g2.setDrawColor(1);
        } else {
            u8g2.drawCircle(x, y, 8);
            u8g2.drawStr(x - 3, y + 4, labels[i]);
        }
    }
}

void SysInfoApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    if (btn == BTN_C) {
        currentPage = (currentPage == Page::INFO) ? Page::BUTTON_TEST : Page::INFO;
        UI::beep();
    } else if (btn == BTN_B || btn == BTN_D) {
        wantsToExit = true;
    }
}

const uint8_t* SysInfoApp::getIcon() {
    return icon_system;
}
