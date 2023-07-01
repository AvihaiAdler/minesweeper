#pragma once

#include "board.h"

enum game_state {
  STATE_INVALID_STATE = 0,
  STATE_LOST,
  STATE_WON,
  STATE_PLAYING,
};

enum mouse_event {
  MOUSE_UP,
  MOUSE_DOWN,
};

struct game {
  struct game_clock {
    time_t start;
    time_t end;
  } game_clock;

  struct board board;
  int mines_counter;

  enum mouse_event prev_event;
  enum difficulty difficulty;
  enum game_state game_state;
};

#define ALERT_WIDTH 200
#define ALERT_HEIGHT 150

// board color
#define BOARD_COLORS 192, 192, 192, 220
#define CELL_COLORS 130, 130, 130, 220

// margin
#define TOP_MARGIN 50
#define BOTTOM_MARGIN 10
#define LEFT_MARGIN 10
#define RIGHT_MARGIN 10
#define NAVBAR_END 20

#define CELL_SIZE 20
#define CELL_SPACING 0
#define NAVBAR_SPACING 5
