#include "WatchyUi.h"
#include <esp_system.h>
#include "AppDisplay.h"
#include "MorseGame.h"
#include "Watchy.h"

namespace {

void drawLetterGame(const MorseGame::Round &round, int correctAnswers,
                    int attempts, bool answered) {
  beginAppDisplay("MORSE LETTER");
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
  Watchy::display.setCursor(65, 58);
  Watchy::display.print(MorseGame::alphabet[round.answerIndex].code);
  Watchy::display.setTextSize(1);

  for (int option = 0; option < MorseGame::optionCount; option++) {
    int column = option % 2;
    int row = option / 2;
    int x = 8 + column * 96;
    int y = 82 + row * 48;
    bool selected = option == round.selectedOption;
    bool correct = round.options[option] == round.answerIndex;
    bool filled = selected || (answered && correct);

    if (filled) {
      Watchy::display.fillRoundRect(x, y, 88, 40, 4, foreground);
    } else {
      WatchyUi::GrayPaint::fillRoundRect(
          {static_cast<int16_t>(x), static_cast<int16_t>(y), 88, 40}, 4,
          WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
      Watchy::display.drawRoundRect(x, y, 88, 40, 4, foreground);
    }
    Watchy::display.setTextColor(filled ? background : foreground);
    char letter[2] = {MorseGame::alphabet[round.options[option]].letter, '\0'};
    WatchyUi::Canvas::centeredText(
        {static_cast<int16_t>(x), static_cast<int16_t>(y), 88, 40}, letter, 2,
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

void showMorseGuessLetterImpl(Watchy *watchy) {
  WatchyUi::Input::begin();

  randomSeed(esp_random());
  MorseGame::Round round;
  MorseGame::startRound(round);
  int correctAnswers = 0;
  int attempts = 0;
  bool answered = false;
  drawLetterGame(round, correctAnswers, attempts, answered);

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
      drawLetterGame(round, correctAnswers, attempts, answered);
    } else if (!answered && event == WatchyUi::Event::DOWN) {
      round.selectedOption = WatchyUi::ListView::next(
          round.selectedOption, MorseGame::optionCount);
      drawLetterGame(round, correctAnswers, attempts, answered);
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
      drawLetterGame(round, correctAnswers, attempts, answered);
    }
  }
}

void Watchy::showMorseGuessLetter() { showMorseGuessLetterImpl(this); }

void WatchySdk::showMorseGuessLetter() { showMorseGuessLetterImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMorseLetterPreview(uint8_t view) {
  MorseGame::Round round{18, {7, 18, 14, 20},
                         static_cast<uint8_t>(view == 1 ? 1 : 0)};
  drawLetterGame(round, view == 1 ? 4 : 3, view == 0 ? 4 : 5,
                 view != 0);
}

} // namespace WatchyDemo
#endif
