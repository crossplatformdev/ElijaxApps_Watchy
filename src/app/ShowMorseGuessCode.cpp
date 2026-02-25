#include "WatchyUi.h"
#include <esp_system.h>
#include "AppDisplay.h"
#include "MorseGame.h"
#include "Watchy.h"

namespace {

void drawCodeGame(const MorseGame::Round &round, int correctAnswers,
                  int attempts, bool answered) {
  beginAppDisplay("MORSE CODE");
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();

  constexpr WatchyUi::Bounds scoreBounds{136, 1, 60, 18};
  WatchyUi::GrayPaint::fillRoundRect(
      scoreBounds, 3, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  char scoreText[12];
  snprintf(scoreText, sizeof(scoreText), "S %d/%d", correctAnswers, attempts);
  WatchyUi::Canvas::centeredText(scoreBounds, scoreText, 1, foreground);

  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(88, 58);
  Watchy::display.print(MorseGame::alphabet[round.answerIndex].letter);
  Watchy::display.setTextSize(1);

  for (int option = 0; option < MorseGame::optionCount; option++) {
    int column = option % 2;
    int row = option / 2;
    int x = 4 + column * 98;
    int y = 82 + row * 48;
    bool selected = option == round.selectedOption;
    bool correct = round.options[option] == round.answerIndex;
    bool filled = selected || (answered && correct);

    if (filled) {
      Watchy::display.fillRoundRect(x, y, 94, 40, 4, foreground);
    } else {
      WatchyUi::GrayPaint::fillRoundRect(
          {static_cast<int16_t>(x), static_cast<int16_t>(y), 94, 40}, 4,
          WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
      Watchy::display.drawRoundRect(x, y, 94, 40, 4, foreground);
    }
    Watchy::display.setTextColor(filled ? background : foreground);
    const char *code = MorseGame::alphabet[round.options[option]].code;
    WatchyUi::Canvas::centeredText(
        {static_cast<int16_t>(x), static_cast<int16_t>(y), 94, 40}, code, 2,
        filled ? background : foreground);
  }

  Watchy::display.setTextColor(foreground);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  WatchyUi::Widget::footer(
      answered ? MorseGame::isCorrect(round) ? "CORRECT  SELECT NEXT"
                                             : "ANSWER SHOWN  SELECT NEXT"
               : "UP/DOWN CHOOSE  SELECT OK");
  finishAppDisplay();
}

} // namespace

void showMorseGuessCodeImpl(Watchy *watchy) {
  WatchyUi::Input::begin();

  randomSeed(esp_random());
  MorseGame::Round round;
  MorseGame::startRound(round);
  int correctAnswers = 0;
  int attempts = 0;
  bool answered = false;
  drawCodeGame(round, correctAnswers, attempts, answered);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }

    if (!answered && event == WatchyUi::Event::UP) {
      round.selectedOption = WatchyUi::ListView::previous(
          round.selectedOption, MorseGame::optionCount);
      drawCodeGame(round, correctAnswers, attempts, answered);
    } else if (!answered && event == WatchyUi::Event::DOWN) {
      round.selectedOption = WatchyUi::ListView::next(
          round.selectedOption, MorseGame::optionCount);
      drawCodeGame(round, correctAnswers, attempts, answered);
    } else if (event == WatchyUi::Event::MENU) {
      if (answered) {
        int previousAnswer = round.answerIndex;
        MorseGame::startRound(round, previousAnswer);
        answered = false;
      } else {
        attempts++;
        if (MorseGame::isCorrect(round)) {
          correctAnswers++;
        }
        MorseGame::vibrateAnswer(round);
        answered = true;
      }
      drawCodeGame(round, correctAnswers, attempts, answered);
    }
  }
}

void Watchy::showMorseGuessCode() { showMorseGuessCodeImpl(this); }

void WatchySdk::showMorseGuessCode() { showMorseGuessCodeImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMorseCodePreview(uint8_t view) {
  MorseGame::Round round{11, {0, 11, 17, 23},
                         static_cast<uint8_t>(view == 1 ? 1 : 0)};
  drawCodeGame(round, view == 1 ? 3 : 2, view == 0 ? 3 : 4,
               view != 0);
}

} // namespace WatchyDemo
#endif
