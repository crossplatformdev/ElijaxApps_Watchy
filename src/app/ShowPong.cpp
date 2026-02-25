#include "WatchyUi.h"
#include <esp_system.h>
#include "AppDisplay.h"
#include "Watchy.h"

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
constexpr WatchyUi::Bounds scoreBounds{80, 0, 48, 14};
constexpr WatchyUi::Bounds overlayBounds{49, 91, 102, 28};

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

WatchyUi::Bounds ballBounds(const PongState &state) {
  return {static_cast<int16_t>(state.ballX),
          static_cast<int16_t>(state.ballY), ballSize, ballSize};
}

WatchyUi::Bounds paddleBounds(int x, int y) {
  return {static_cast<int16_t>(x), static_cast<int16_t>(y),
          paddleWidth, paddleHeight};
}

bool sameBounds(const WatchyUi::Bounds &first,
                const WatchyUi::Bounds &second) {
  return first.x == second.x && first.y == second.y &&
         first.width == second.width && first.height == second.height;
}

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

void drawPongField(uint16_t foreground) {
  Watchy::display.drawRect(1, fieldTop - 1, DISPLAY_WIDTH - 2,
                           fieldBottom - fieldTop + 2, foreground);
  for (int y = fieldTop + 5; y < fieldBottom - 4; y += 12) {
    Watchy::display.drawFastVLine(DISPLAY_WIDTH / 2, y, 6, foreground);
  }
}

void drawPongActors(const PongState &state, uint16_t foreground) {
  Watchy::display.fillRect(playerPaddleX, state.playerY, paddleWidth,
                           paddleHeight, foreground);
  Watchy::display.fillRect(computerPaddleX, state.computerY, paddleWidth,
                           paddleHeight, foreground);
  Watchy::display.fillRect(state.ballX, state.ballY, ballSize, ballSize,
                           foreground);
}

void drawPongScore(const PongState &state, uint16_t foreground) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foreground);
  char score[8];
  snprintf(score, sizeof(score), "%u:%u", state.playerScore,
           state.computerScore);
  Watchy::display.setCursor(86, AppVisual::centeredCursorY(9, score));
  Watchy::display.print(score);
}

void drawPongOverlay(const PongState &state, uint16_t background,
                     uint16_t foreground) {
  if (!state.paused && !state.gameOver) return;
  Watchy::display.fillRect(overlayBounds.x, overlayBounds.y,
                           overlayBounds.width, overlayBounds.height,
                           background);
  WatchyUi::GrayPaint::fillRoundRect(
      overlayBounds, 4, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(overlayBounds, foreground);
  WatchyUi::Canvas::centeredText(
      overlayBounds, state.gameOver
                         ? state.playerScore > state.computerScore ? "YOU WIN"
                                                                    : "CPU WINS"
                         : "PAUSED",
      1, foreground);
}

void drawPong(const PongState &state) {
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();

  WatchyUi::Screen::beginCanvas();
  WatchyUi::GrayPaint::fillRect(
      {0, 0, DISPLAY_WIDTH, 18},
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, AppVisual::centeredCursorY(9, "PONG"));
  Watchy::display.print("PONG");
  Watchy::display.setCursor(48, AppVisual::centeredCursorY(9, "YOU"));
  Watchy::display.print("YOU");
  Watchy::display.setCursor(130, AppVisual::centeredCursorY(9, "CPU"));
  Watchy::display.print("CPU");
  drawPongScore(state, foreground);
  drawPongField(foreground);
  drawPongActors(state, foreground);
  drawPongOverlay(state, background, foreground);

  WatchyUi::Screen::present();
}

void drawPongDelta(const PongState &previous, const PongState &current) {
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::Bounds previousBall = ballBounds(previous);
  WatchyUi::Bounds currentBall = ballBounds(current);
  WatchyUi::Bounds previousPlayer =
      paddleBounds(playerPaddleX, previous.playerY);
  WatchyUi::Bounds currentPlayer =
      paddleBounds(playerPaddleX, current.playerY);
  WatchyUi::Bounds previousComputer =
      paddleBounds(computerPaddleX, previous.computerY);
  WatchyUi::Bounds currentComputer =
      paddleBounds(computerPaddleX, current.computerY);

  auto clearChangedActor = [&](const WatchyUi::Bounds &oldBounds,
                               const WatchyUi::Bounds &newBounds) {
    if (sameBounds(oldBounds, newBounds)) return;
    Watchy::display.fillRect(oldBounds.x, oldBounds.y, oldBounds.width,
                            oldBounds.height, background);
    Watchy::display.fillRect(newBounds.x, newBounds.y, newBounds.width,
                            newBounds.height, background);
    WatchyUi::Screen::invalidate(oldBounds);
    WatchyUi::Screen::invalidate(newBounds);
  };

  clearChangedActor(previousBall, currentBall);
  clearChangedActor(previousPlayer, currentPlayer);
  clearChangedActor(previousComputer, currentComputer);

  bool scoreChanged = previous.playerScore != current.playerScore ||
                      previous.computerScore != current.computerScore;
  if (scoreChanged) {
    Watchy::display.fillRect(scoreBounds.x, scoreBounds.y,
                            scoreBounds.width, scoreBounds.height,
                            background);
    WatchyUi::GrayPaint::fillRect(
        scoreBounds, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    WatchyUi::Screen::invalidate(scoreBounds);
  }

  bool overlayChanged = previous.paused != current.paused ||
                        previous.gameOver != current.gameOver ||
                        (current.gameOver && scoreChanged);
  if (overlayChanged) {
    Watchy::display.fillRect(overlayBounds.x, overlayBounds.y,
                            overlayBounds.width, overlayBounds.height,
                            background);
    WatchyUi::Screen::invalidate(overlayBounds);
  }

  drawPongField(foreground);
  drawPongActors(current, foreground);
  if (scoreChanged) drawPongScore(current, foreground);
  if (overlayChanged || current.paused || current.gameOver) {
    drawPongOverlay(current, background, foreground);
  }
  WatchyUi::Screen::presentDirty(APP_STATE);
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

void showPongImpl(Watchy *watchy) {
  WatchyUi::Input::begin();

  randomSeed(esp_random());
  PongState state;
  resetGame(state);
  drawPong(state);
  uint32_t lastFrame = millis();

  while (true) {
    uint32_t elapsed = millis() - lastFrame;
    uint32_t waitMs = state.paused || elapsed >= 120
                          ? state.paused ? UINT32_MAX : 0
                          : 120 - elapsed;
    WatchyUi::Event event = WatchyUi::Input::wait(waitMs);
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) watchy->showMenu(menuIndex, false);
      else WatchySdk::showMenu(menuIndex, false);
      return;
    }

    if (event == WatchyUi::Event::MENU) {
      PongState previous = state;
      if (state.gameOver) {
        resetGame(state);
      } else {
        state.paused = !state.paused;
      }
      drawPongDelta(previous, state);
      lastFrame = millis();
    }

    if (!state.paused && millis() - lastFrame >= 120) {
      PongState previous = state;
      if (WatchyUi::Input::poll() == WatchyUi::Event::UP) {
        state.playerY = clampPaddleY(state.playerY - 12);
      }
      if (WatchyUi::Input::poll() == WatchyUi::Event::DOWN) {
        state.playerY = clampPaddleY(state.playerY + 12);
      }

      uint8_t hits = updatePong(state);
      while (hits-- > 0) {
        Watchy::vibMotor(hitVibrationMs, 1);
      }
      drawPongDelta(previous, state);
      lastFrame = millis();
    }
  }
}

void Watchy::showPong() { showPongImpl(this); }

void WatchySdk::showPong() { showPongImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderPongPreview(uint8_t view) {
  PongState state{76, 105, 116, 81, 8, -4, 2, 1, 0, false, false};
  if (view == 1) {
    state.paused = true;
  } else if (view >= 2) {
    state.playerScore = view == 2 ? winningScore : 2;
    state.computerScore = view == 2 ? 3 : winningScore;
    state.gameOver = true;
  }
  drawPong(state);
}

} // namespace WatchyDemo
#endif
