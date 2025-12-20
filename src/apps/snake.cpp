#include "apps/snake.h"
#include "ui.h"
#include "icons.h"
#include <Preferences.h>

void SnakeApp::init() {
    state = State::MENU;
    loadHighScore();
}

void SnakeApp::loadHighScore() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    highScore = prefs.getInt("snake_high", 0);
    prefs.end();
}

void SnakeApp::saveHighScore() {
    if (score > highScore) {
        highScore = score;
        Preferences prefs;
        prefs.begin(NVS_NAMESPACE, false);
        prefs.putInt("snake_high", highScore);
        prefs.end();
    }
}

void SnakeApp::startGame() {
    // Initialize snake in center
    snakeLen = 3;
    snakeHead = 0;

    int startX = SNAKE_GRID_W / 2;
    int startY = SNAKE_GRID_H / 2;

    for (int i = 0; i < snakeLen; i++) {
        snakeX[i] = startX - i;
        snakeY[i] = startY;
    }

    direction = 0;  // Moving right
    nextDirection = 0;
    score = 0;
    speed = 150;
    lastMove = millis();

    spawnFood();
    state = State::PLAYING;
}

void SnakeApp::spawnFood() {
    // Find empty spot
    bool valid = false;
    while (!valid) {
        foodX = random(0, SNAKE_GRID_W);
        foodY = random(0, SNAKE_GRID_H);

        valid = true;
        for (int i = 0; i < snakeLen; i++) {
            int idx = (snakeHead - i + SNAKE_MAX_LEN) % SNAKE_MAX_LEN;
            if (snakeX[idx] == foodX && snakeY[idx] == foodY) {
                valid = false;
                break;
            }
        }
    }
}

bool SnakeApp::checkCollision(int x, int y) {
    // Wall collision
    if (x < 0 || x >= SNAKE_GRID_W || y < 0 || y >= SNAKE_GRID_H) {
        return true;
    }

    // Self collision (skip head)
    for (int i = 1; i < snakeLen; i++) {
        int idx = (snakeHead - i + SNAKE_MAX_LEN) % SNAKE_MAX_LEN;
        if (snakeX[idx] == x && snakeY[idx] == y) {
            return true;
        }
    }

    return false;
}

void SnakeApp::moveSnake() {
    // Apply direction change
    direction = nextDirection;

    // Calculate new head position
    int headX = snakeX[snakeHead];
    int headY = snakeY[snakeHead];

    switch (direction) {
        case 0: headX++; break;  // Right
        case 1: headY++; break;  // Down
        case 2: headX--; break;  // Left
        case 3: headY--; break;  // Up
    }

    // Check collision
    if (checkCollision(headX, headY)) {
        state = State::GAME_OVER;
        saveHighScore();
        UI::beep(300, 300);
        return;
    }

    // Check food
    bool ateFood = (headX == foodX && headY == foodY);

    if (ateFood) {
        score++;
        if (snakeLen < SNAKE_MAX_LEN) {
            snakeLen++;
        }
        // Speed up slightly
        if (speed > 80) speed -= 2;

        UI::beep(1500, 50);
        spawnFood();
    }

    // Move head
    snakeHead = (snakeHead + 1) % SNAKE_MAX_LEN;
    snakeX[snakeHead] = headX;
    snakeY[snakeHead] = headY;
}

void SnakeApp::update() {
    if (state != State::PLAYING) return;

    if (millis() - lastMove >= (unsigned long)speed) {
        moveSnake();
        lastMove = millis();
    }
}

void SnakeApp::render() {
    UI::clear();

    switch (state) {
        case State::MENU:
            UI::drawTitleBar("Snake");
            UI::drawCentered(28, "Press A to start");

            {
                char buf[32];
                snprintf(buf, sizeof(buf), "High Score: %d", highScore);
                UI::drawCentered(42, buf);
            }

            UI::drawStatusBar("A:Start", "B:Exit");
            break;

        case State::PLAYING:
            {
                // Draw border
                u8g2.drawFrame(0, 0, SNAKE_GRID_W * 4, SNAKE_GRID_H * 4);

                // Draw snake
                for (int i = 0; i < snakeLen; i++) {
                    int idx = (snakeHead - i + SNAKE_MAX_LEN) % SNAKE_MAX_LEN;
                    int x = snakeX[idx] * 4;
                    int y = snakeY[idx] * 4;

                    if (i == 0) {
                        // Head - filled
                        u8g2.drawBox(x, y, 4, 4);
                    } else {
                        // Body
                        u8g2.drawBox(x + 1, y + 1, 2, 2);
                    }
                }

                // Draw food
                u8g2.drawBox(foodX * 4, foodY * 4, 4, 4);

                // Score
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", score);
                u8g2.drawStr(SNAKE_GRID_W * 4 + 4, 10, buf);
            }
            break;

        case State::GAME_OVER:
            UI::drawTitleBar("Game Over");

            {
                char buf[32];
                snprintf(buf, sizeof(buf), "Score: %d", score);
                UI::drawCentered(28, buf);

                if (score >= highScore) {
                    UI::drawCentered(40, "NEW HIGH SCORE!");
                } else {
                    snprintf(buf, sizeof(buf), "Best: %d", highScore);
                    UI::drawCentered(40, buf);
                }
            }

            UI::drawStatusBar("A:Again", "B:Exit");
            break;
    }

    UI::flush();
}

void SnakeApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    switch (state) {
        case State::MENU:
            if (btn == BTN_A) {
                startGame();
                UI::beep();
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;

        case State::PLAYING:
            // Direction changes (prevent 180 degree turns)
            if (btn == BTN_UP && direction != 1) {
                nextDirection = 3;
            } else if (btn == BTN_DOWN && direction != 3) {
                nextDirection = 1;
            } else if (btn == BTN_LEFT && direction != 0) {
                nextDirection = 2;
            } else if (btn == BTN_RIGHT && direction != 2) {
                nextDirection = 0;
            } else if (btn == BTN_D) {
                state = State::GAME_OVER;
                saveHighScore();
            }
            break;

        case State::GAME_OVER:
            if (btn == BTN_A) {
                startGame();
                UI::beep();
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;
    }
}

const uint8_t* SnakeApp::getIcon() {
    return icon_snake;
}
