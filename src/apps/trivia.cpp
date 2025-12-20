#include "apps/trivia.h"
#include "ui.h"
#include "wifi_manager.h"
#include "icons.h"
#include <ArduinoJson.h>

void TriviaApp::init() {
    state = State::MENU;
    loading = false;
    errorMsg[0] = '\0';
    score = 0;
    questionNum = 0;
}

void TriviaApp::update() {
    // No auto-update
}

void TriviaApp::decodeHtml(char* str) {
    // Simple HTML entity decoder
    char* read = str;
    char* write = str;

    while (*read) {
        if (strncmp(read, "&quot;", 6) == 0) {
            *write++ = '"';
            read += 6;
        } else if (strncmp(read, "&amp;", 5) == 0) {
            *write++ = '&';
            read += 5;
        } else if (strncmp(read, "&lt;", 4) == 0) {
            *write++ = '<';
            read += 4;
        } else if (strncmp(read, "&gt;", 4) == 0) {
            *write++ = '>';
            read += 4;
        } else if (strncmp(read, "&#039;", 6) == 0) {
            *write++ = '\'';
            read += 6;
        } else if (strncmp(read, "&apos;", 6) == 0) {
            *write++ = '\'';
            read += 6;
        } else {
            *write++ = *read++;
        }
    }
    *write = '\0';
}

void TriviaApp::fetchQuestion() {
    if (!WiFiManager::isConnected()) {
        strcpy(errorMsg, "No WiFi");
        return;
    }

    loading = true;

    // Open Trivia Database
    String response = WiFiManager::httpGet("https://opentdb.com/api.php?amount=1&type=multiple");
    loading = false;

    if (response.length() == 0) {
        strcpy(errorMsg, "Network error");
        return;
    }

    // Parse JSON
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        strcpy(errorMsg, "Parse error");
        return;
    }

    if (doc["response_code"] != 0) {
        strcpy(errorMsg, "API error");
        return;
    }

    JsonObject q = doc["results"][0];
    const char* questionText = q["question"] | "";
    const char* correct = q["correct_answer"] | "";
    JsonArray incorrect = q["incorrect_answers"];

    strncpy(question, questionText, sizeof(question) - 1);
    decodeHtml(question);

    // Shuffle answers
    correctAnswer = random(0, 4);

    int idx = 0;
    for (int i = 0; i < 4; i++) {
        if (i == correctAnswer) {
            strncpy(answers[i], correct, sizeof(answers[0]) - 1);
        } else {
            if (idx < incorrect.size()) {
                strncpy(answers[i], incorrect[idx].as<const char*>(), sizeof(answers[0]) - 1);
                idx++;
            }
        }
        decodeHtml(answers[i]);
    }

    selectedAnswer = 0;
    answered = false;
    questionNum++;
    errorMsg[0] = '\0';
    state = State::PLAYING;
}

void TriviaApp::render() {
    UI::clear();

    switch (state) {
        case State::MENU:
            UI::drawTitleBar("Trivia Quiz");
            UI::drawCentered(30, "Test your knowledge!");
            UI::setSmallFont();
            UI::drawCentered(42, "Press A to start");
            UI::setNormalFont();
            UI::drawStatusBar("A:Start", "B:Back");
            break;

        case State::PLAYING:
            {
                char buf[16];
                snprintf(buf, sizeof(buf), "Q%d Score:%d", questionNum, score);
                UI::drawTitleBar(buf);

                if (loading) {
                    UI::drawCentered(35, "Loading...");
                } else if (strlen(errorMsg) > 0) {
                    UI::drawCentered(30, errorMsg);
                } else {
                    // Question (truncated)
                    UI::setSmallFont();
                    char shortQ[60];
                    strncpy(shortQ, question, 59);
                    shortQ[59] = '\0';
                    u8g2.drawStr(2, 18, shortQ);

                    // Answers
                    for (int i = 0; i < 4; i++) {
                        int y = 28 + i * 9;
                        char ans[24];
                        snprintf(ans, sizeof(ans), "%c) %.18s", 'A' + i, answers[i]);

                        if (answered) {
                            if (i == correctAnswer) {
                                // Correct answer - highlight
                                u8g2.drawBox(0, y - 7, 128, 9);
                                u8g2.setDrawColor(0);
                                u8g2.drawStr(2, y, ans);
                                u8g2.setDrawColor(1);
                            } else if (i == selectedAnswer) {
                                // Wrong selection
                                u8g2.drawStr(2, y, ans);
                                u8g2.drawStr(118, y, "X");
                            } else {
                                u8g2.drawStr(2, y, ans);
                            }
                        } else {
                            if (i == selectedAnswer) {
                                u8g2.drawFrame(0, y - 7, 128, 9);
                            }
                            u8g2.drawStr(2, y, ans);
                        }
                    }
                    UI::setNormalFont();
                }

                UI::drawStatusBar(answered ? "A:Next" : "A:Submit", "B:Quit");
            }
            break;

        case State::RESULT:
            UI::drawTitleBar("Game Over");
            {
                char buf[32];
                snprintf(buf, sizeof(buf), "Score: %d/%d", score, questionNum);
                UI::drawCentered(30, buf);

                int pct = questionNum > 0 ? (score * 100 / questionNum) : 0;
                const char* rating = pct >= 80 ? "Excellent!" :
                                    pct >= 60 ? "Good job!" :
                                    pct >= 40 ? "Not bad" : "Keep trying!";
                UI::drawCentered(44, rating);
            }
            UI::drawStatusBar("A:Again", "B:Exit");
            break;
    }

    UI::flush();
}

void TriviaApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    switch (state) {
        case State::MENU:
            if (btn == BTN_A) {
                score = 0;
                questionNum = 0;
                fetchQuestion();
                UI::beep();
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;

        case State::PLAYING:
            if (loading) break;

            if (!answered) {
                if (btn == BTN_UP && selectedAnswer > 0) {
                    selectedAnswer--;
                    UI::beep(2500, 20);
                } else if (btn == BTN_DOWN && selectedAnswer < 3) {
                    selectedAnswer++;
                    UI::beep(2500, 20);
                } else if (btn == BTN_A) {
                    // Submit answer
                    answered = true;
                    if (selectedAnswer == correctAnswer) {
                        score++;
                        UI::beep(1000, 100);
                    } else {
                        UI::beep(400, 200);
                    }
                }
            } else {
                if (btn == BTN_A) {
                    // Next question
                    fetchQuestion();
                    UI::beep();
                }
            }

            if (btn == BTN_B) {
                state = State::RESULT;
                UI::beep();
            }
            break;

        case State::RESULT:
            if (btn == BTN_A) {
                score = 0;
                questionNum = 0;
                fetchQuestion();
                UI::beep();
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;
    }
}

const uint8_t* TriviaApp::getIcon() {
    return icon_trivia;
}
