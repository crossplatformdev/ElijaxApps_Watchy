#include <Watchy.h>
#include <esp_system.h>
#include "sdk/WatchyUi.h"

namespace {

constexpr int boardX = 10;
constexpr int boardY = 20;
constexpr int cellSize = 9;
constexpr int gridColumns = 20;
constexpr int gridRows = 20;
constexpr int maximumLength = gridColumns * gridRows;
constexpr int initialLength = 4;
constexpr uint16_t fruitVibrationMs = 100;

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

void drawSnake(const SnakeState &state) {
  const uint16_t background = WatchyUi::Theme::background();
  const uint16_t foreground = WatchyUi::Theme::foreground();

  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, 8);
  Watchy::display.print("SNAKE");
  Watchy::display.setCursor(154, 8);
  Watchy::display.print(state.score);
  Watchy::display.drawRect(boardX - 1, boardY - 1,
                           gridColumns * cellSize + 2,
                           gridRows * cellSize + 1, foreground);

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

  if (state.paused || state.gameOver) {
    Watchy::display.fillRect(49, 91, 102, 28, background);
    Watchy::display.drawRect(49, 91, 102, 28, foreground);
    Watchy::display.setCursor(state.gameOver ? 70 : 79, 102);
    Watchy::display.print(state.gameOver ? "GAME OVER" : "PAUSED");
  }

  WatchyUi::Screen::present();
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

void Watchy::showSnake() {
  WatchyUi::Input::begin();

  randomSeed(esp_random());
  SnakeState state;
  resetSnake(state);
  drawSnake(state);

  bool turnQueued = false;
  uint32_t lastMove = millis();

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
      return;
    }

    if (event == WatchyUi::Event::SELECT) {
      if (state.gameOver) {
        resetSnake(state);
      } else {
        state.paused = !state.paused;
      }
      turnQueued = false;
      drawSnake(state);
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
      MoveResult result = moveSnake(state);
      if (result == ATE_FRUIT) {
        vibMotor(fruitVibrationMs, 1);
      }
      turnQueued = false;
      drawSnake(state);
      lastMove = millis();
    }
    delay(10);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSnakePreview() {
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
  drawSnake(state);
}

} // namespace WatchyDemo
#endif