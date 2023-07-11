#pragma once
#include <time.h>
#include "board.h"

enum game_state {
  STATE_INVALID,
  STATE_PLAYING,
  STATE_WON,
  STATE_LOST,
};

struct game_clock {
  time_t start;
  time_t end;
};

struct game {
  struct game_clock clock;

  struct board board;
  int mines;
  enum game_state state;
};

struct game game_create(enum difficulty difficulty);

struct game game_restart(struct game *restrict game, enum difficulty difficulty);

void game_destroy(struct game *restrict game);
