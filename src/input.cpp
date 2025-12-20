#include "input.h"
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

bool Input::currentState[NUM_BUTTONS] = {false};
bool Input::previousState[NUM_BUTTONS] = {false};
unsigned long Input::lastDebounce[NUM_BUTTONS] = {0};
unsigned long Input::lastActivity = 0;
int Input::sleepTimeoutIndex = 0;
bool Input::skipNextCallback = false;
Input::ButtonCallback Input::callback = nullptr;

const uint8_t Input::buttonPins[NUM_BUTTONS] = {
    BTN_PIN_LEFT, BTN_PIN_RIGHT, BTN_PIN_UP, BTN_PIN_DOWN,
    BTN_PIN_A, BTN_PIN_B, BTN_PIN_C, BTN_PIN_D
};

// Sleep timeout values in ms: Off, 30s, 1min, 2min, 5min
static const unsigned long sleepTimeouts[] = {0, 30000, 60000, 120000, 300000};

void Input::init() {
    lastActivity = millis();
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        currentState[i] = false;
        previousState[i] = false;
        lastDebounce[i] = 0;
    }
}

void Input::update() {
    unsigned long now = millis();

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        previousState[i] = currentState[i];

        // Read raw state (active low)
        bool reading = !digitalRead(buttonPins[i]);

        // Debounce
        if (reading != currentState[i]) {
            if (now - lastDebounce[i] > DEBOUNCE_MS) {
                currentState[i] = reading;
                lastDebounce[i] = now;

                // Update activity on any button press
                if (reading) {
                    lastActivity = now;
                }

                // Fire callback (skip if waking from sleep)
                if (callback && !skipNextCallback) {
                    callback(i, reading);
                } else if (skipNextCallback && !reading) {
                    // Button released after wake - clear flag
                    skipNextCallback = false;
                }
            }
        }
    }
}

bool Input::isPressed(uint8_t btn) {
    if (btn >= NUM_BUTTONS) return false;
    return currentState[btn];
}

bool Input::justPressed(uint8_t btn) {
    if (btn >= NUM_BUTTONS) return false;
    return currentState[btn] && !previousState[btn];
}

bool Input::justReleased(uint8_t btn) {
    if (btn >= NUM_BUTTONS) return false;
    return !currentState[btn] && previousState[btn];
}

bool Input::getState(uint8_t btn) {
    if (btn >= NUM_BUTTONS) return false;
    return currentState[btn];
}

void Input::setCallback(ButtonCallback cb) {
    callback = cb;
}

unsigned long Input::getLastActivity() {
    return lastActivity;
}

void Input::resetActivity() {
    lastActivity = millis();
}

int Input::getSleepTimeoutIndex() {
    return sleepTimeoutIndex;
}

unsigned long Input::getSleepTimeoutMs() {
    return sleepTimeouts[sleepTimeoutIndex];
}

void Input::loadSleepTimeout() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    sleepTimeoutIndex = prefs.getInt("sleep_timeout", 0);
    if (sleepTimeoutIndex < 0 || sleepTimeoutIndex > 4) sleepTimeoutIndex = 0;
    prefs.end();
}

void Input::saveSleepTimeout(int index) {
    if (index < 0 || index > 4) return;
    sleepTimeoutIndex = index;
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putInt("sleep_timeout", sleepTimeoutIndex);
    prefs.end();
}

void Input::enterSleep() {
    // Enable GPIO wakeup on all buttons
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_wakeup_enable((gpio_num_t)buttonPins[i], GPIO_INTR_LOW_LEVEL);
    }
    esp_sleep_enable_gpio_wakeup();

    // Enter light sleep (preserves state, no reboot)
    esp_light_sleep_start();

    // Disable all wakeup sources after wake
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_wakeup_disable((gpio_num_t)buttonPins[i]);
    }

    // Skip next button callback (the wake press)
    skipNextCallback = true;
    lastActivity = millis();
}
