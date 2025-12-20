#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <U8g2lib.h>

// Forward declaration
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

// App base class - all apps inherit from this
class App {
public:
    virtual ~App() {}

    // Called once when app is launched
    virtual void init() = 0;

    // Called every frame to update logic
    virtual void update() = 0;

    // Called every frame to render
    virtual void render() = 0;

    // Called on button events
    // btn: BTN_LEFT, BTN_RIGHT, BTN_UP, BTN_DOWN, BTN_A, BTN_B, BTN_C, BTN_D
    // pressed: true on press, false on release
    virtual void onButton(uint8_t btn, bool pressed) = 0;

    // App metadata
    virtual const char* getName() = 0;
    virtual const uint8_t* getIcon() = 0;  // 16x16 XBM bitmap

    // Optional: called when app is about to close
    virtual void onClose() {}

    // Request to exit app (handled by main loop)
    bool wantsToExit = false;

    // Request keyboard input
    bool needsKeyboard = false;
    char keyboardBuffer[64] = {0};
    const char* keyboardPrompt = "";

protected:
    // Helper to request text input
    void requestKeyboard(const char* prompt) {
        needsKeyboard = true;
        keyboardPrompt = prompt;
        memset(keyboardBuffer, 0, sizeof(keyboardBuffer));
    }
};

// App state machine
enum class AppState {
    LAUNCHER,
    APP_RUNNING,
    KEYBOARD_OVERLAY
};

#endif
