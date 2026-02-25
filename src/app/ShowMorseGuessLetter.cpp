#include <Watchy.h>
#include <esp_system.h>
#include "AppDisplay.h"
#include "MorseGame.h"

namespace {

void drawLetterGame(const MorseGame::Round &round, int correctAnswers,
                    int attempts, bool answered) {
  beginAppDisplay("MORSE LETTER");
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();

  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(145, 12);
  Watchy::display.print(correctAnswers);
  Watchy::display.print('/');
  Watchy::display.print(attempts);

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
      Watchy::display.fillRect(x, y, 88, 40, foreground);
    } else {
      Watchy::display.drawRect(x, y, 88, 40, foreground);
    }
    Watchy::display.setTextColor(filled ? background : foreground);
    Watchy::display.setCursor(x + 37, y + 27);
    Watchy::display.print(MorseGame::alphabet[round.options[option]].letter);
  }

  Watchy::display.setTextColor(foreground);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(0, 190);
  if (answered) {
    Watchy::display.print(MorseGame::isCorrect(round) ? "CORRECT  SELECT: NEXT"
                                                       : "ANSWER SHOWN SELECT: NEXT");
  } else {
    Watchy::display.print("UP/DOWN      SELECT: OK");
  }
  finishAppDisplay();
}

} // namespace

void Watchy::showMorseGuessLetter() {
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
      showMenu(menuIndex, false);
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
    } else if (event == WatchyUi::Event::SELECT) {
      if (answered) {
        int previousAnswer = round.answerIndex;
        MorseGame::startRound(round, previousAnswer);
        answered = false;
      } else {
        attempts++;
        if (MorseGame::isCorrect(round)) {
          correctAnswers++;
        }
        MorseGame::vibrateAnswer(*this, round);
        answered = true;
      }
      drawLetterGame(round, correctAnswers, attempts, answered);
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMorseLetterPreview() {
  MorseGame::Round round{18, {7, 18, 14, 20}, 1};
  drawLetterGame(round, 4, 5, true);
}

} // namespace WatchyDemo
#endif