#include <Watchy.h>
#include <esp_system.h>
#include <math.h>
#include "AppDisplay.h"

namespace {

enum MiniGame : uint8_t {
  ROCK_PAPER_SCISSORS,
  REACTION_TEST,
  HIGHER_LOWER,
  NUMBER_GUESS,
  NIM,
  TIC_TAC_TOE,
  LIGHTS_OUT,
  BLACKJACK,
  QUICK_MATH,
  BALANCE_CHALLENGE,
  MINI_GAME_COUNT
};

void pulse(uint16_t duration = 50) {
  Watchy::vibMotor(duration, 1);
}

void useSmallText(int16_t x = 4, int16_t y = 38) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void drawRps(uint8_t choice, int8_t result, uint8_t computer,
             uint16_t wins, uint16_t losses) {
  const char *const names[] = {"ROCK", "PAPER", "SCISSORS"};
  beginAppDisplay("ROCK PAPER SCISSORS");
  useSmallText(10, 48);
  Watchy::display.print("YOU: "); Watchy::display.println(names[choice]);
  Watchy::display.setCursor(10, 75);
  if (result >= 0) {
    Watchy::display.print("CPU: "); Watchy::display.println(names[computer]);
    Watchy::display.setTextSize(2);
    Watchy::display.setCursor(10, 112);
    Watchy::display.println(result == 0 ? "DRAW" : result > 0 ? "YOU WIN" : "YOU LOSE");
  } else {
    Watchy::display.println("UP/DOWN TO CHOOSE");
  }
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 150);
  Watchy::display.print("W "); Watchy::display.print(wins);
  Watchy::display.print("  L "); Watchy::display.println(losses);
  Watchy::display.setCursor(10, 180);
  Watchy::display.println("SELECT: PLAY  BACK: EXIT");
  finishAppDisplay();
}

void runRps() {
  WatchyUi::Input::begin();
  uint8_t choice = 0;
  uint16_t wins = 0;
  uint16_t losses = 0;
  drawRps(choice, -2, 0, wins, losses);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) {
      choice = WatchyUi::ListView::previous(choice, 3);
      drawRps(choice, -2, 0, wins, losses);
    } else if (event == WatchyUi::Event::DOWN) {
      choice = WatchyUi::ListView::next(choice, 3);
      drawRps(choice, -2, 0, wins, losses);
    } else if (event == WatchyUi::Event::SELECT) {
      uint8_t computer = esp_random() % 3;
      int8_t result = choice == computer ? 0 : (choice + 2) % 3 == computer ? 1 : -1;
      wins += result > 0; losses += result < 0;
      pulse(result > 0 ? 100 : 35);
      drawRps(choice, result, computer, wins, losses);
    }
  }
}

void drawReaction(const char *message, uint32_t result = 0) {
  beginAppDisplay("REACTION TEST");
  useSmallText(18, 70);
  Watchy::display.setTextSize(3);
  Watchy::display.println(message);
  Watchy::display.setTextSize(1);
  if (result > 0) {
    Watchy::display.setCursor(18, 118);
    Watchy::display.print(result); Watchy::display.println(" ms");
  }
  Watchy::display.setCursor(18, 170);
  Watchy::display.println("SELECT: START / HIT BACK: EXIT");
  finishAppDisplay();
}

void runReaction() {
  WatchyUi::Input::begin();
  drawReaction("READY");
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event != WatchyUi::Event::SELECT) continue;
    WatchyUi::Input::waitForRelease(WatchyUi::Event::SELECT);
    drawReaction("WAIT...");
    uint32_t waitMs = 1200 + esp_random() % 2800;
    uint32_t started = millis();
    bool falseStart = false;
    while (millis() - started < waitMs) {
      if (WatchyUi::Input::pressed(WatchyUi::Event::BACK)) {
        WatchyUi::Input::waitForRelease(WatchyUi::Event::BACK);
        return;
      }
      if (WatchyUi::Input::pressed(WatchyUi::Event::SELECT)) {
        WatchyUi::Input::waitForRelease(WatchyUi::Event::SELECT);
        falseStart = true;
        break;
      }
      delay(5);
    }
    if (falseStart) { drawReaction("TOO SOON"); continue; }
    drawReaction("GO!");
    pulse(30);
    uint32_t go = millis();
    while (!WatchyUi::Input::pressed(WatchyUi::Event::SELECT)) {
      if (WatchyUi::Input::pressed(WatchyUi::Event::BACK)) {
        WatchyUi::Input::waitForRelease(WatchyUi::Event::BACK);
        return;
      }
      delay(1);
    }
    uint32_t elapsed = millis() - go;
    WatchyUi::Input::waitForRelease(WatchyUi::Event::SELECT);
    drawReaction("RESULT", elapsed);
  }
}

void drawHigherLower(uint8_t card, int score, const char *result) {
  beginAppDisplay("HIGHER / LOWER");
  useSmallText(70, 72);
  Watchy::display.setTextSize(5);
  Watchy::display.println(card);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 118);
  Watchy::display.println(result);
  Watchy::display.setCursor(8, 146);
  Watchy::display.print("SCORE "); Watchy::display.println(score);
  Watchy::display.setCursor(8, 178);
  Watchy::display.println("UP: HIGHER  DOWN: LOWER");
  finishAppDisplay();
}

void runHigherLower() {
  WatchyUi::Input::begin();
  uint8_t card = esp_random() % 13 + 1;
  int score = 0;
  drawHigherLower(card, score, "MAKE YOUR GUESS");
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    bool higher;
    if (event == WatchyUi::Event::UP) higher = true;
    else if (event == WatchyUi::Event::DOWN) higher = false;
    else continue;
    uint8_t next = esp_random() % 13 + 1;
    bool tie = next == card;
    bool correct = !tie && (higher ? next > card : next < card);
    score = tie ? score : correct ? score + 1 : 0;
    card = next;
    pulse(tie ? 45 : correct ? 80 : 30);
    drawHigherLower(card, score,
            tie ? "TIE - NO POINT" :
            correct ? "CORRECT" : "WRONG - SCORE RESET");
  }
}

void drawGuess(uint8_t guess, const char *hint, uint16_t attempts) {
  beginAppDisplay("NUMBER GUESS");
  useSmallText(54, 72);
  Watchy::display.setTextSize(5);
  Watchy::display.println(guess);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(12, 117);
  Watchy::display.println(hint);
  Watchy::display.setCursor(12, 145);
  Watchy::display.print("ATTEMPTS "); Watchy::display.println(attempts);
  Watchy::display.setCursor(12, 178);
  Watchy::display.println("UP/DOWN +/-1 SELECT: GUESS");
  finishAppDisplay();
}

void runNumberGuess() {
  WatchyUi::Input::begin();
  uint8_t secret = esp_random() % 100 + 1;
  uint8_t guess = 50;
  uint16_t attempts = 0;
  const char *hint = "1 TO 100";
  bool won = false;
  drawGuess(guess, hint, attempts);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (won) {
      if (event == WatchyUi::Event::SELECT) {
        secret = esp_random() % 100 + 1;
        guess = 50;
        attempts = 0;
        hint = "1 TO 100";
        won = false;
        drawGuess(guess, hint, attempts);
      }
      continue;
    }
    if (event == WatchyUi::Event::UP) {
      guess = min<uint8_t>(100, guess + 1);
      drawGuess(guess, hint, attempts);
    } else if (event == WatchyUi::Event::DOWN) {
      guess = max<uint8_t>(1, guess - 1);
      drawGuess(guess, hint, attempts);
    } else if (event == WatchyUi::Event::SELECT) {
      attempts++;
      if (guess == secret) {
        pulse(150);
        hint = "CORRECT! SELECT: NEW";
        won = true;
      } else hint = guess < secret ? "TOO LOW" : "TOO HIGH";
      drawGuess(guess, hint, attempts);
    }
  }
}

void drawNim(uint8_t pile, uint8_t take, const char *status) {
  beginAppDisplay("NIM - TAKE THE LAST");
  useSmallText(45, 68);
  Watchy::display.setTextSize(5);
  Watchy::display.println(pile);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(12, 115);
  Watchy::display.print("TAKE "); Watchy::display.println(take);
  Watchy::display.setCursor(12, 140);
  Watchy::display.println(status);
  Watchy::display.setCursor(12, 177);
  Watchy::display.println("UP/DOWN: 1-3 SELECT: TAKE");
  finishAppDisplay();
}

void runNim() {
  WatchyUi::Input::begin();
  uint8_t pile = 21;
  uint8_t take = 1;
  const char *status = "YOUR TURN";
  drawNim(pile, take, status);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (pile == 0) {
      if (event == WatchyUi::Event::SELECT) {
        pile = 21;
        status = "YOUR TURN";
        drawNim(pile, take, status);
      }
      continue;
    }
    if (event == WatchyUi::Event::UP) {
      take = take == 3 ? 1 : take + 1;
      drawNim(pile, take, status);
    } else if (event == WatchyUi::Event::DOWN) {
      take = take == 1 ? 3 : take - 1;
      drawNim(pile, take, status);
    } else if (event == WatchyUi::Event::SELECT) {
      pile -= min(take, pile);
      if (pile == 0) { status = "YOU WIN! SELECT: NEW"; pulse(150); }
      else {
        uint8_t cpu = pile % 4 == 0 ? esp_random() % 3 + 1 : pile % 4;
        pile -= min(cpu, pile);
        status = pile == 0 ? "CPU WINS. SELECT: NEW" : "YOUR TURN";
      }
      drawNim(pile, take, status);
    }
  }
}

int8_t boardWinner(const int8_t board[9]) {
  const uint8_t lines[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},
                               {1,4,7},{2,5,8},{0,4,8},{2,4,6}};
  for (const auto &line : lines) {
    if (board[line[0]] != 0 && board[line[0]] == board[line[1]] &&
        board[line[1]] == board[line[2]]) return board[line[0]];
  }
  for (uint8_t cell = 0; cell < 9; cell++) if (board[cell] == 0) return 0;
  return 2;
}

void drawTicTacToe(const int8_t board[9], uint8_t cursor, const char *status) {
  beginAppDisplay("TIC TAC TOE");
  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  constexpr int16_t boardX = 43;
  constexpr int16_t boardY = 35;
  constexpr int16_t cellSize = 38;
  for (uint8_t line = 1; line < 3; line++) {
    Watchy::display.drawLine(boardX + line * cellSize, boardY,
                             boardX + line * cellSize,
                             boardY + cellSize * 3, color);
    Watchy::display.drawLine(boardX, boardY + line * cellSize,
                             boardX + cellSize * 3,
                             boardY + line * cellSize, color);
  }
  for (uint8_t cell = 0; cell < 9; cell++) {
    WatchyUi::Bounds cellBounds{
        static_cast<int16_t>(boardX + (cell % 3) * cellSize),
        static_cast<int16_t>(boardY + (cell / 3) * cellSize),
        cellSize, cellSize};
    char symbol[2] = {
        board[cell] == 1 ? 'X' : board[cell] == -1 ? 'O' : '\0', '\0'};
    WatchyUi::Canvas::centeredText(cellBounds, symbol, 3, color);
    if (cell == cursor) {
      WatchyUi::Canvas::outline(cellBounds.inset(2), color);
    }
  }
  useSmallText();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(4, 168); Watchy::display.println(status);
  Watchy::display.setCursor(4, 188); Watchy::display.println("UP/DOWN MOVE SELECT PLACE");
  finishAppDisplay();
}

void runTicTacToe() {
  WatchyUi::Input::begin();
  int8_t board[9] = {};
  uint8_t cursor = 0;
  const char *status = "YOU ARE X";
  drawTicTacToe(board, cursor, status);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (boardWinner(board) != 0) {
      if (event == WatchyUi::Event::DOWN) {
        memset(board, 0, sizeof(board));
        status = "YOU ARE X";
        drawTicTacToe(board, cursor, status);
      }
      continue;
    }
    if (event == WatchyUi::Event::UP) {
      cursor = WatchyUi::ListView::previous(cursor, 9);
      drawTicTacToe(board, cursor, status);
    } else if (event == WatchyUi::Event::DOWN) {
      cursor = WatchyUi::ListView::next(cursor, 9);
      drawTicTacToe(board, cursor, status);
    } else if (event == WatchyUi::Event::SELECT) {
      if (board[cursor] != 0 || boardWinner(board) != 0) continue;
      board[cursor] = 1;
      int8_t winner = boardWinner(board);
      if (winner == 0) {
        uint8_t empty[9]; uint8_t count = 0;
        for (uint8_t cell = 0; cell < 9; cell++) if (board[cell] == 0) empty[count++] = cell;
        board[empty[esp_random() % count]] = -1;
        winner = boardWinner(board);
      }
      status = winner == 1 ? "YOU WIN - DOWN: RESET" : winner == -1 ? "CPU WINS - DOWN: RESET" : winner == 2 ? "DRAW - DOWN: RESET" : "YOUR TURN";
      drawTicTacToe(board, cursor, status);
    }
  }
}

void toggleLight(bool board[25], uint8_t cell) {
  int row = cell / 5;
  int column = cell % 5;
  const int8_t offsets[5][2] = {{0,0},{-1,0},{1,0},{0,-1},{0,1}};
  for (const auto &offset : offsets) {
    int targetRow = row + offset[0]; int targetColumn = column + offset[1];
    if (targetRow >= 0 && targetRow < 5 && targetColumn >= 0 && targetColumn < 5)
      board[targetRow * 5 + targetColumn] = !board[targetRow * 5 + targetColumn];
  }
}

void drawLights(const bool board[25], uint8_t cursor, uint16_t moves) {
  beginAppDisplay("LIGHTS OUT");
  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  for (uint8_t cell = 0; cell < 25; cell++) {
    int16_t x = 31 + (cell % 5) * 28; int16_t y = 30 + (cell / 5) * 28;
    if (board[cell]) Watchy::display.fillRect(x, y, 22, 22, color);
    else Watchy::display.drawRect(x, y, 22, 22, color);
    if (cell == cursor) Watchy::display.drawRect(x - 3, y - 3, 28, 28, color);
  }
  useSmallText(4, 178); Watchy::display.print("MOVES "); Watchy::display.println(moves);
  Watchy::display.setCursor(4, 191); Watchy::display.println("UP/DOWN MOVE SELECT TOGGLE");
  finishAppDisplay();
}

void runLightsOut() {
  WatchyUi::Input::begin();
  bool board[25] = {};
  for (uint8_t move = 0; move < 12; move++) toggleLight(board, esp_random() % 25);
  uint8_t cursor = 0; uint16_t moves = 0;
  drawLights(board, cursor, moves);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) {
      cursor = WatchyUi::ListView::previous(cursor, 25);
      drawLights(board, cursor, moves);
    } else if (event == WatchyUi::Event::DOWN) {
      cursor = WatchyUi::ListView::next(cursor, 25);
      drawLights(board, cursor, moves);
    } else if (event == WatchyUi::Event::SELECT) {
      toggleLight(board, cursor); moves++;
      bool solved = true; for (bool light : board) solved &= !light;
      if (solved) pulse(180);
      drawLights(board, cursor, moves);
    }
  }
}

uint8_t drawCard() { return min<uint8_t>(10, esp_random() % 13 + 1); }

void drawBlackjack(uint8_t player, uint8_t dealer, const char *status) {
  beginAppDisplay("BLACKJACK 21");
  useSmallText(10, 55);
  Watchy::display.setTextSize(3);
  Watchy::display.print("YOU "); Watchy::display.println(player);
  Watchy::display.setCursor(10, 100); Watchy::display.print("CPU "); Watchy::display.println(dealer);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 135); Watchy::display.println(status);
  Watchy::display.setCursor(10, 165); Watchy::display.println("SELECT: HIT  UP: STAND");
  Watchy::display.setCursor(10, 183); Watchy::display.println("DOWN: NEW  BACK: EXIT");
  finishAppDisplay();
}

void runBlackjack() {
  WatchyUi::Input::begin();
  uint8_t player = drawCard() + drawCard();
  uint8_t dealer = drawCard() + drawCard();
  const char *status = "YOUR TURN";
  drawBlackjack(player, dealer, status);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::DOWN) {
      player = drawCard() + drawCard(); dealer = drawCard() + drawCard(); status = "YOUR TURN";
    } else if (event == WatchyUi::Event::SELECT && player <= 21) {
      player += drawCard(); status = player > 21 ? "BUST - CPU WINS" : "HIT OR STAND";
    } else if (event == WatchyUi::Event::UP && player <= 21) {
      while (dealer < 17) dealer += drawCard();
      status = dealer > 21 || player > dealer ? "YOU WIN" : player == dealer ? "PUSH" : "CPU WINS";
    } else continue;
    drawBlackjack(player, dealer, status);
  }
}

void drawMath(uint8_t left, uint8_t right, int answer, uint16_t score, const char *status) {
  beginAppDisplay("QUICK MATH");
  useSmallText(30, 65);
  Watchy::display.setTextSize(3);
  Watchy::display.print(left); Watchy::display.print(" + "); Watchy::display.print(right); Watchy::display.print(" = "); Watchy::display.println(answer);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 115); Watchy::display.println(status);
  Watchy::display.setCursor(10, 145); Watchy::display.print("SCORE "); Watchy::display.println(score);
  Watchy::display.setCursor(10, 178); Watchy::display.println("UP/DOWN ANSWER SELECT: OK");
  finishAppDisplay();
}

void runQuickMath() {
  WatchyUi::Input::begin();
  uint8_t left = esp_random() % 20 + 1; uint8_t right = esp_random() % 20 + 1;
  int answer = 0; uint16_t score = 0; const char *status = "SOLVE IT";
  drawMath(left, right, answer, score, status);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) { answer++; drawMath(left, right, answer, score, status); }
    else if (event == WatchyUi::Event::DOWN) { answer--; drawMath(left, right, answer, score, status); }
    else if (event == WatchyUi::Event::SELECT) {
      bool correct = answer == left + right; score += correct; pulse(correct ? 80 : 25);
      status = correct ? "CORRECT" : "WRONG"; drawMath(left, right, answer, score, status); delay(500);
      left = esp_random() % 20 + 1; right = esp_random() % 20 + 1; answer = 0; status = "SOLVE IT"; drawMath(left, right, answer, score, status);
    }
  }
}

void drawBalance(uint8_t seconds, bool level) {
  beginAppDisplay("BALANCE CHALLENGE");
  useSmallText(30, 65);
  Watchy::display.setTextSize(4);
  Watchy::display.println(seconds);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(30, 110); Watchy::display.println(level ? "HOLD LEVEL" : "TILT BACK TO LEVEL");
  Watchy::display.setCursor(30, 145); Watchy::display.println("Reach 10 seconds");
  Watchy::display.setCursor(30, 180); Watchy::display.println("BACK: EXIT");
  finishAppDisplay();
}

void runBalance() {
  WatchyUi::Input::begin();
  uint32_t levelStarted = 0; uint8_t displayed = UINT8_MAX;
  while (true) {
    if (WatchyUi::Input::poll() == WatchyUi::Event::BACK) return;
    Accel acceleration; bool valid = sensor.getAccel(acceleration);
    bool level = valid && abs(acceleration.x) < 120 && abs(acceleration.y) < 120;
    if (level && levelStarted == 0) levelStarted = millis();
    if (!level) levelStarted = 0;
    uint8_t seconds = levelStarted == 0 ? 0 : min<uint32_t>(10, (millis() - levelStarted) / 1000);
    if (seconds != displayed) { displayed = seconds; drawBalance(seconds, level); }
    if (seconds >= 10) { pulse(250); drawBalance(seconds, true); delay(800); levelStarted = 0; displayed = UINT8_MAX; }
    delay(50);
  }
}

} // namespace

void Watchy::showMiniGame(uint8_t game) {
  switch (game) {
  case ROCK_PAPER_SCISSORS: runRps(); break;
  case REACTION_TEST: runReaction(); break;
  case HIGHER_LOWER: runHigherLower(); break;
  case NUMBER_GUESS: runNumberGuess(); break;
  case NIM: runNim(); break;
  case TIC_TAC_TOE: runTicTacToe(); break;
  case LIGHTS_OUT: runLightsOut(); break;
  case BLACKJACK: runBlackjack(); break;
  case QUICK_MATH: runQuickMath(); break;
  case BALANCE_CHALLENGE: runBalance(); break;
  default: return;
  }
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMiniGamePreview(uint8_t game) {
  switch (game) {
  case ROCK_PAPER_SCISSORS:
    drawRps(1, 1, 0, 3, 1);
    break;
  case REACTION_TEST:
    drawReaction("RESULT", 247);
    break;
  case HIGHER_LOWER:
    drawHigherLower(9, 4, "CORRECT");
    break;
  case NUMBER_GUESS:
    drawGuess(64, "TOO LOW", 3);
    break;
  case NIM:
    drawNim(13, 2, "YOUR TURN");
    break;
  case TIC_TAC_TOE: {
    const int8_t board[] = {1, -1, 0, 0, 1, -1, 0, 0, 0};
    drawTicTacToe(board, 8, "YOUR TURN");
    break;
  }
  case LIGHTS_OUT: {
    const bool board[] = {
        true, false, true, false, false,
        true, true, false, false, true,
        false, true, true, false, false,
        false, false, true, true, false,
        true, false, false, true, true};
    drawLights(board, 12, 8);
    break;
  }
  case BLACKJACK:
    drawBlackjack(18, 16, "HIT OR STAND");
    break;
  case QUICK_MATH:
    drawMath(7, 8, 15, 4, "CORRECT");
    break;
  case BALANCE_CHALLENGE:
    drawBalance(7, true);
    break;
  default:
    break;
  }
}

} // namespace WatchyDemo
#endif