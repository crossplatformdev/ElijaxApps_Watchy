#ifndef MORSE_GAME_H
#define MORSE_GAME_H

#include <Arduino.h>
#include <Watchy.h>

namespace MorseGame {

struct Entry {
  char letter;
  const char *code;
};

constexpr Entry alphabet[] = {
    {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
    {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
    {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
    {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
    {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
    {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."}};

constexpr int alphabetSize = sizeof(alphabet) / sizeof(alphabet[0]);
constexpr int optionCount = 4;
constexpr uint16_t morseUnitMs = 120;

struct Round {
  int answerIndex;
  int options[optionCount];
  int selectedOption;
};

inline void startRound(Round &round, int previousAnswer = -1) {
  do {
    round.answerIndex = random(alphabetSize);
  } while (alphabetSize > 1 && round.answerIndex == previousAnswer);

  round.options[0] = round.answerIndex;
  for (int option = 1; option < optionCount; option++) {
    bool duplicate;
    do {
      duplicate = false;
      round.options[option] = random(alphabetSize);
      for (int previous = 0; previous < option; previous++) {
        if (round.options[option] == round.options[previous]) {
          duplicate = true;
          break;
        }
      }
    } while (duplicate);
  }

  for (int option = optionCount - 1; option > 0; option--) {
    int swapWith = random(option + 1);
    int temporary = round.options[option];
    round.options[option] = round.options[swapWith];
    round.options[swapWith] = temporary;
  }
  round.selectedOption = 0;
}

inline bool isCorrect(const Round &round) {
  return round.options[round.selectedOption] == round.answerIndex;
}

inline void vibrateAnswer(Watchy &watchy, const Round &round) {
  const char *code = alphabet[round.answerIndex].code;
  for (int element = 0; code[element] != '\0'; element++) {
    uint16_t duration = code[element] == '-' ? morseUnitMs * 3 : morseUnitMs;
    watchy.vibMotor(duration, 1);
    if (code[element + 1] != '\0') {
      delay(morseUnitMs);
    }
  }
}

} // namespace MorseGame

#endif