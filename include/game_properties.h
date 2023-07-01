#pragma once

enum game_state {
  STATE_INVALID_STATE = 0,
  STATE_LOST,
  STATE_WON,
  STATE_PLAYING,
};

enum game_mode {
  MODE_BEGINNER,
  MODE_INTERMEDIATE,
  MODE_ADVANCED,
};

enum mouse_event {
  MOUSE_UP,
  MOUSE_DOWN,
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
#define SPACING 0
