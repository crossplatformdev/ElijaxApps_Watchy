#include "WatchyUi.h"
#include "Watchy.h"
#include <esp_system.h>
#include <math.h>
#include "AppDisplay.h"
#include "SensorManager.h"

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
  beginAppDisplay("ROCK PAPER");
  const uint16_t foreground = WatchyUi::Theme::foreground();
  constexpr WatchyUi::Bounds youBounds{16, 43, 70, 70};
  constexpr WatchyUi::Bounds cpuBounds{114, 43, 70, 70};
  WatchyUi::GrayPaint::fillRoundRect(
      youBounds, 5, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(youBounds, foreground);
  WatchyUi::Canvas::centeredText({youBounds.x, 29, youBounds.width, 13}, "YOU", 1,
                                 foreground);
  char selected[2] = {names[choice][0], '\0'};
  WatchyUi::Canvas::centeredText(youBounds, selected, 5, foreground);
  if (result >= 0) {
    WatchyUi::GrayPaint::fillRoundRect(
        cpuBounds, 5, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    WatchyUi::Canvas::outline(cpuBounds, foreground);
    WatchyUi::Canvas::centeredText({cpuBounds.x, 29, cpuBounds.width, 13}, "CPU", 1,
                                   foreground);
    char opponent[2] = {names[computer][0], '\0'};
    WatchyUi::Canvas::centeredText(cpuBounds, opponent, 5, foreground);
    WatchyUi::Canvas::centeredText(
        {0, 121, 200, 18}, result == 0 ? "DRAW" : result > 0 ? "YOU WIN" : "YOU LOSE",
        2, foreground);
  } else {
    WatchyUi::Canvas::centeredText({0, 124, 200, 16}, "UP/DOWN TO CHOOSE", 1,
                                   foreground);
  }
  char score[20];
  snprintf(score, sizeof(score), "W %u    L %u", wins, losses);
  WatchyUi::Canvas::centeredText({0, 152, 200, 16}, score, 1, foreground);
  WatchyUi::Widget::footer("SELECT PLAY  BACK EXIT");
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
    } else if (event == WatchyUi::Event::MENU) {
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
  const uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::Bounds panel{22, 44, 156, 72};
  WatchyUi::GrayPaint::fillRoundRect(
      panel, 5, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(panel, foreground);
  WatchyUi::Canvas::centeredText(panel, message,
                                 strlen(message) > 7 ? 2 : 3, foreground);
  if (result > 0) {
    char elapsed[12];
    snprintf(elapsed, sizeof(elapsed), "%lu ms", static_cast<unsigned long>(result));
    WatchyUi::Canvas::centeredText({0, 129, 200, 20}, elapsed, 2, foreground);
  } else {
    WatchyUi::Canvas::centeredText({0, 132, 200, 16},
                                   !strcmp(message, "GO!") ? "TAP NOW" : "WAIT FOR THE SIGNAL",
                                   1, foreground);
  }
  WatchyUi::Widget::footer("SELECT START / HIT  BACK EXIT");
  finishAppDisplay();
}

void runReaction() {
  WatchyUi::Input::begin();
  drawReaction("READY");
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event != WatchyUi::Event::MENU) continue;
    WatchyUi::Input::waitForRelease(WatchyUi::Event::MENU);
    drawReaction("WAIT...");
    uint32_t waitMs = 1200 + esp_random() % 2800;
    WatchyUi::Event waitEvent = WatchyUi::Input::wait(waitMs);
    if (waitEvent == WatchyUi::Event::BACK) return;
    if (waitEvent == WatchyUi::Event::MENU) {
      drawReaction("TOO SOON");
      continue;
    }
    drawReaction("GO!");
    pulse(30);
    uint32_t go = millis();
    WatchyUi::Event response = WatchyUi::Input::wait();
    if (response == WatchyUi::Event::BACK) return;
    if (response != WatchyUi::Event::MENU) continue;
    uint32_t elapsed = millis() - go;
    drawReaction("RESULT", elapsed);
  }
}

void drawHigherLower(uint8_t card, int score, const char *result) {
  beginAppDisplay("HIGHER / LOWER");
  const uint16_t foreground = WatchyUi::Theme::foreground();
  constexpr WatchyUi::Bounds cardBounds{62, 35, 76, 87};
  WatchyUi::GrayPaint::fillRoundRect(
      cardBounds, 6, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(cardBounds, foreground);
  char value[4];
  snprintf(value, sizeof(value), "%u", card);
  WatchyUi::Canvas::centeredText(cardBounds, value, 5, foreground);
  WatchyUi::Canvas::centeredText({0, 130, 200, 15}, result, 1, foreground);
  char scoreText[12];
  snprintf(scoreText, sizeof(scoreText), "SCORE %d", score);
  WatchyUi::Canvas::centeredText({0, 151, 200, 15}, scoreText, 1, foreground);
  WatchyUi::Widget::footer("UP HIGHER  DOWN LOWER  BACK EXIT");
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
  const uint16_t foreground = WatchyUi::Theme::foreground();
  constexpr int16_t centerX = 100;
  constexpr int16_t centerY = 80;
  Watchy::display.drawCircle(centerX, centerY, 40, foreground);
  float angle = M_PI * (1.25f + static_cast<float>(guess - 1) / 99.0f * 1.5f);
  int16_t markerX = centerX + static_cast<int16_t>(cosf(angle) * 32.0f);
  int16_t markerY = centerY + static_cast<int16_t>(sinf(angle) * 32.0f);
  Watchy::display.fillCircle(markerX, markerY, 4, foreground);
  char value[4];
  snprintf(value, sizeof(value), "%u", guess);
  WatchyUi::Canvas::centeredText({68, 58, 64, 42}, value, 4, foreground);
  WatchyUi::Canvas::centeredText({0, 129, 200, 15}, hint, 1, foreground);
  char attemptsText[18];
  snprintf(attemptsText, sizeof(attemptsText), "ATTEMPTS %u", attempts);
  WatchyUi::Canvas::centeredText({0, 151, 200, 15}, attemptsText, 1, foreground);
  WatchyUi::Widget::footer("UP/DOWN +/-1  SELECT GUESS");
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
      if (event == WatchyUi::Event::MENU) {
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
    } else if (event == WatchyUi::Event::MENU) {
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
  beginAppDisplay("NIM");
  const uint16_t foreground = WatchyUi::Theme::foreground();
  for (uint8_t index = 0; index < pile; index++) {
    int16_t x = 28 + (index % 7) * 24;
    int16_t y = 36 + (index / 7) * 24;
    Watchy::display.drawLine(x, y, x, y + 15, foreground);
    Watchy::display.drawLine(x - 3, y + 3, x + 3, y + 3, foreground);
  }
  for (uint8_t index = 0; index < 3; index++) {
    int16_t x = 79 + index * 18;
    if (index < take) Watchy::display.fillCircle(x, 122, 5, foreground);
    else Watchy::display.drawCircle(x, 122, 5, foreground);
  }
  WatchyUi::Canvas::centeredText({0, 136, 200, 15}, status, 1, foreground);
  char pileText[18];
  snprintf(pileText, sizeof(pileText), "%u LEFT", pile);
  WatchyUi::Canvas::centeredText({0, 155, 200, 15}, pileText, 1, foreground);
  WatchyUi::Widget::footer("UP/DOWN TAKE  SELECT TAKE");
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
      if (event == WatchyUi::Event::MENU) {
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
    } else if (event == WatchyUi::Event::MENU) {
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
    } else if (event == WatchyUi::Event::MENU) {
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
    } else if (event == WatchyUi::Event::MENU) {
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
  const uint16_t foreground = WatchyUi::Theme::foreground();
  constexpr WatchyUi::Bounds playerCard{28, 37, 61, 74};
  constexpr WatchyUi::Bounds dealerCard{111, 37, 61, 74};
  WatchyUi::GrayPaint::fillRoundRect(
      playerCard, 5, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::GrayPaint::fillRoundRect(
      dealerCard, 5, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(playerCard, foreground);
  WatchyUi::Canvas::outline(dealerCard, foreground);
  char playerText[4];
  char dealerText[4];
  snprintf(playerText, sizeof(playerText), "%u", player);
  snprintf(dealerText, sizeof(dealerText), "%u", dealer);
  WatchyUi::Canvas::centeredText({playerCard.x, 25, playerCard.width, 12}, "YOU", 1,
                                 foreground);
  WatchyUi::Canvas::centeredText({dealerCard.x, 25, dealerCard.width, 12}, "CPU", 1,
                                 foreground);
  WatchyUi::Canvas::centeredText(playerCard, playerText, 4, foreground);
  WatchyUi::Canvas::centeredText(dealerCard, dealerText, 4, foreground);
  WatchyUi::Canvas::centeredText({0, 125, 200, 15}, status, 1, foreground);
  WatchyUi::Widget::footer("SELECT HIT  UP STAND  DOWN NEW");
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
    } else if (event == WatchyUi::Event::MENU && player <= 21) {
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
  const uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::Bounds equation{12, 39, 176, 65};
  WatchyUi::GrayPaint::fillRoundRect(
      equation, 5, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(equation, foreground);
  char text[20];
  snprintf(text, sizeof(text), "%u + %u = %d", left, right, answer);
  WatchyUi::Canvas::centeredText(equation, text, 3, foreground);
  WatchyUi::Canvas::centeredText({0, 119, 200, 15}, status, 1, foreground);
  char scoreText[14];
  snprintf(scoreText, sizeof(scoreText), "SCORE %u", score);
  WatchyUi::Canvas::centeredText({0, 143, 200, 15}, scoreText, 1, foreground);
  WatchyUi::Widget::footer("UP/DOWN ANSWER  SELECT OK");
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
    else if (event == WatchyUi::Event::MENU) {
      bool correct = answer == left + right; score += correct; pulse(correct ? 80 : 25);
      status = correct ? "CORRECT" : "WRONG"; drawMath(left, right, answer, score, status); WatchyUi::deepSleepDelay(500);
      left = esp_random() % 20 + 1; right = esp_random() % 20 + 1; answer = 0; status = "SOLVE IT"; drawMath(left, right, answer, score, status);
    }
  }
}

void drawBalance(uint8_t seconds, bool level) {
  beginAppDisplay("BALANCE CHALLENGE");
  const uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.drawLine(35, 93, 165, 93, foreground);
  int16_t markerX = level ? 100 : 135;
  Watchy::display.fillCircle(markerX, 93, 8, foreground);
  char value[4];
  snprintf(value, sizeof(value), "%u", seconds);
  WatchyUi::Canvas::centeredText({0, 38, 200, 38}, value, 4, foreground);
  AppVisual::drawProgressTrack({28, 115, 144, 12}, seconds / 10.0f);
  WatchyUi::Canvas::centeredText({0, 140, 200, 16},
                                 level ? "HOLD LEVEL" : "TILT BACK TO LEVEL", 1,
                                 foreground);
  WatchyUi::Widget::footer("REACH 10 SECONDS  BACK EXIT");
  finishAppDisplay();
}

void runBalance() {
  WatchyUi::Input::begin();
  if (!WatchySensor::acquireForeground(
          WatchySensor::Mode::LiveAcceleration)) {
    WatchyUi::Feedback::showMessage(
        "BALANCE CHALLENGE", "Accelerometer unavailable.",
        WatchyUi::MessageKind::ERROR, "BACK EXIT");
    while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
    return;
  }
  uint32_t levelStarted = 0; uint8_t displayed = UINT8_MAX;
  while (true) {
    if (WatchyUi::Input::wait(50) == WatchyUi::Event::BACK) break;
    Accel acceleration; bool valid = WatchySensor::readAcceleration(acceleration);
    bool level = valid && abs(acceleration.x) < 120 && abs(acceleration.y) < 120;
    if (level && levelStarted == 0) levelStarted = millis();
    if (!level) levelStarted = 0;
    uint8_t seconds = levelStarted == 0 ? 0 : min<uint32_t>(10, (millis() - levelStarted) / 1000);
    if (seconds != displayed) { displayed = seconds; drawBalance(seconds, level); }
    if (seconds >= 10) {
      pulse(250);
      drawBalance(seconds, true);
      if (WatchyUi::Input::wait(800) == WatchyUi::Event::BACK) break;
      levelStarted = 0;
      displayed = UINT8_MAX;
    }
  }
  WatchySensor::releaseForeground(WatchySensor::Mode::LiveAcceleration);
}

} // namespace

void showMiniGameImpl(uint8_t game, Watchy *watchy) {
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
  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showMiniGame(uint8_t game) { showMiniGameImpl(game, this); }

void WatchySdk::showMiniGame(uint8_t game) { showMiniGameImpl(game, nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMiniGamePreview(uint8_t game, uint8_t view) {
  switch (game) {
  case ROCK_PAPER_SCISSORS:
    drawRps(1, view == 0 ? -2 : view == 1 ? 1 : view == 2 ? -1 : 0,
            view == 1 ? 0 : view == 2 ? 2 : 1,
            view == 1 ? 4 : 3, view == 2 ? 2 : 1);
    break;
  case REACTION_TEST:
    drawReaction(view == 0 ? "READY" : view == 1 ? "WAIT..."
                                : view == 2 ? "TOO SOON" : "RESULT",
                 view == 3 ? 247 : 0);
    break;
  case HIGHER_LOWER:
    drawHigherLower(9, view == 2 ? 0 : 4,
                    view == 0 ? "MAKE YOUR GUESS" : view == 1 ? "CORRECT"
                        : view == 2 ? "WRONG - SCORE RESET" : "TIE - NO POINT");
    break;
  case NUMBER_GUESS:
    drawGuess(view == 0 ? 50 : view == 1 ? 42 : view == 2 ? 78 : 64,
              view == 0 ? "MAKE YOUR GUESS" : view == 1 ? "TOO LOW"
                         : view == 2 ? "TOO HIGH" : "CORRECT! SELECT: NEW",
              view == 0 ? 0 : view == 3 ? 4 : 3);
    break;
  case NIM:
    drawNim(view == 0 ? 13 : 0, 2,
            view == 0 ? "YOUR TURN" : view == 1 ? "YOU WIN! SELECT: NEW"
                                                  : "CPU WINS. SELECT: NEW");
    break;
  case TIC_TAC_TOE: {
    const int8_t boards[][9] = {
        {1, -1, 0, 0, 1, -1, 0, 0, 0},
        {1, 1, 1, -1, -1, 0, 0, 0, 0},
        {-1, -1, -1, 1, 1, 0, 0, 0, 0},
        {1, -1, 1, 1, -1, -1, -1, 1, 1}};
    const char *const statuses[] = {
        "YOUR TURN", "YOU WIN - DOWN: RESET", "CPU WINS - DOWN: RESET",
        "DRAW - DOWN: RESET"};
    uint8_t state = min<uint8_t>(view, 3);
    drawTicTacToe(boards[state], view == 0 ? 8 : 4, statuses[state]);
    break;
  }
  case LIGHTS_OUT: {
    bool board[] = {
        true, false, true, false, false,
        true, true, false, false, true,
        false, true, true, false, false,
        false, false, true, true, false,
        true, false, false, true, true};
    if (view != 0) {
      memset(board, 0, sizeof(board));
    }
    drawLights(board, 12, view == 0 ? 8 : 14);
    break;
  }
  case BLACKJACK:
    drawBlackjack(view == 1 ? 19 : view == 3 ? 18 : 20,
                  view == 2 ? 22 : view == 3 ? 20 : 18,
                  view == 0 ? "YOUR TURN" : view == 1 ? "HIT OR STAND"
                      : view == 2 ? "YOU WIN" : view == 3 ? "CPU WINS"
                                                           : "PUSH");
    break;
  case QUICK_MATH:
    drawMath(7, 8, view == 0 ? 0 : view == 1 ? 15 : 14,
             view == 1 ? 4 : 3,
             view == 0 ? "SOLVE IT" : view == 1 ? "CORRECT" : "WRONG");
    break;
  case BALANCE_CHALLENGE:
    drawBalance(view == 2 ? 10 : view == 0 ? 7 : 0, view != 1);
    break;
  default:
    break;
  }
}

} // namespace WatchyDemo
#endif
