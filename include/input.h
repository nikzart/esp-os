#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "config.h"

class Input {
public:
    static void init();
    static void update();

    // Check if button is currently pressed
    static bool isPressed(uint8_t btn);

    // Check if button was just pressed this frame
    static bool justPressed(uint8_t btn);

    // Check if button was just released this frame
    static bool justReleased(uint8_t btn);

    // Get raw button state
    static bool getState(uint8_t btn);

    // Callback type for button events
    typedef void (*ButtonCallback)(uint8_t btn, bool pressed);
    static void setCallback(ButtonCallback cb);

    // Sleep management
    static unsigned long getLastActivity();
    static void resetActivity();
    static void enterSleep();
    static void loadSleepTimeout();
    static void saveSleepTimeout(int index);
    static int getSleepTimeoutIndex();
    static unsigned long getSleepTimeoutMs();

private:
    static bool currentState[NUM_BUTTONS];
    static bool previousState[NUM_BUTTONS];
    static unsigned long lastDebounce[NUM_BUTTONS];
    static unsigned long lastActivity;
    static int sleepTimeoutIndex;
    static ButtonCallback callback;
    static const uint8_t buttonPins[NUM_BUTTONS];
};

#endif
