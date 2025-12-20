#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "config.h"

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

namespace UI {
    // Initialize display
    void init();

    // Clear and prepare buffer
    void clear();

    // Send buffer to display
    void flush();

    // Draw text with word wrap
    void drawTextWrapped(int x, int y, int maxWidth, const char* text);

    // Draw centered text
    void drawCentered(int y, const char* text);

    // Draw title bar
    void drawTitleBar(const char* title);

    // Draw status bar (bottom)
    void drawStatusBar(const char* left, const char* right);

    // Draw menu item
    void drawMenuItem(int y, const char* text, bool selected);

    // Draw progress bar
    void drawProgressBar(int x, int y, int w, int h, int percent);

    // Draw icon (16x16 XBM)
    void drawIcon(int x, int y, const uint8_t* icon);

    // Draw selection box
    void drawSelectionBox(int x, int y, int w, int h);

    // Draw scrollbar
    void drawScrollbar(int x, int y, int h, int pos, int total, int visible);

    // Play beep sound
    void beep(int freq = BEEP_FREQ, int duration = BEEP_DURATION);

    // Draw loading spinner
    void drawLoading(int x, int y, int frame);

    // Get text width
    int getTextWidth(const char* text);

    // Set font
    void setFont(const uint8_t* font);
    void setSmallFont();
    void setNormalFont();
    void setLargeFont();

    // Brightness control (0-255)
    void setBrightness(uint8_t level);
    uint8_t getBrightness();
    void loadBrightness();
    void saveBrightness();
}

#endif
