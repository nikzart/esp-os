#include "apps/launcher.h"
#include "ui.h"
#include "icons.h"
#include <WiFi.h>

void LauncherApp::init() {
    selectedIndex = 0;
    scrollOffset = 0;
}

void LauncherApp::setApps(App** apps, int count) {
    appList = apps;
    appCount = count;
}

int LauncherApp::getSelectedApp() {
    return selectedIndex;
}

void LauncherApp::update() {
    // Nothing to update
}

void LauncherApp::render() {
    UI::clear();

    // Title
    UI::setSmallFont();
    u8g2.drawStr(2, 7, "ESP32 OS");

    // WiFi indicator
    if (WiFi.status() == WL_CONNECTED) {
        u8g2.drawStr(100, 7, "WiFi");
    }

    u8g2.drawLine(0, 9, 127, 9);

    // Draw app grid
    UI::setSmallFont();
    int startY = 12;

    int totalRows = (appCount + COLS - 1) / COLS;
    int maxScroll = max(0, totalRows - VISIBLE_ROWS);

    // Ensure selected is visible
    int selectedRow = selectedIndex / COLS;
    if (selectedRow < scrollOffset) {
        scrollOffset = selectedRow;
    } else if (selectedRow >= scrollOffset + VISIBLE_ROWS) {
        scrollOffset = selectedRow - VISIBLE_ROWS + 1;
    }

    for (int row = 0; row < VISIBLE_ROWS; row++) {
        int actualRow = row + scrollOffset;
        for (int col = 0; col < COLS; col++) {
            int index = actualRow * COLS + col;
            if (index < appCount) {
                drawAppIcon(index, col, row, index == selectedIndex);
            }
        }
    }

    // Draw selected app name at bottom
    if (selectedIndex < appCount && appList[selectedIndex]) {
        u8g2.drawLine(0, 54, 127, 54);
        UI::drawCentered(63, appList[selectedIndex]->getName());
    }

    // Scroll indicator
    if (totalRows > VISIBLE_ROWS) {
        int scrollbarH = 40;
        int thumbH = scrollbarH / totalRows;
        int thumbY = 12 + (scrollbarH - thumbH) * scrollOffset / maxScroll;
        u8g2.drawVLine(126, 12, scrollbarH);
        u8g2.drawBox(125, thumbY, 3, max(4, thumbH));
    }

    UI::flush();
}

void LauncherApp::drawAppIcon(int index, int gridX, int gridY, bool selected) {
    int x = gridX * CELL_WIDTH + (CELL_WIDTH - ICON_SIZE) / 2;
    int y = 12 + gridY * CELL_HEIGHT;

    // Draw icon
    if (appList && appList[index] && appList[index]->getIcon()) {
        UI::drawIcon(x, y, appList[index]->getIcon());
    } else {
        // Default icon (simple box)
        u8g2.drawFrame(x, y, ICON_SIZE, ICON_SIZE);
        // Draw first letter of app name
        if (appList && appList[index]) {
            char c[2] = {appList[index]->getName()[0], '\0'};
            u8g2.drawStr(x + 5, y + 12, c);
        }
    }

    // Selection box
    if (selected) {
        u8g2.drawFrame(x - 2, y - 2, ICON_SIZE + 4, ICON_SIZE + 4);
    }
}

void LauncherApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    UI::beep(2500, 30);

    int row = selectedIndex / COLS;
    int col = selectedIndex % COLS;
    int totalRows = (appCount + COLS - 1) / COLS;

    switch (btn) {
        case BTN_UP:
            if (row > 0) {
                selectedIndex -= COLS;
            }
            break;

        case BTN_DOWN:
            if (row < totalRows - 1 && selectedIndex + COLS < appCount) {
                selectedIndex += COLS;
            }
            break;

        case BTN_LEFT:
            if (col > 0) {
                selectedIndex--;
            }
            break;

        case BTN_RIGHT:
            if (col < COLS - 1 && selectedIndex + 1 < appCount) {
                selectedIndex++;
            }
            break;

        case BTN_A:
            // Launch app - handled by main loop
            wantsToExit = true;  // Signal to launch selected app
            break;
    }
}
