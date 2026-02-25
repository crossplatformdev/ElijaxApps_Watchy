#include <Watchy.h>
#include <esp_system.h>
#include "sdk/WatchyUi.h"

namespace {

constexpr int fieldTop = 23;
constexpr int fieldBottom = DISPLAY_HEIGHT - 2;
constexpr int playerPaddleX = 8;
constexpr int computerPaddleX = 187;
constexpr int paddleWidth = 5;
constexpr int paddleHeight = 32;
constexpr int ballSize = 6;
constexpr int winningScore = 5;
constexpr uint16_t hitVibrationMs = 50;

struct PongState {
  int playerY;
  int computerY;
  int ballX;
  int ballY;
  int ballVelocityX;
  int ballVelocityY;
  uint8_t playerScore;
  uint8_t computerScore;
  uint8_t serveTicks;
  bool paused;
  bool gameOver;
};

int clampPaddleY(int y) {
  return constrain(y, fieldTop + 3, fieldBottom - paddleHeight - 2);
}

void resetBall(PongState &state, int direction) {
  state.ballX = (DISPLAY_WIDTH - ballSize) / 2;
  state.ballY = fieldTop + (fieldBottom - fieldTop - ballSize) / 2;
  state.ballVelocityX = direction * 8;
  state.ballVelocityY = random(0, 2) == 0 ? -4 : 4;
  state.serveTicks = 2;
}

void resetGame(PongState &state) {
  state.playerY = fieldTop + 67;
  state.computerY = fieldTop + 67;
  state.playerScore = 0;
  state.computerScore = 0;
  state.paused = false;
  state.gameOver = false;
  resetBall(state, random(0, 2) == 0 ? -1 : 1);
}

void drawPong(const PongState &state) {
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();

  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, 8);
  Watchy::display.print("PONG");
  Watchy::display.setCursor(86, 8);
  Watchy::display.print(state.playerScore);
  Watchy::display.print(':');
  Watchy::display.print(state.computerScore);

  Watchy::display.drawRect(1, fieldTop - 1, DISPLAY_WIDTH - 2,
                           fieldBottom - fieldTop + 2, foreground);
  for (int y = fieldTop + 5; y < fieldBottom - 4; y += 12) {
    Watchy::display.drawFastVLine(DISPLAY_WIDTH / 2, y, 6, foreground);
  }
  Watchy::display.fillRect(playerPaddleX, state.playerY, paddleWidth,
                           paddleHeight, foreground);
  Watchy::display.fillRect(computerPaddleX, state.computerY, paddleWidth,
                           paddleHeight, foreground);
  Watchy::display.fillRect(state.ballX, state.ballY, ballSize, ballSize,
                           foreground);

  if (state.paused || state.gameOver) {
    Watchy::display.fillRect(49, 91, 102, 28, background);
    Watchy::display.drawRect(49, 91, 102, 28, foreground);
    Watchy::display.setCursor(state.gameOver ? 67 : 79, 102);
    if (state.gameOver) {
      Watchy::display.print(state.playerScore > state.computerScore
                                ? "YOU WIN"
                                : "CPU WINS");
    } else {
      Watchy::display.print("PAUSED");
    }
  }

  WatchyUi::Screen::present();
}

void adjustBounce(PongState &state, int paddleY) {
  int impact = state.ballY + ballSize / 2 - (paddleY + paddleHeight / 2);
  state.ballVelocityY = constrain(impact / 3, -6, 6);
  if (state.ballVelocityY == 0) {
    state.ballVelocityY = random(0, 2) == 0 ? -2 : 2;
  }
}

uint8_t updatePong(PongState &state) {
  if (state.serveTicks > 0) {
    state.serveTicks--;
    return 0;
  }

  int computerCenter = state.computerY + paddleHeight / 2;
  int ballCenter = state.ballY + ballSize / 2;
  if (ballCenter < computerCenter - 3) {
    state.computerY = clampPaddleY(state.computerY - 5);
  } else if (ballCenter > computerCenter + 3) {
    state.computerY = clampPaddleY(state.computerY + 5);
  }

  state.ballX += state.ballVelocityX;
  state.ballY += state.ballVelocityY;
  uint8_t hits = 0;

  const int minimumBallY = fieldTop + 2;
  const int maximumBallY = fieldBottom - ballSize - 1;
  if (state.ballY <= minimumBallY) {
    state.ballY = minimumBallY;
    state.ballVelocityY = abs(state.ballVelocityY);
    hits++;
  } else if (state.ballY >= maximumBallY) {
    state.ballY = maximumBallY;
    state.ballVelocityY = -abs(state.ballVelocityY);
    hits++;
  }

  if (state.ballVelocityX < 0 &&
      state.ballX <= playerPaddleX + paddleWidth &&
      state.ballX + ballSize >= playerPaddleX &&
      state.ballY + ballSize >= state.playerY &&
      state.ballY <= state.playerY + paddleHeight) {
    state.ballX = playerPaddleX + paddleWidth;
    state.ballVelocityX = abs(state.ballVelocityX);
    adjustBounce(state, state.playerY);
    hits++;
  } else if (state.ballVelocityX > 0 &&
             state.ballX + ballSize >= computerPaddleX &&
             state.ballX <= computerPaddleX + paddleWidth &&
             state.ballY + ballSize >= state.computerY &&
             state.ballY <= state.computerY + paddleHeight) {
    state.ballX = computerPaddleX - ballSize;
    state.ballVelocityX = -abs(state.ballVelocityX);
    adjustBounce(state, state.computerY);
    hits++;
  }

  if (state.ballX + ballSize < 0) {
    state.computerScore++;
    if (state.computerScore >= winningScore) {
      state.gameOver = true;
      state.paused = true;
    } else {
      resetBall(state, -1);
    }
  } else if (state.ballX >= DISPLAY_WIDTH) {
    state.playerScore++;
    if (state.playerScore >= winningScore) {
      state.gameOver = true;
      state.paused = true;
    } else {
      resetBall(state, 1);
    }
  }

  return hits;
}

} // namespace

void Watchy::showPong() {
  WatchyUi::Input::begin();

  randomSeed(esp_random());
  PongState state;
  resetGame(state);
  drawPong(state);
  uint32_t lastFrame = millis();

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
      return;
    }

    if (event == WatchyUi::Event::SELECT) {
      if (state.gameOver) {
        resetGame(state);
      } else {
        state.paused = !state.paused;
      }
      drawPong(state);
      lastFrame = millis();
    }

    if (!state.paused && millis() - lastFrame >= 120) {
      if (WatchyUi::Input::pressed(WatchyUi::Event::UP)) {
        state.playerY = clampPaddleY(state.playerY - 12);
      }
      if (WatchyUi::Input::pressed(WatchyUi::Event::DOWN)) {
        state.playerY = clampPaddleY(state.playerY + 12);
      }

      uint8_t hits = updatePong(state);
      while (hits-- > 0) {
        vibMotor(hitVibrationMs, 1);
      }
      drawPong(state);
      lastFrame = millis();
    }
    delay(10);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderPongPreview() {
  PongState state{76, 105, 116, 81, 8, -4, 2, 1, 0, false, false};
  drawPong(state);
}

} // namespace WatchyDemo
#endif