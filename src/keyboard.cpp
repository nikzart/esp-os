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
bool Keyboard::inActionColumn = false;
const char* Keyboard::prompt = "";

// Keyboard layout - 4 rows, actions on right side
static const char* rows[] = {
    "1234567890",
    "qwertyuiop",
    "asdfghjkl",
    "zxcvbnm._"
};

static const char* symbolRows[] = {
    "!@#$%^&*()",
    "~`[]{}|\\;:",
    "'\",<>/?+=",
    "-_.*@#$%&"
};

static const int rowLengths[] = {10, 10, 9, 9};
static const int numRows = 4;

// Right column action labels
static const char* actionLabels[] = {"CAP", "SPC", "OK", "DEL"};

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
    inActionColumn = false;
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
    u8g2.drawFrame(0, 10, 100, 12);
    UI::setNormalFont();

    // Show input with cursor
    char display[32];
    snprintf(display, sizeof(display), "%s_", inputBuffer);
    u8g2.drawStr(2, 20, display);

    // Draw keyboard
    UI::setSmallFont();
    const char** layout = symbolMode ? symbolRows : rows;

    int startY = 26;
    int rowHeight = 9;
    int charStartX = 4;  // Left margin for characters

    // Draw character rows
    for (int row = 0; row < numRows; row++) {
        int y = startY + row * rowHeight;
        int len = rowLengths[row];

        for (int col = 0; col < len; col++) {
            int x = charStartX + col * 10;
            char c = layout[row][col];

            if (capsLock && c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            }

            char str[2] = {c, '\0'};

            // Highlight selected character
            if (!inActionColumn && row == cursorY && col == cursorX) {
                u8g2.drawBox(x - 1, y - 6, 9, 8);
                u8g2.setDrawColor(0);
                u8g2.drawStr(x, y, str);
                u8g2.setDrawColor(1);
            } else {
                u8g2.drawStr(x, y, str);
            }
        }
    }

    // Draw right action column
    int actionX = 106;
    int actionWidth = 20;

    for (int i = 0; i < numRows; i++) {
        int y = startY + i * rowHeight;
        bool selected = inActionColumn && (cursorY == i);

        // Draw action button
        if (selected) {
            u8g2.drawBox(actionX - 1, y - 6, actionWidth, 8);
            u8g2.setDrawColor(0);
        }

        // Show appropriate label
        const char* label = actionLabels[i];
        if (i == 0) {
            // CAP button - show mode
            if (symbolMode) {
                label = "ABC";
            } else if (capsLock) {
                label = "abc";
            }
        }

        u8g2.drawStr(actionX, y, label);

        if (selected) u8g2.setDrawColor(1);
    }

    // Draw separator line between chars and actions
    u8g2.drawVLine(102, startY - 6, numRows * rowHeight);

    // Status hints at bottom
    u8g2.drawStr(2, 63, "A:sel B:back C:mode D:exit");

    UI::flush();
}

void Keyboard::onButton(uint8_t btn, bool pressed) {
    if (!active || !pressed) return;

    UI::beep(3000, 20);

    switch (btn) {
        case BTN_UP:
            cursorY = (cursorY - 1 + numRows) % numRows;
            if (!inActionColumn && cursorX >= rowLengths[cursorY]) {
                cursorX = rowLengths[cursorY] - 1;
            }
            break;

        case BTN_DOWN:
            cursorY = (cursorY + 1) % numRows;
            if (!inActionColumn && cursorX >= rowLengths[cursorY]) {
                cursorX = rowLengths[cursorY] - 1;
            }
            break;

        case BTN_LEFT:
            if (inActionColumn) {
                // Exit action column, go to last char of current row
                inActionColumn = false;
                cursorX = rowLengths[cursorY] - 1;
            } else {
                // Navigate left in character area
                if (cursorX > 0) {
                    cursorX--;
                }
            }
            break;

        case BTN_RIGHT:
            if (inActionColumn) {
                // Wrap to first char of current row
                inActionColumn = false;
                cursorX = 0;
            } else {
                // Navigate right in character area
                if (cursorX < rowLengths[cursorY] - 1) {
                    cursorX++;
                } else {
                    // Enter action column
                    inActionColumn = true;
                }
            }
            break;

        case BTN_A:  // Type character or select action
            if (inActionColumn) {
                // Action column
                switch (cursorY) {
                    case 0:  // CAP - Toggle caps/symbol
                        if (symbolMode) {
                            symbolMode = false;
                        } else if (capsLock) {
                            capsLock = false;
                            symbolMode = true;
                        } else {
                            capsLock = true;
                        }
                        break;
                    case 1:  // SPC - Space
                        typeChar(' ');
                        break;
                    case 2:  // OK - Confirm
                        strncpy(outputBuffer, inputBuffer, maxLength);
                        confirmed = true;
                        active = false;
                        break;
                    case 3:  // DEL - Backspace
                        backspace();
                        break;
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
    if (y >= numRows || x >= rowLengths[y]) return ' ';

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
