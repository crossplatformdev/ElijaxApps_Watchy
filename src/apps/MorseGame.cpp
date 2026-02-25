#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/Fonts.h"
#include "../sdk/UiTemplates.h"

#if defined(ESP32)
  #include <esp_system.h>
#endif

char ascii_alphabet[38] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '.', ','
};

String morse_alphabet[38] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
    "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.",
    ".-.-.-", "--..--"
};

// Game: Roll a dice of the length of the alphabet, display the letter and ask the user to input dots and dashes until they match the letter, then roll another.
/*

___________________
|BACK         UP  |  (MenuSpec with the two options, controls: BACK, UP, DOWN, SELECT)
| Guess letter    |
| Guess morse     |
|                 |
|                 |
|                 |
|HINT         DOWN|
-------------------
___________________
|DELETE       DASH|  (GAME 1: Guess letter, GAME 2: Guess morse, controls: DELETE LAST INPUT, ADD DASH, ADD DOT, SUBMIT)
|  GUESS LETTER:  |
|    <letter>     |
|     MORSE:      |
|    <morse>      |
|                 |
|SUBMIT        DOT|
-------------------

___________________
|BACK         UP  |  (GAME 2: Guess morse, controls: DELETE LAST INPUT, NEXT LETTER, PREVIOUS LETTER, SUBMIT)
|  GUESS MORSE:   |
|    <morse>      |
|     MORSE:      |
|    <letter>     |
|                 |
|SUBMIT       DOWN|
-------------------

___________________
|BACK            |  (RESULT screen, controls: BACK, RETRY/NEXT)
|     RESULT:    |
|   CORRECT/NO   |
|                 |
|   ANSWER:      |
|   <letter>     |
|   <morse>      |
|RETRY/NEXT      |
-------------------

1. Roll a dice to select a random element from the alpabet corresponding to the option selected (letter or morse)
2. Display the letter or morse and ask the user to input the corresponding morse or letter
3. If the user is correct, increment the score and roll again, if not, end the game and display the score as [guessed/total (percentage)]
4. The user has 3 tries. After 3 wrong tries, roll again and count the try as failed.

*/

namespace {

constexpr uint8_t kAlphabetLen = 38;
constexpr uint8_t kMaxTries    = 3;

enum class Screen : uint8_t {
  ModeMenu = 0,
  GuessLetter,
  GuessMorse,
  Result,
};

struct Score {
  uint16_t correct = 0;
  uint16_t total   = 0;
};

struct ResultState {
  Screen returnTo;
  bool correct;
  bool advanced;          // true when we move on to a new target after this result
  uint8_t triesLeftThis;  // tries left for the *current* target after the attempt
  char letter;
  String morse;
  String yourAnswer;
  bool buzzPending;
};

struct MorseGameState {
  Screen screen = Screen::ModeMenu;
  int8_t menuSelected = 0;

  // Per-run game state (reset on entering a mode)
  Score score{};
  uint8_t wrongTries = 0;
  uint8_t targetIndex = 0;

  // Guess-letter mode input
  String morseInput;

  // Guess-morse mode input
  int8_t guessIndex = 0;

  ResultState result{};
  Screen lastModeScreen = Screen::ModeMenu;

  bool dirty = true;
  volatile bool exitRequested = false;
};

static MorseGameState sGame;

static uint8_t rollIndex();
static void buzzSymbol(Watchy &watchy, char symbol);

static void resetRun() {
  sGame.score = {};
  sGame.wrongTries = 0;
  sGame.targetIndex = rollIndex();
  sGame.morseInput = "";
  sGame.guessIndex = 0;
  sGame.result = {};
  sGame.dirty = true;
}

static void morseBack(Watchy *watchy) {
  
  switch (sGame.screen) {
    case Screen::ModeMenu:
      sGame.exitRequested = true;
      return;
    case Screen::Result:
      sGame.screen = Screen::ModeMenu;
      sGame.dirty = true;
      return;
    case Screen::GuessLetter:
      if (sGame.morseInput.length() > 0) {
        sGame.morseInput.remove(sGame.morseInput.length() - 1);
      } else {
        sGame.screen = Screen::ModeMenu;
      }
      sGame.dirty = true;
      return;
    case Screen::GuessMorse:
      if (sGame.guessIndex != 0) {
        sGame.guessIndex = 0;
      } else {
        sGame.screen = Screen::ModeMenu;
      }
      sGame.dirty = true;
      return;
  }
}

static void morseUp(Watchy *watchy) {
  switch (sGame.screen) {
    case Screen::ModeMenu:
      sGame.menuSelected = (sGame.menuSelected <= 0) ? 1 : 0;
      sGame.dirty = true;
      return;
    case Screen::GuessLetter:
      sGame.morseInput += "-";
      buzzSymbol(*watchy, '-');
      sGame.dirty = true;
      return;
    case Screen::GuessMorse:
      sGame.guessIndex = static_cast<int8_t>((sGame.guessIndex + 1) % kAlphabetLen);
      sGame.dirty = true;
      return;
    case Screen::Result:
      return;
  }
}

static void morseDown(Watchy *watchy) {
  switch (sGame.screen) {
    case Screen::ModeMenu:
      sGame.menuSelected = (sGame.menuSelected >= 1) ? 0 : 1;
      sGame.dirty = true;
      return;
    case Screen::GuessLetter:
      sGame.morseInput += ".";
      buzzSymbol(*watchy, '.');
      sGame.dirty = true;
      return;
    case Screen::GuessMorse:
      sGame.guessIndex = static_cast<int8_t>((sGame.guessIndex - 1 + kAlphabetLen) % kAlphabetLen);
      sGame.dirty = true;
      return;
    case Screen::Result:
      return;
  }
}

static void morseMenu(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);
  if (sGame.screen == Screen::ModeMenu) {
    resetRun();
    sGame.screen = (sGame.menuSelected == 0) ? Screen::GuessLetter : Screen::GuessMorse;
    sGame.dirty = true;
    return;
  }

  if (sGame.screen == Screen::Result) {
    sGame.screen = sGame.result.returnTo;
    sGame.dirty = true;
    resetRun();
    return;
  }

  if (sGame.screen == Screen::GuessLetter) {
    sGame.result.returnTo = Screen::GuessLetter;
    sGame.result.letter = ascii_alphabet[sGame.targetIndex];
    sGame.result.morse = morse_alphabet[sGame.targetIndex];
    sGame.result.yourAnswer = ""; //sGame.morseInput;

    if (sGame.morseInput == morse_alphabet[sGame.targetIndex]) {
      sGame.result.correct = true;
      sGame.result.advanced = true;
      sGame.result.triesLeftThis = kMaxTries;
      sGame.result.buzzPending = true;

      sGame.score.correct++;
      sGame.score.total++;
      sGame.wrongTries = 0;
      sGame.morseInput = "";
      sGame.targetIndex = rollIndex();
    } else {
      sGame.result.correct = false;
      sGame.wrongTries++;
      sGame.result.buzzPending = true;

      if (sGame.wrongTries >= kMaxTries) {
        sGame.result.advanced = true;
        sGame.result.triesLeftThis = 0;
        sGame.score.total++;
        sGame.wrongTries = 0;
        sGame.morseInput = "";
        sGame.targetIndex = rollIndex();
      } else {
        sGame.result.advanced = false;
        sGame.result.triesLeftThis = static_cast<uint8_t>(kMaxTries - sGame.wrongTries);
        sGame.morseInput = "";
      }
    }

    sGame.screen = Screen::Result;
    sGame.dirty = true;
    return;
  }

  if (sGame.screen == Screen::GuessMorse) {
    sGame.result.returnTo = Screen::GuessMorse;
    sGame.result.letter = ascii_alphabet[sGame.targetIndex];
    sGame.result.morse = morse_alphabet[sGame.targetIndex];
    sGame.result.yourAnswer = ""; //String(ascii_alphabet[static_cast<uint8_t>((sGame.guessIndex < 0) ? 0 : sGame.guessIndex)]);

    if (static_cast<uint8_t>(sGame.guessIndex) == sGame.targetIndex) {
      sGame.result.correct = true;
      sGame.result.advanced = true;
      sGame.result.triesLeftThis = kMaxTries;
      sGame.result.buzzPending = true;

      sGame.score.correct++;
      sGame.score.total++;
      sGame.wrongTries = 0;
      sGame.guessIndex = 0;
      sGame.targetIndex = rollIndex();
    } else {
      sGame.result.correct = false;
      sGame.wrongTries++;
      sGame.result.buzzPending = true;

      if (sGame.wrongTries >= kMaxTries) {
        sGame.result.advanced = true;
        sGame.result.triesLeftThis = 0;
        sGame.score.total++;
        sGame.wrongTries = 0;
        sGame.targetIndex = rollIndex();
        sGame.guessIndex = 0;
      } else {
        sGame.result.advanced = false;
        sGame.result.triesLeftThis = static_cast<uint8_t>(kMaxTries - sGame.wrongTries);
        sGame.guessIndex = 0;
      }
    }

    sGame.screen = Screen::Result;
    sGame.dirty = true;
    return;
  }
}

constexpr uint16_t kDotBuzzMs = 80;
constexpr uint16_t kDashBuzzMs = 240;
constexpr uint16_t kInterSymbolGapMs = 90;

static void buzzSymbol(Watchy &watchy, char sym) {
  if (sym == '.') {
    watchy.vibMotor(kDotBuzzMs, 1);
  } else if (sym == '-') {
    watchy.vibMotor(kDashBuzzMs, 1);
  }
}

static void buzzMorse(Watchy &watchy, const String &morse) {
  for (uint16_t i = 0; i < morse.length(); ++i) {
    const char c = morse[i];
    if (c != '.' && c != '-') {
      continue;
    }
    buzzSymbol(watchy, c);
    delay(kInterSymbolGapMs);
  }
}

static void seedRandomOnce() {
  static bool seeded = false;
  if (seeded) return;
  seeded = true;

  uint32_t seed = static_cast<uint32_t>(micros()) ^ static_cast<uint32_t>(millis());
#if defined(ESP32)
  seed ^= static_cast<uint32_t>(esp_random());
#endif
  randomSeed(seed);
}

static uint8_t rollIndex() {
  return static_cast<uint8_t>(random(static_cast<long>(kAlphabetLen)));
}

static String scoreText(const Score &s) {
  if (s.total == 0) {
    return String("Score 0/0 (0%)");
  }
  const uint16_t pct = static_cast<uint16_t>((100UL * s.correct + (s.total / 2)) / s.total);
  return String("Score ") + String(s.correct) + "/" + String(s.total) + " (" + String(pct) + "%)";
}

static int16_t centeredX(WatchyGxDisplay &display, const GFXfont *font, const String &text) {
  display.setFont(font);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int16_t x = (WatchyDisplay::WIDTH - static_cast<int16_t>(w)) / 2;
  return (x < 0) ? 0 : x;
}

static void drawModeMenu(Watchy &watchy, int8_t selected) {
  UiSDK::initScreen(watchy.display);

  static UIMenuItemSpec items[2] = {
    {"Guess letter"},
    {"Guess morse"},
  };

  UIMenuSpec menu{};
  menu.x             = 0;
  menu.y             = 50;
  menu.w             = WatchyDisplay::WIDTH;
  menu.h             = (MENU_HEIGHT * 2) - 50;
  menu.itemHeight    = MENU_HEIGHT;
  menu.font          = &FreeMonoBold9pt7b;
  menu.items         = items;
  menu.itemCount     = 2;
  menu.selectedIndex = selected;
  menu.startIndex    = 0;
  menu.visibleCount  = 2;

  UIAppSpec app{};
  app.menus     = &menu;
  app.menuCount = 1;
  app.controls[0].label = "BACK";
  app.controls[0].callback = &Watchy::backPressed;
  app.controls[1].label = "UP";
  app.controls[1].callback = &Watchy::upPressed;
  app.controls[2].label = "SELECT";
  app.controls[2].callback = &Watchy::menuPressed;
  app.controls[3].label = "DOWN";
  app.controls[3].callback = &Watchy::downPressed;
  
  UiSDK::renderApp(watchy, app);
}

static void drawGuessLetter(Watchy &watchy,
                            uint8_t targetIndex,
                            const String &input,
                            uint8_t wrongTries,
                            const Score &score,
                            const String & /*toast*/) {
  UiSDK::initScreen(watchy.display);

  const uint8_t triesLeft = (wrongTries >= kMaxTries) ? 0 : static_cast<uint8_t>(kMaxTries - wrongTries);

  // Title
  {
    const String t = "GUESS LETTER:";
    watchy.display.setFont(&FreeMonoBold12pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold12pt7b, t), 44);
    watchy.display.print(t);
  }

  // Big letter
  {
    char letterBuf[2] = {ascii_alphabet[targetIndex], '\0'};
    const String letter(letterBuf);
    watchy.display.setFont(&FreeMonoBold24pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold24pt7b, letter), 92);
    watchy.display.print(letter);
  }

  // "MORSE:" label + user input
  {
    watchy.display.setFont(&FreeMonoBold12pt7b);
    watchy.display.setCursor(10, 122);
    watchy.display.print("MORSE:");

    watchy.display.setFont(&FreeMonoBold18pt7b);
    const String shown = (input.length() == 0) ? String("") : input;
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold18pt7b, shown), 152);
    watchy.display.print(shown);
  }

  // Score + tries
  {
    watchy.display.setFont(UiSDK::tinyMono6x8());
    watchy.display.setCursor(10, 174);
    watchy.display.print(scoreText(score));

    watchy.display.setCursor(10, 186);
    watchy.display.print(String("Tries left: ") + String(triesLeft));
  }

  UIControlsRowLayout controls[4] = {
      {"DELETE", nullptr},
      {"DASH", nullptr},
      {"SUBMIT", nullptr},
      {"DOT", nullptr},
  };
  UiSDK::renderControlsRow(watchy, controls);
  watchy.display.display(true);
}

static void drawGuessMorse(Watchy &watchy,
                           uint8_t targetIndex,
                           int8_t guessIndex,
                           uint8_t wrongTries,
                           const Score &score,
                           const String & /*toast*/) {
  UiSDK::initScreen(watchy.display);

  const uint8_t triesLeft = (wrongTries >= kMaxTries) ? 0 : static_cast<uint8_t>(kMaxTries - wrongTries);

  // Title
  {
    const String t = "GUESS MORSE:";
    watchy.display.setFont(&FreeMonoBold12pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold12pt7b, t), 44);
    watchy.display.print(t);
  }

  // Prompt morse
  {
    const String prompt = morse_alphabet[targetIndex];
    watchy.display.setFont(&FreeMonoBold18pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold18pt7b, prompt), 86);
    watchy.display.print(prompt);
  }

  // "LETTER:" label + current guess
  {
    watchy.display.setFont(&FreeMonoBold12pt7b);
    watchy.display.setCursor(10, 122);
    watchy.display.print("LETTER:");

    const char c = ascii_alphabet[(guessIndex < 0) ? 0 : static_cast<uint8_t>(guessIndex)];
    char buf[2] = {c, '\0'};
    const String letter(buf);

    watchy.display.setFont(&FreeMonoBold24pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold24pt7b, letter), 160);
    watchy.display.print(letter);
  }

  // Score + tries
  {
    watchy.display.setFont(UiSDK::tinyMono6x8());
    watchy.display.setCursor(10, 174);
    watchy.display.print(scoreText(score));

    watchy.display.setCursor(10, 186);
    watchy.display.print(String("Tries left: ") + String(triesLeft));
  }

  UIControlsRowLayout controls[4] = {
      {"DELETE", nullptr},
      {"NEXT", nullptr},
      {"SUBMIT", nullptr},
      {"PREV", nullptr},
  };
  UiSDK::renderControlsRow(watchy, controls);
  watchy.display.display(true);
}

static void drawResultScreen(Watchy &watchy,
                             const ResultState &r,
                             const Score &score) {
  UiSDK::initScreen(watchy.display);

  // Header
  {
    const String t = "RESULT:";
    watchy.display.setFont(&FreeMonoBold12pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold12pt7b, t), 44);
    watchy.display.print(t);
  }

  // Big outcome
  {
    const String outcome = r.correct ? "CORRECT" : "NO";
    watchy.display.setFont(&FreeMonoBold18pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold18pt7b, outcome), 84);
    watchy.display.print(outcome);
  }

  // Answer block
  {
    watchy.display.setFont(&FreeMonoBold12pt7b);
    watchy.display.setCursor(10, 114);
    watchy.display.print("ANSWER:");

    String letterLine;
    letterLine += r.letter;

    watchy.display.setFont(&FreeMonoBold24pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold24pt7b, letterLine), 150);
    watchy.display.print(letterLine);

    watchy.display.setFont(&FreeMonoBold18pt7b);
    watchy.display.setCursor(centeredX(watchy.display, &FreeMonoBold18pt7b, r.morse), 176);
    watchy.display.print(r.morse);
  }

  // Footer (score + tries)
  {
    watchy.display.setFont(UiSDK::tinyMono6x8());
    watchy.display.setCursor(10, 194);
    watchy.display.print(scoreText(score));
  }

  const char *acceptLabel = r.advanced ? "NEXT" : "RETRY";
  UIControlsRowLayout controls[4] = {
      {"BACK", nullptr},
      {nullptr, nullptr},
      {acceptLabel, nullptr},
      {nullptr, nullptr},
  };
  
  UiSDK::renderControlsRow(watchy, controls);
  watchy.display.display(true);
}

} // namespace

void Watchy::showMorseGame() {
  guiState = APP_STATE;
  seedRandomOnce();

  // Init state
  sGame = {};
  sGame.screen = Screen::ModeMenu;
  sGame.menuSelected = 0;
  sGame.targetIndex = rollIndex();
  sGame.result.returnTo = Screen::ModeMenu;
  sGame.result.correct = false;
  sGame.result.advanced = true;
  sGame.result.triesLeftThis = kMaxTries;
  sGame.result.letter = 'A';
  sGame.result.morse = "";
  sGame.result.yourAnswer = "";
  sGame.result.buzzPending = false;
  sGame.dirty = true;
  sGame.exitRequested = false;
  UiTemplates::waitForAllButtonsReleased();
  setButtonHandlers(morseBack, morseUp, morseMenu, morseDown);

  while (true) {
    // Pump input frequently; labels depend on the current screen.
    const char *acceptLabel = nullptr;
    if (sGame.screen == Screen::ModeMenu) {
      acceptLabel = "SELECT";
    } else if (sGame.screen == Screen::GuessLetter || sGame.screen == Screen::GuessMorse) {
      acceptLabel = "SUBMIT";
    } else if (sGame.screen == Screen::Result) {
      acceptLabel = sGame.result.advanced ? "NEXT" : "RETRY";
    }

    UIControlsRowLayout controls[4] = {
        {"BACK", &Watchy::backPressed},
        {"UP", &Watchy::upPressed},
        {acceptLabel, &Watchy::menuPressed},
        {"DOWN", &Watchy::downPressed},
    };
    if (sGame.screen == Screen::GuessLetter) {
      controls[0].label = "DEL";
      controls[1].label = "DASH";
      controls[3].label = "DOT";
    } else if (sGame.screen == Screen::GuessMorse) {
      controls[0].label = "DEL";
      controls[1].label = "NEXT";
      controls[3].label = "PREV";
    } else if (sGame.screen == Screen::Result) {
      controls[1].label = nullptr;
      controls[1].callback = nullptr;
      controls[3].label = nullptr;
      controls[3].callback = nullptr;
    }
    UiSDK::renderControlsRow(*this, controls);

    if (sGame.exitRequested) {
      clearButtonHandlers();
      showMenu(menuIndex);
      return;
    }

    if (sGame.dirty) {
      switch (sGame.screen) {
        case Screen::ModeMenu:
          drawModeMenu(*this, sGame.menuSelected);
          break;
        case Screen::GuessLetter:
          drawGuessLetter(*this, sGame.targetIndex, sGame.morseInput, sGame.wrongTries, sGame.score, "");
          sGame.lastModeScreen = Screen::GuessLetter;
          break;
        case Screen::GuessMorse:
          drawGuessMorse(*this, sGame.targetIndex, sGame.guessIndex, sGame.wrongTries, sGame.score, "");
          sGame.lastModeScreen = Screen::GuessMorse;
          break;
        case Screen::Result:
          drawResultScreen(*this, sGame.result, sGame.score);
          if (sGame.result.buzzPending) {
            // Buzz the answer pattern while the result screen is shown.
            buzzMorse(*this, sGame.result.morse);
            sGame.result.buzzPending = false;
            // clear the input answer after buzzing, so that if the user retries, they won't be biased by the previous wrong answer still being shown.
            sGame.result.yourAnswer = "";             
          }
          break;
        }
      sGame.dirty = false;
    }

    delay(10);
  }
}