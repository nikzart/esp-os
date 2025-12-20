#include "apps/pong.h"
#include "ui.h"
#include "input.h"
#include "icons.h"

void PongApp::init() {
    state = State::MENU;
}

void PongApp::startGame() {
    playerY = 26;
    aiY = 26;
    playerScore = 0;
    aiScore = 0;
    aiSpeed = 1.5;

    resetBall(random(0, 2) == 0);
    state = State::PLAYING;
    lastUpdate = millis();
}

void PongApp::resetBall(bool towardsPlayer) {
    ballX = 64;
    ballY = 32;

    float speed = 2.0;
    float angle = random(-45, 46) * PI / 180.0;

    ballVX = cos(angle) * speed * (towardsPlayer ? -1 : 1);
    ballVY = sin(angle) * speed;
}

void PongApp::updateAI() {
    // Simple AI: move towards ball
    float targetY = ballY - PADDLE_H / 2;

    // Add some prediction
    if (ballVX > 0) {
        // Ball coming towards AI
        float timeToReach = (120 - ballX) / ballVX;
        targetY = ballY + ballVY * timeToReach - PADDLE_H / 2;
    }

    // Move towards target
    if (aiY < targetY - 2) {
        aiY += aiSpeed;
    } else if (aiY > targetY + 2) {
        aiY -= aiSpeed;
    }

    // Clamp to screen
    if (aiY < 0) aiY = 0;
    if (aiY > 64 - PADDLE_H) aiY = 64 - PADDLE_H;
}

void PongApp::update() {
    if (state != State::PLAYING) return;

    // Frame rate control (~60 FPS)
    if (millis() - lastUpdate < 16) return;
    lastUpdate = millis();

    // Player input
    if (Input::isPressed(BTN_UP)) {
        playerY -= 3;
        if (playerY < 0) playerY = 0;
    }
    if (Input::isPressed(BTN_DOWN)) {
        playerY += 3;
        if (playerY > 64 - PADDLE_H) playerY = 64 - PADDLE_H;
    }

    // Update AI
    updateAI();

    // Move ball
    ballX += ballVX;
    ballY += ballVY;

    // Top/bottom wall bounce
    if (ballY <= 0 || ballY >= 64 - BALL_SIZE) {
        ballVY = -ballVY;
        ballY = constrain(ballY, 0, 64 - BALL_SIZE);
        UI::beep(1500, 20);
    }

    // Player paddle collision (left side)
    if (ballX <= PADDLE_W + 4 && ballX >= 4) {
        if (ballY + BALL_SIZE >= playerY && ballY <= playerY + PADDLE_H) {
            ballVX = abs(ballVX) * 1.05;  // Speed up slightly
            ballVY += (ballY - (playerY + PADDLE_H / 2)) * 0.1;
            ballX = PADDLE_W + 5;
            UI::beep(2000, 30);
        }
    }

    // AI paddle collision (right side)
    if (ballX >= 128 - PADDLE_W - 4 - BALL_SIZE && ballX <= 128 - 4 - BALL_SIZE) {
        if (ballY + BALL_SIZE >= aiY && ballY <= aiY + PADDLE_H) {
            ballVX = -abs(ballVX) * 1.05;
            ballVY += (ballY - (aiY + PADDLE_H / 2)) * 0.1;
            ballX = 128 - PADDLE_W - 5 - BALL_SIZE;
            UI::beep(2000, 30);
        }
    }

    // Scoring
    if (ballX < 0) {
        // AI scores
        aiScore++;
        UI::beep(500, 100);
        if (aiScore >= winScore) {
            playerWon = false;
            state = State::GAME_OVER;
        } else {
            resetBall(true);
        }
    } else if (ballX > 128) {
        // Player scores
        playerScore++;
        UI::beep(1000, 100);
        if (playerScore >= winScore) {
            playerWon = true;
            state = State::GAME_OVER;
        } else {
            resetBall(false);
        }
    }

    // Increase AI speed as game progresses
    aiSpeed = 1.5 + (playerScore + aiScore) * 0.1;
}

void PongApp::render() {
    UI::clear();

    switch (state) {
        case State::MENU:
            UI::drawTitleBar("Pong");
            UI::drawCentered(28, "You vs AI");
            UI::drawCentered(40, "First to 5 wins");
            UI::drawStatusBar("A:Start", "B:Exit");
            break;

        case State::PLAYING:
            // Center line
            for (int y = 0; y < 64; y += 8) {
                u8g2.drawVLine(64, y, 4);
            }

            // Score
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", playerScore);
                u8g2.drawStr(50, 10, buf);
                snprintf(buf, sizeof(buf), "%d", aiScore);
                u8g2.drawStr(74, 10, buf);
            }

            // Player paddle (left)
            u8g2.drawBox(4, playerY, PADDLE_W, PADDLE_H);

            // AI paddle (right)
            u8g2.drawBox(128 - 4 - PADDLE_W, aiY, PADDLE_W, PADDLE_H);

            // Ball
            u8g2.drawBox((int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE);
            break;

        case State::GAME_OVER:
            UI::drawTitleBar("Game Over");

            if (playerWon) {
                UI::drawCentered(28, "You Win!");
            } else {
                UI::drawCentered(28, "AI Wins!");
            }

            {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d - %d", playerScore, aiScore);
                UI::drawCentered(42, buf);
            }

            UI::drawStatusBar("A:Again", "B:Exit");
            break;
    }

    UI::flush();
}

void PongApp::onButton(uint8_t btn, bool pressed) {
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
            if (btn == BTN_D) {
                state = State::GAME_OVER;
                playerWon = playerScore > aiScore;
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

const uint8_t* PongApp::getIcon() {
    return icon_pong;
}
