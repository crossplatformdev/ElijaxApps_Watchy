#include <Watchy.h>
#include <limits.h>
#include <string.h>
#include "sdk/WatchyUi.h"

namespace {

constexpr int boardSize = 8;
constexpr int cellSize = 22;
constexpr int boardX = 12;
constexpr int boardY = 22;
constexpr int boardPixels = boardSize * cellSize;
constexpr int aiSearchDepth = 4;

enum Piece : uint8_t {
  EMPTY,
  HUMAN,
  COMPUTER
};

enum GameStatus : uint8_t {
  HUMAN_TURN,
  AI_THINKING,
  HUMAN_PASSES,
  AI_PASSES,
  HUMAN_WINS,
  AI_WINS,
  DRAW_GAME
};

struct OthelloState {
  uint8_t board[boardSize * boardSize];
  uint8_t selectedMove;
  uint8_t status;
  bool gameOver;
};

constexpr int16_t positionWeights[boardSize * boardSize] = {
    120, -30, 20, 5, 5, 20, -30, 120,
    -30, -45, -5, -5, -5, -5, -45, -30,
     20,  -5, 15, 3, 3, 15,  -5,  20,
      5,  -5,  3, 3, 3,  3,  -5,   5,
      5,  -5,  3, 3, 3,  3,  -5,   5,
     20,  -5, 15, 3, 3, 15,  -5,  20,
    -30, -45, -5, -5, -5, -5, -45, -30,
    120, -30, 20, 5, 5, 20, -30, 120};

bool isInsideBoard(int row, int column) {
  return row >= 0 && row < boardSize && column >= 0 && column < boardSize;
}

uint8_t otherPlayer(uint8_t player) {
  return player == HUMAN ? COMPUTER : HUMAN;
}

bool isLegalMove(const uint8_t board[], int cell, uint8_t player) {
  if (board[cell] != EMPTY) {
    return false;
  }

  constexpr int8_t rowDirection[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  constexpr int8_t columnDirection[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  int row = cell / boardSize;
  int column = cell % boardSize;
  uint8_t opponent = otherPlayer(player);

  for (int direction = 0; direction < 8; direction++) {
    int nextRow = row + rowDirection[direction];
    int nextColumn = column + columnDirection[direction];
    bool foundOpponent = false;
    while (isInsideBoard(nextRow, nextColumn) &&
           board[nextRow * boardSize + nextColumn] == opponent) {
      foundOpponent = true;
      nextRow += rowDirection[direction];
      nextColumn += columnDirection[direction];
    }
    if (foundOpponent && isInsideBoard(nextRow, nextColumn) &&
        board[nextRow * boardSize + nextColumn] == player) {
      return true;
    }
  }
  return false;
}

int collectLegalMoves(const uint8_t board[], uint8_t player,
                      uint8_t moves[]) {
  int count = 0;
  for (int cell = 0; cell < boardSize * boardSize; cell++) {
    if (isLegalMove(board, cell, player)) {
      moves[count++] = cell;
    }
  }
  return count;
}

void applyMove(uint8_t board[], int cell, uint8_t player) {
  constexpr int8_t rowDirection[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  constexpr int8_t columnDirection[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  int row = cell / boardSize;
  int column = cell % boardSize;
  uint8_t opponent = otherPlayer(player);
  board[cell] = player;

  for (int direction = 0; direction < 8; direction++) {
    int nextRow = row + rowDirection[direction];
    int nextColumn = column + columnDirection[direction];
    int captured = 0;
    while (isInsideBoard(nextRow, nextColumn) &&
           board[nextRow * boardSize + nextColumn] == opponent) {
      captured++;
      nextRow += rowDirection[direction];
      nextColumn += columnDirection[direction];
    }
    if (captured == 0 || !isInsideBoard(nextRow, nextColumn) ||
        board[nextRow * boardSize + nextColumn] != player) {
      continue;
    }
    nextRow = row + rowDirection[direction];
    nextColumn = column + columnDirection[direction];
    while (captured-- > 0) {
      board[nextRow * boardSize + nextColumn] = player;
      nextRow += rowDirection[direction];
      nextColumn += columnDirection[direction];
    }
  }
}

int countPieces(const uint8_t board[], uint8_t player) {
  int count = 0;
  for (int cell = 0; cell < boardSize * boardSize; cell++) {
    if (board[cell] == player) {
      count++;
    }
  }
  return count;
}

int evaluateBoard(const uint8_t board[]) {
  int score = 0;
  for (int cell = 0; cell < boardSize * boardSize; cell++) {
    if (board[cell] == COMPUTER) {
      score += positionWeights[cell];
    } else if (board[cell] == HUMAN) {
      score -= positionWeights[cell];
    }
  }

  uint8_t computerMoves[boardSize * boardSize];
  uint8_t humanMoves[boardSize * boardSize];
  int computerMobility = collectLegalMoves(board, COMPUTER, computerMoves);
  int humanMobility = collectLegalMoves(board, HUMAN, humanMoves);
  score += (computerMobility - humanMobility) * 8;
  score += (countPieces(board, COMPUTER) - countPieces(board, HUMAN)) * 2;
  return score;
}

int minimax(const uint8_t board[], uint8_t player, int depth, int alpha,
            int beta) {
  uint8_t moves[boardSize * boardSize];
  int moveCount = collectLegalMoves(board, player, moves);
  uint8_t opponent = otherPlayer(player);

  if (moveCount == 0) {
    uint8_t opponentMoves[boardSize * boardSize];
    int opponentMoveCount = collectLegalMoves(board, opponent, opponentMoves);
    if (opponentMoveCount == 0) {
      int difference =
          countPieces(board, COMPUTER) - countPieces(board, HUMAN);
      return difference > 0 ? 10000 + difference
                            : difference < 0 ? -10000 + difference : 0;
    }
    if (depth == 0) {
      return evaluateBoard(board);
    }
    return minimax(board, opponent, depth - 1, alpha, beta);
  }

  if (depth == 0) {
    return evaluateBoard(board);
  }

  if (player == COMPUTER) {
    int bestScore = INT_MIN;
    for (int move = 0; move < moveCount; move++) {
      uint8_t nextBoard[boardSize * boardSize];
      memcpy(nextBoard, board, sizeof(nextBoard));
      applyMove(nextBoard, moves[move], player);
      int score = minimax(nextBoard, opponent, depth - 1, alpha, beta);
      bestScore = max(bestScore, score);
      alpha = max(alpha, score);
      if (beta <= alpha) {
        break;
      }
    }
    return bestScore;
  }

  int bestScore = INT_MAX;
  for (int move = 0; move < moveCount; move++) {
    uint8_t nextBoard[boardSize * boardSize];
    memcpy(nextBoard, board, sizeof(nextBoard));
    applyMove(nextBoard, moves[move], player);
    int score = minimax(nextBoard, opponent, depth - 1, alpha, beta);
    bestScore = min(bestScore, score);
    beta = min(beta, score);
    if (beta <= alpha) {
      break;
    }
  }
  return bestScore;
}

uint8_t chooseComputerMove(const uint8_t board[]) {
  uint8_t moves[boardSize * boardSize];
  int moveCount = collectLegalMoves(board, COMPUTER, moves);
  uint8_t bestMove = moves[0];
  int bestScore = INT_MIN;

  for (int move = 0; move < moveCount; move++) {
    uint8_t nextBoard[boardSize * boardSize];
    memcpy(nextBoard, board, sizeof(nextBoard));
    applyMove(nextBoard, moves[move], COMPUTER);
    int score = minimax(nextBoard, HUMAN, aiSearchDepth - 1, INT_MIN,
                        INT_MAX);
    if (score > bestScore) {
      bestScore = score;
      bestMove = moves[move];
    }
    yield();
  }
  return bestMove;
}

const char *statusText(uint8_t status) {
  switch (status) {
  case HUMAN_TURN:
    return "YOUR TURN";
  case AI_THINKING:
    return "AI THINK";
  case HUMAN_PASSES:
    return "YOU PASS";
  case AI_PASSES:
    return "AI PASS";
  case HUMAN_WINS:
    return "YOU WIN";
  case AI_WINS:
    return "AI WINS";
  default:
    return "DRAW";
  }
}

void drawOthello(const OthelloState &state) {
  const uint16_t foreground = WatchyUi::Theme::foreground();
  int humanPieces = countPieces(state.board, HUMAN);
  int computerPieces = countPieces(state.board, COMPUTER);

  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(2, 8);
  Watchy::display.print("Y:");
  Watchy::display.print(humanPieces);
  Watchy::display.setCursor(174, 8);
  Watchy::display.print("A:");
  Watchy::display.print(computerPieces);
  const char *label = statusText(state.status);
  Watchy::display.setCursor((DISPLAY_WIDTH - strlen(label) * 6) / 2, 8);
  Watchy::display.print(label);

  Watchy::display.drawRect(boardX, boardY, boardPixels + 1,
                           boardPixels + 1, foreground);
  for (int line = 1; line < boardSize; line++) {
    Watchy::display.drawFastVLine(boardX + line * cellSize, boardY,
                                  boardPixels, foreground);
    Watchy::display.drawFastHLine(boardX, boardY + line * cellSize,
                                  boardPixels, foreground);
  }

  for (int cell = 0; cell < boardSize * boardSize; cell++) {
    int column = cell % boardSize;
    int row = cell / boardSize;
    int centerX = boardX + column * cellSize + cellSize / 2;
    int centerY = boardY + row * cellSize + cellSize / 2;
    if (state.board[cell] == HUMAN) {
      Watchy::display.fillCircle(centerX, centerY, 8, foreground);
    } else if (state.board[cell] == COMPUTER) {
      Watchy::display.drawCircle(centerX, centerY, 8, foreground);
      Watchy::display.drawCircle(centerX, centerY, 7, foreground);
    }
  }

  if (!state.gameOver && state.status == HUMAN_TURN) {
    uint8_t moves[boardSize * boardSize];
    int moveCount = collectLegalMoves(state.board, HUMAN, moves);
    for (int move = 0; move < moveCount; move++) {
      int column = moves[move] % boardSize;
      int row = moves[move] / boardSize;
      int centerX = boardX + column * cellSize + cellSize / 2;
      int centerY = boardY + row * cellSize + cellSize / 2;
      Watchy::display.fillCircle(centerX, centerY, 2, foreground);
    }
    if (moveCount > 0) {
      int selectedCell = moves[state.selectedMove % moveCount];
      int selectedColumn = selectedCell % boardSize;
      int selectedRow = selectedCell / boardSize;
      Watchy::display.drawRect(boardX + selectedColumn * cellSize + 3,
                               boardY + selectedRow * cellSize + 3,
                               cellSize - 6, cellSize - 6, foreground);
    }
  }

  WatchyUi::Screen::present();
}

void finishGame(OthelloState &state) {
  int humanPieces = countPieces(state.board, HUMAN);
  int computerPieces = countPieces(state.board, COMPUTER);
  state.gameOver = true;
  if (humanPieces > computerPieces) {
    state.status = HUMAN_WINS;
  } else if (computerPieces > humanPieces) {
    state.status = AI_WINS;
  } else {
    state.status = DRAW_GAME;
  }
}

void runComputerPhase(OthelloState &state) {
  while (true) {
    uint8_t computerMoves[boardSize * boardSize];
    uint8_t humanMoves[boardSize * boardSize];
    int computerMoveCount =
        collectLegalMoves(state.board, COMPUTER, computerMoves);
    int humanMoveCount = collectLegalMoves(state.board, HUMAN, humanMoves);

    if (computerMoveCount == 0 && humanMoveCount == 0) {
      finishGame(state);
      drawOthello(state);
      return;
    }
    if (computerMoveCount == 0) {
      state.status = AI_PASSES;
      drawOthello(state);
      delay(700);
      state.status = HUMAN_TURN;
      state.selectedMove = 0;
      drawOthello(state);
      return;
    }

    state.status = AI_THINKING;
    drawOthello(state);
    applyMove(state.board, chooseComputerMove(state.board), COMPUTER);

    humanMoveCount = collectLegalMoves(state.board, HUMAN, humanMoves);
    computerMoveCount =
        collectLegalMoves(state.board, COMPUTER, computerMoves);
    if (humanMoveCount == 0 && computerMoveCount == 0) {
      finishGame(state);
      drawOthello(state);
      return;
    }
    if (humanMoveCount > 0) {
      state.status = HUMAN_TURN;
      state.selectedMove = 0;
      drawOthello(state);
      return;
    }

    state.status = HUMAN_PASSES;
    drawOthello(state);
    delay(700);
  }
}

void resetOthello(OthelloState &state) {
  memset(state.board, EMPTY, sizeof(state.board));
  state.board[3 * boardSize + 3] = COMPUTER;
  state.board[4 * boardSize + 4] = COMPUTER;
  state.board[3 * boardSize + 4] = HUMAN;
  state.board[4 * boardSize + 3] = HUMAN;
  state.selectedMove = 0;
  state.status = HUMAN_TURN;
  state.gameOver = false;
}

} // namespace

void Watchy::showOthello() {
  WatchyUi::Input::begin();

  OthelloState state;
  resetOthello(state);
  drawOthello(state);

  while (true) {
    uint8_t moves[boardSize * boardSize];
    int moveCount = collectLegalMoves(state.board, HUMAN, moves);
    if (!state.gameOver && moveCount == 0) {
      runComputerPhase(state);
      continue;
    }

    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
      return;
    }
    if (!state.gameOver && event == WatchyUi::Event::UP) {
      state.selectedMove = WatchyUi::ListView::previous(
          state.selectedMove, moveCount);
      drawOthello(state);
    } else if (!state.gameOver && event == WatchyUi::Event::DOWN) {
      state.selectedMove = WatchyUi::ListView::next(
          state.selectedMove, moveCount);
      drawOthello(state);
    } else if (event == WatchyUi::Event::SELECT) {
      if (state.gameOver) {
        resetOthello(state);
        drawOthello(state);
      } else {
        applyMove(state.board, moves[state.selectedMove % moveCount], HUMAN);
        runComputerPhase(state);
      }
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderOthelloPreview() {
  OthelloState state;
  resetOthello(state);
  if (isLegalMove(state.board, 19, HUMAN)) {
    applyMove(state.board, 19, HUMAN);
  }
  if (isLegalMove(state.board, 18, COMPUTER)) {
    applyMove(state.board, 18, COMPUTER);
  }
  state.selectedMove = 1;
  state.status = HUMAN_TURN;
  drawOthello(state);
}

} // namespace WatchyDemo
#endif