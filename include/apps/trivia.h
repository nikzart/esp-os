#ifndef TRIVIA_H
#define TRIVIA_H

#include "app.h"

class TriviaApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Trivia"; }
    const uint8_t* getIcon() override;

private:
    enum class State {
        MENU,
        PLAYING,
        RESULT
    };

    State state = State::MENU;
    bool loading = false;
    char errorMsg[32] = "";

    char question[200] = "";
    char answers[4][64];
    int correctAnswer = 0;
    int selectedAnswer = 0;
    int score = 0;
    int questionNum = 0;
    bool answered = false;

    void fetchQuestion();
    void decodeHtml(char* str);
};

#endif
