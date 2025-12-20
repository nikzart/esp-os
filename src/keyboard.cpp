#include "keyboard.h"
#include "ui.h"
#include "input.h"

bool Keyboard::active = false;
bool Keyboard::confirmed = false;
bool Keyboard::cancelled = false;
char* Keyboard::outputBuffer = nullptr;
int Keyboard::maxLength = 0;
char Keyboard::inputBuffer[64] = {0};
int Keyboard::inputLen = 0;
int Keyboard::cursorX = 0;
int Keyboard::cursorY = 0;
bool Keyboard::capsLock = false;
bool Keyboard::symbolMode = false;
const char* Keyboard::prompt = "";

// Keyboard layout
static const char* rows[] = {
    "1234567890",
    "qwertyuiop",
    "asdfghjkl",
    "zxcvbnm.-",
    " @_"  // space, @, underscore (special row)
};

static const char* symbolRows[] = {
    "!@#$%^&*()",
    "~`[]{}|\\;:",
    "'\",<>/?+=",
    "-_.*@#$%&",
    " @_"
};

static const int rowLengths[] = {10, 10, 9, 9, 3};
static const int numRows = 5;

void Keyboard::init() {
    active = false;
    confirmed = false;
    cancelled = false;
}

void Keyboard::show(const char* p, char* buffer, int maxLen) {
    active = true;
    confirmed = false;
    cancelled = false;
    prompt = p;
    outputBuffer = buffer;
    maxLength = maxLen;

    // Copy existing content if any
    strncpy(inputBuffer, buffer, sizeof(inputBuffer) - 1);
    inputLen = strlen(inputBuffer);

    cursorX = 0;
    cursorY = 1;  // Start on QWERTY row
    capsLock = false;
    symbolMode = false;
}

void Keyboard::hide() {
    active = false;
}

void Keyboard::update() {
    // Handle held buttons for repeat (optional)
}

void Keyboard::render() {
    if (!active) return;

    UI::clear();

    // Draw prompt and input
    UI::setSmallFont();
    u8g2.drawStr(2, 8, prompt);

    // Input box
    u8g2.drawFrame(0, 10, 128, 12);
    UI::setNormalFont();

    // Show input with cursor
    char display[32];
    snprintf(display, sizeof(display), "%s_", inputBuffer);
    u8g2.drawStr(2, 20, display);

    // Draw keyboard
    UI::setSmallFont();
    const char** layout = symbolMode ? symbolRows : rows;

    int startY = 26;
    int rowHeight = 8;

    for (int row = 0; row < numRows - 1; row++) {
        int y = startY + row * rowHeight;
        int len = rowLengths[row];
        int startX = (128 - len * 10) / 2;

        for (int col = 0; col < len; col++) {
            int x = startX + col * 10;
            char c = layout[row][col];

            if (capsLock && c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            }

            char str[2] = {c, '\0'};

            // Highlight selected
            if (row == cursorY && col == cursorX) {
                u8g2.drawBox(x - 1, y - 6, 9, 8);
                u8g2.setDrawColor(0);
                u8g2.drawStr(x, y, str);
                u8g2.setDrawColor(1);
            } else {
                u8g2.drawStr(x, y, str);
            }
        }
    }

    // Bottom row: CAPS, SPACE, OK
    int y = startY + 4 * rowHeight;
    const char* labels[] = {"CAP", "SPC", "OK"};
    int widths[] = {20, 40, 20};
    int positions[] = {10, 44, 98};

    for (int i = 0; i < 3; i++) {
        int x = positions[i];
        bool selected = (cursorY == 4 && cursorX == i);

        if (selected) {
            u8g2.drawBox(x - 2, y - 6, widths[i], 8);
            u8g2.setDrawColor(0);
        }

        if (i == 0 && capsLock) {
            u8g2.drawStr(x, y, "cap");
        } else if (i == 0 && symbolMode) {
            u8g2.drawStr(x, y, "123");
        } else {
            u8g2.drawStr(x, y, labels[i]);
        }

        if (selected) u8g2.setDrawColor(1);
    }

    // Instructions
    u8g2.drawStr(2, 63, "A:type B:del C:mode D:cancel");

    UI::flush();
}

void Keyboard::onButton(uint8_t btn, bool pressed) {
    if (!active || !pressed) return;

    UI::beep(3000, 20);

    switch (btn) {
        case BTN_UP:
            cursorY = (cursorY - 1 + numRows) % numRows;
            if (cursorX >= rowLengths[cursorY]) {
                cursorX = rowLengths[cursorY] - 1;
            }
            break;

        case BTN_DOWN:
            cursorY = (cursorY + 1) % numRows;
            if (cursorX >= rowLengths[cursorY]) {
                cursorX = rowLengths[cursorY] - 1;
            }
            break;

        case BTN_LEFT:
            cursorX = (cursorX - 1 + rowLengths[cursorY]) % rowLengths[cursorY];
            break;

        case BTN_RIGHT:
            cursorX = (cursorX + 1) % rowLengths[cursorY];
            break;

        case BTN_A:  // Type character or select action
            if (cursorY == 4) {
                // Bottom row actions
                if (cursorX == 0) {
                    // Toggle caps/symbol
                    if (symbolMode) {
                        symbolMode = false;
                    } else if (capsLock) {
                        capsLock = false;
                        symbolMode = true;
                    } else {
                        capsLock = true;
                    }
                } else if (cursorX == 1) {
                    // Space
                    typeChar(' ');
                } else if (cursorX == 2) {
                    // OK - confirm
                    strncpy(outputBuffer, inputBuffer, maxLength);
                    confirmed = true;
                    active = false;
                }
            } else {
                // Type character
                char c = getChar(cursorX, cursorY);
                typeChar(c);
            }
            break;

        case BTN_B:  // Backspace
            backspace();
            break;

        case BTN_C:  // Toggle mode
            symbolMode = !symbolMode;
            if (symbolMode) capsLock = false;
            break;

        case BTN_D:  // Cancel
            cancelled = true;
            active = false;
            break;
    }
}

bool Keyboard::isActive() {
    return active;
}

bool Keyboard::isConfirmed() {
    return confirmed;
}

bool Keyboard::isCancelled() {
    return cancelled;
}

const char* Keyboard::getText() {
    return inputBuffer;
}

char Keyboard::getChar(int x, int y) {
    if (y >= numRows - 1 || x >= rowLengths[y]) return ' ';

    const char** layout = symbolMode ? symbolRows : rows;
    char c = layout[y][x];

    if (capsLock && c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    }

    return c;
}

void Keyboard::typeChar(char c) {
    if (inputLen < (int)sizeof(inputBuffer) - 1 && inputLen < maxLength - 1) {
        inputBuffer[inputLen++] = c;
        inputBuffer[inputLen] = '\0';
    }
}

void Keyboard::backspace() {
    if (inputLen > 0) {
        inputBuffer[--inputLen] = '\0';
    }
}
