#include "WatchyUi.h"
#include <esp_system.h>
#include "AppDisplay.h"
#include "Watchy.h"

namespace {

constexpr int boardX = 10;
constexpr int boardY = 20;
constexpr int cellSize = 9;
constexpr int gridColumns = 20;
constexpr int gridRows = 20;
constexpr int maximumLength = gridColumns * gridRows;
constexpr int initialLength = 4;
constexpr uint16_t fruitVibrationMs = 100;
constexpr WatchyUi::Bounds snakeScoreBounds{170, 0, 26, 14};
constexpr WatchyUi::Bounds snakeOverlayBounds{49, 91, 102, 28};

enum Direction : uint8_t {
  RIGHT,
  UP,
  LEFT,
  DOWN
};

enum MoveResult : uint8_t {
  MOVED,
  ATE_FRUIT,
  COLLIDED
};

struct SnakeState {
  uint8_t bodyX[maximumLength];
  uint8_t bodyY[maximumLength];
  uint16_t length;
  uint16_t score;
  uint8_t direction;
  uint8_t fruitX;
  uint8_t fruitY;
  bool paused;
  bool gameOver;
};

struct SnakeVisualSnapshot {
  uint8_t tailX;
  uint8_t tailY;
  uint8_t fruitX;
  uint8_t fruitY;
  uint16_t score;
  bool paused;
  bool gameOver;
};

SnakeVisualSnapshot visualSnapshot(const SnakeState &state) {
  uint16_t tail = state.length > 0 ? state.length - 1 : 0;
  return {state.bodyX[tail], state.bodyY[tail], state.fruitX,
          state.fruitY, state.score, state.paused, state.gameOver};
}

WatchyUi::Bounds snakeCellBounds(uint8_t x, uint8_t y) {
  return {static_cast<int16_t>(boardX + x * cellSize),
          static_cast<int16_t>(boardY + y * cellSize),
          cellSize, cellSize};
}

bool snakeOccupies(const SnakeState &state, uint8_t x, uint8_t y,
                   uint16_t segmentCount) {
  for (uint16_t segment = 0; segment < segmentCount; segment++) {
    if (state.bodyX[segment] == x && state.bodyY[segment] == y) {
      return true;
    }
  }
  return false;
}

bool placeFruit(SnakeState &state) {
  if (state.length >= maximumLength) {
    return false;
  }

  int start = random(maximumLength);
  for (int offset = 0; offset < maximumLength; offset++) {
    int cell = (start + offset) % maximumLength;
    uint8_t x = cell % gridColumns;
    uint8_t y = cell / gridColumns;
    if (!snakeOccupies(state, x, y, state.length)) {
      state.fruitX = x;
      state.fruitY = y;
      return true;
    }
  }
  return false;
}

void resetSnake(SnakeState &state) {
  state.length = initialLength;
  state.score = 0;
  state.direction = RIGHT;
  state.paused = false;
  state.gameOver = false;
  for (int segment = 0; segment < initialLength; segment++) {
    state.bodyX[segment] = 10 - segment;
    state.bodyY[segment] = 10;
  }
  placeFruit(state);
}

void drawSnakeBorder(uint16_t foreground) {
  Watchy::display.drawRect(boardX - 1, boardY - 1,
                           gridColumns * cellSize + 2,
                           gridRows * cellSize + 1, foreground);
}

void drawSnakeScore(const SnakeState &state, uint16_t foreground) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foreground);
  char scoreText[8];
  snprintf(scoreText, sizeof(scoreText), "%u", state.score);
  Watchy::display.setCursor(176, AppVisual::centeredCursorY(9, scoreText));
  Watchy::display.print(scoreText);
}

void drawSnakeActors(const SnakeState &state, uint16_t foreground) {
  for (uint16_t segment = 0; segment < state.length; segment++) {
    int x = boardX + state.bodyX[segment] * cellSize + 1;
    int y = boardY + state.bodyY[segment] * cellSize + 1;
    Watchy::display.fillRect(x, y, cellSize - 2, cellSize - 2,
                             foreground);
  }

  int fruitCenterX = boardX + state.fruitX * cellSize + cellSize / 2;
  int fruitCenterY = boardY + state.fruitY * cellSize + cellSize / 2;
  Watchy::display.drawCircle(fruitCenterX, fruitCenterY, 3, foreground);
  Watchy::display.fillCircle(fruitCenterX, fruitCenterY, 1, foreground);
}

void drawSnakeOverlay(const SnakeState &state, uint16_t background,
                      uint16_t foreground) {
  if (!state.paused && !state.gameOver) return;
  Watchy::display.fillRect(snakeOverlayBounds.x, snakeOverlayBounds.y,
                           snakeOverlayBounds.width,
                           snakeOverlayBounds.height, background);
  WatchyUi::GrayPaint::fillRoundRect(
      snakeOverlayBounds, 4,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::outline(snakeOverlayBounds, foreground);
  WatchyUi::Canvas::centeredText(snakeOverlayBounds,
                                 state.gameOver ? "GAME OVER" : "PAUSED", 1,
                                 foreground);
}

void drawSnake(const SnakeState &state) {
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();

  WatchyUi::Screen::beginCanvas();
  WatchyUi::GrayPaint::fillRect(
      {0, 0, DISPLAY_WIDTH, 18},
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, AppVisual::centeredCursorY(9, "SNAKE"));
  Watchy::display.print("SNAKE");
  Watchy::display.setCursor(140, AppVisual::centeredCursorY(9, "SCORE"));
  Watchy::display.print("SCORE");
  drawSnakeScore(state, foreground);
  drawSnakeBorder(foreground);
  drawSnakeActors(state, foreground);
  drawSnakeOverlay(state, background, foreground);

  WatchyUi::Screen::present();
}

void drawSnakeOverlayDelta(const SnakeState &state) {
  uint16_t background = WatchyUi::Theme::background();
  uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.fillRect(snakeOverlayBounds.x, snakeOverlayBounds.y,
                           snakeOverlayBounds.width,
                           snakeOverlayBounds.height, background);
  drawSnakeBorder(foreground);
  drawSnakeActors(state, foreground);
  drawSnakeOverlay(state, background, foreground);
  WatchyUi::Screen::invalidate(snakeOverlayBounds);
  WatchyUi::Screen::presentDirty(APP_STATE);
}

void drawSnakeMoveDelta(const SnakeVisualSnapshot &previous,
                        const SnakeState &current, MoveResult result) {
  uint16_t background = WatchyUi::Theme::background();
  uint16_t foreground = WatchyUi::Theme::foreground();

  if (result != COLLIDED) {
    WatchyUi::Bounds head =
        snakeCellBounds(current.bodyX[0], current.bodyY[0]);
    Watchy::display.fillRect(head.x, head.y, head.width, head.height,
                            background);
    WatchyUi::Screen::invalidate(head);

    if (result == MOVED) {
      WatchyUi::Bounds tail =
          snakeCellBounds(previous.tailX, previous.tailY);
      Watchy::display.fillRect(tail.x, tail.y, tail.width, tail.height,
                              background);
      WatchyUi::Screen::invalidate(tail);
    } else {
      WatchyUi::Bounds previousFruit =
          snakeCellBounds(previous.fruitX, previous.fruitY);
      WatchyUi::Bounds currentFruit =
          snakeCellBounds(current.fruitX, current.fruitY);
      Watchy::display.fillRect(previousFruit.x, previousFruit.y,
                              previousFruit.width, previousFruit.height,
                              background);
      Watchy::display.fillRect(currentFruit.x, currentFruit.y,
                              currentFruit.width, currentFruit.height,
                              background);
      WatchyUi::Screen::invalidate(previousFruit);
      WatchyUi::Screen::invalidate(currentFruit);
    }
  }

  bool scoreChanged = previous.score != current.score;
  if (scoreChanged) {
    Watchy::display.fillRect(snakeScoreBounds.x, snakeScoreBounds.y,
                            snakeScoreBounds.width,
                            snakeScoreBounds.height, background);
    WatchyUi::GrayPaint::fillRect(
        snakeScoreBounds, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    WatchyUi::Screen::invalidate(snakeScoreBounds);
  }

  bool overlayChanged = previous.paused != current.paused ||
                        previous.gameOver != current.gameOver;
  if (overlayChanged) {
    Watchy::display.fillRect(snakeOverlayBounds.x, snakeOverlayBounds.y,
                            snakeOverlayBounds.width,
                            snakeOverlayBounds.height, background);
    WatchyUi::Screen::invalidate(snakeOverlayBounds);
  }

  drawSnakeBorder(foreground);
  drawSnakeActors(current, foreground);
  if (scoreChanged) drawSnakeScore(current, foreground);
  if (overlayChanged || current.paused || current.gameOver) {
    drawSnakeOverlay(current, background, foreground);
  }
  WatchyUi::Screen::presentDirty(APP_STATE);
}

MoveResult moveSnake(SnakeState &state) {
  constexpr int8_t deltaX[] = {1, 0, -1, 0};
  constexpr int8_t deltaY[] = {0, -1, 0, 1};
  int nextX = state.bodyX[0] + deltaX[state.direction];
  int nextY = state.bodyY[0] + deltaY[state.direction];

  if (nextX < 0 || nextX >= gridColumns || nextY < 0 ||
      nextY >= gridRows) {
    state.gameOver = true;
    state.paused = true;
    return COLLIDED;
  }

  bool eating = nextX == state.fruitX && nextY == state.fruitY;
  if (eating && state.length >= maximumLength) {
    state.gameOver = true;
    state.paused = true;
    return COLLIDED;
  }
  uint16_t collisionSegments = eating ? state.length : state.length - 1;
  if (snakeOccupies(state, nextX, nextY, collisionSegments)) {
    state.gameOver = true;
    state.paused = true;
    return COLLIDED;
  }

  uint16_t nextLength = eating ? state.length + 1 : state.length;
  for (uint16_t segment = nextLength - 1; segment > 0; segment--) {
    if (segment < state.length) {
      state.bodyX[segment] = state.bodyX[segment - 1];
      state.bodyY[segment] = state.bodyY[segment - 1];
    } else {
      state.bodyX[segment] = state.bodyX[state.length - 1];
      state.bodyY[segment] = state.bodyY[state.length - 1];
    }
  }
  state.bodyX[0] = nextX;
  state.bodyY[0] = nextY;

  if (!eating) {
    return MOVED;
  }

  state.length = nextLength;
  state.score++;
  if (!placeFruit(state)) {
    state.gameOver = true;
    state.paused = true;
  }
  return ATE_FRUIT;
}

} // namespace

void showSnakeImpl(Watchy *watchy) {
  WatchyUi::Input::begin();

  randomSeed(esp_random());
  SnakeState state;
  resetSnake(state);
  drawSnake(state);

  bool turnQueued = false;
  uint32_t lastMove = millis();

  while (true) {
    uint32_t elapsed = millis() - lastMove;
    uint32_t waitMs = state.paused || elapsed >= 180
                          ? state.paused ? UINT32_MAX : 0
                          : 180 - elapsed;
    WatchyUi::Event event = WatchyUi::Input::wait(waitMs);
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) watchy->showMenu(menuIndex, false);
      else WatchySdk::showMenu(menuIndex, false);
      return;
    }

    if (event == WatchyUi::Event::MENU) {
      if (state.gameOver) {
        resetSnake(state);
        drawSnake(state);
      } else {
        state.paused = !state.paused;
        drawSnakeOverlayDelta(state);
      }
      turnQueued = false;
      lastMove = millis();
    }

    if (!state.paused && !turnQueued && event == WatchyUi::Event::UP) {
      state.direction = (state.direction + 1) % 4;
      turnQueued = true;
    } else if (!state.paused && !turnQueued &&
               event == WatchyUi::Event::DOWN) {
      state.direction = (state.direction + 3) % 4;
      turnQueued = true;
    }

    if (!state.paused && millis() - lastMove >= 180) {
      SnakeVisualSnapshot previous = visualSnapshot(state);
      MoveResult result = moveSnake(state);
      if (result == ATE_FRUIT) {
        Watchy::vibMotor(fruitVibrationMs, 1);
      }
      turnQueued = false;
      drawSnakeMoveDelta(previous, state, result);
      lastMove = millis();
    }
  }
}

void Watchy::showSnake() { showSnakeImpl(this); }

void WatchySdk::showSnake() { showSnakeImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSnakePreview(uint8_t view) {
  SnakeState state{};
  state.length = 8;
  state.score = 4;
  state.direction = UP;
  state.fruitX = 15;
  state.fruitY = 6;
  state.paused = false;
  state.gameOver = false;
  const uint8_t bodyX[] = {11, 11, 11, 10, 9, 8, 7, 6};
  const uint8_t bodyY[] = {7, 8, 9, 9, 9, 9, 9, 9};
  memcpy(state.bodyX, bodyX, sizeof(bodyX));
  memcpy(state.bodyY, bodyY, sizeof(bodyY));
  state.paused = view == 1;
  state.gameOver = view >= 2;
  drawSnake(state);
}

} // namespace WatchyDemo
#endif
