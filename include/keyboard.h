#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Arduino.h>
#include "config.h"

class Keyboard {
public:
    // Initialize keyboard
    static void init();

    // Show keyboard with prompt
    static void show(const char* prompt, char* buffer, int maxLen);

    // Hide keyboard
    static void hide();

    // Update keyboard state (call every frame when active)
    static void update();

    // Render keyboard (call every frame when active)
    static void render();

    // Handle button input
    static void onButton(uint8_t btn, bool pressed);

    // Check if keyboard is active
    static bool isActive();

    // Check if input was confirmed
    static bool isConfirmed();

    // Check if input was cancelled
    static bool isCancelled();

    // Get current input text
    static const char* getText();

private:
    static bool active;
    static bool confirmed;
    static bool cancelled;
    static char* outputBuffer;
    static int maxLength;
    static char inputBuffer[64];
    static int inputLen;
    static int cursorX;
    static int cursorY;
    static bool capsLock;
    static bool symbolMode;
    static bool inActionColumn;
    static const char* prompt;

    static char getChar(int x, int y);
    static void typeChar(char c);
    static void backspace();
};

#endif
