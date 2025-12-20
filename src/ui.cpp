#include "ui.h"
#include <Wire.h>
#include <Preferences.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

static uint8_t currentBrightness = 255;

namespace UI {

void init() {
    Wire.begin(I2C_SDA, I2C_SCL);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    pinMode(BUZZER_PIN, OUTPUT);
}

void clear() {
    u8g2.clearBuffer();
}

void flush() {
    u8g2.sendBuffer();
}

void drawTextWrapped(int x, int y, int maxWidth, const char* text) {
    char line[64];
    int lineIdx = 0;
    int startX = x;
    int lineHeight = 10;

    for (int i = 0; text[i] != '\0'; i++) {
        line[lineIdx++] = text[i];
        line[lineIdx] = '\0';

        int w = u8g2.getStrWidth(line);

        if (w > maxWidth || text[i] == '\n') {
            // Find last space for word wrap
            if (text[i] != '\n' && text[i] != ' ') {
                int lastSpace = -1;
                for (int j = lineIdx - 1; j >= 0; j--) {
                    if (line[j] == ' ') {
                        lastSpace = j;
                        break;
                    }
                }
                if (lastSpace > 0) {
                    line[lastSpace] = '\0';
                    i -= (lineIdx - lastSpace - 1);
                }
            } else {
                line[lineIdx - 1] = '\0';
            }

            u8g2.drawStr(startX, y, line);
            y += lineHeight;
            lineIdx = 0;

            if (y > SCREEN_HEIGHT - lineHeight) break;
        }
    }

    if (lineIdx > 0) {
        u8g2.drawStr(startX, y, line);
    }
}

void drawCentered(int y, const char* text) {
    int w = u8g2.getStrWidth(text);
    u8g2.drawStr((SCREEN_WIDTH - w) / 2, y, text);
}

void drawTitleBar(const char* title) {
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, SCREEN_WIDTH, 11);
    u8g2.setDrawColor(0);
    int w = u8g2.getStrWidth(title);
    u8g2.drawStr((SCREEN_WIDTH - w) / 2, 9, title);
    u8g2.setDrawColor(1);
}

void drawStatusBar(const char* left, const char* right) {
    u8g2.drawLine(0, 54, SCREEN_WIDTH - 1, 54);
    if (left) u8g2.drawStr(2, 63, left);
    if (right) {
        int w = u8g2.getStrWidth(right);
        u8g2.drawStr(SCREEN_WIDTH - w - 2, 63, right);
    }
}

void drawMenuItem(int y, const char* text, bool selected) {
    if (selected) {
        u8g2.drawBox(0, y - 9, SCREEN_WIDTH, 11);
        u8g2.setDrawColor(0);
        u8g2.drawStr(4, y, text);
        u8g2.setDrawColor(1);
    } else {
        u8g2.drawStr(4, y, text);
    }
}

void drawProgressBar(int x, int y, int w, int h, int percent) {
    u8g2.drawFrame(x, y, w, h);
    int fillW = (w - 2) * percent / 100;
    if (fillW > 0) {
        u8g2.drawBox(x + 1, y + 1, fillW, h - 2);
    }
}

void drawIcon(int x, int y, const uint8_t* icon) {
    u8g2.drawXBM(x, y, 16, 16, icon);
}

void drawSelectionBox(int x, int y, int w, int h) {
    u8g2.drawFrame(x - 1, y - 1, w + 2, h + 2);
}

void drawScrollbar(int x, int y, int h, int pos, int total, int visible) {
    if (total <= visible) return;

    u8g2.drawVLine(x, y, h);

    int thumbH = max(4, h * visible / total);
    int thumbY = y + (h - thumbH) * pos / (total - visible);

    u8g2.drawBox(x - 1, thumbY, 3, thumbH);
}

void beep(int freq, int duration) {
    tone(BUZZER_PIN, freq, duration);
}

void drawLoading(int x, int y, int frame) {
    const char* spinner = "|/-\\";
    char c[2] = {spinner[frame % 4], '\0'};
    u8g2.drawStr(x, y, c);
}

int getTextWidth(const char* text) {
    return u8g2.getStrWidth(text);
}

void setFont(const uint8_t* font) {
    u8g2.setFont(font);
}

void setSmallFont() {
    u8g2.setFont(u8g2_font_5x7_tf);
}

void setNormalFont() {
    u8g2.setFont(u8g2_font_6x10_tf);
}

void setLargeFont() {
    u8g2.setFont(u8g2_font_7x14_tf);
}

void setBrightness(uint8_t level) {
    currentBrightness = level;
    u8g2.setContrast(level);
}

uint8_t getBrightness() {
    return currentBrightness;
}

void loadBrightness() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    currentBrightness = prefs.getUChar("brightness", 255);
    prefs.end();
    u8g2.setContrast(currentBrightness);
}

void saveBrightness() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putUChar("brightness", currentBrightness);
    prefs.end();
}

}
