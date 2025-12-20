#include "input.h"

bool Input::currentState[NUM_BUTTONS] = {false};
bool Input::previousState[NUM_BUTTONS] = {false};
unsigned long Input::lastDebounce[NUM_BUTTONS] = {0};
Input::ButtonCallback Input::callback = nullptr;

const uint8_t Input::buttonPins[NUM_BUTTONS] = {
    BTN_PIN_LEFT, BTN_PIN_RIGHT, BTN_PIN_UP, BTN_PIN_DOWN,
    BTN_PIN_A, BTN_PIN_B, BTN_PIN_C, BTN_PIN_D
};

void Input::init() {
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

                // Fire callback
                if (callback) {
                    callback(i, reading);
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
