#pragma once

#include <stddef.h>

#include "board.h"
#include "game_properties.h"
#include "panel.h"
#include "tigr.h"

struct window {
  Tigr *bmp;

  size_t panels_amount;
  struct panel *panels[];
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

struct window *window_create(size_t width, size_t height, char *const title, int flags, size_t panels_amount, ...);

void window_destroy(struct window *window);

void draw_window(struct window *restrict window, struct game *restrict game, enum mouse_event mouse_event);

Tigr *draw_alert(char const *message);

void reveal_all_mines(struct board *restrict board);

void on_mouse_click(struct window *restrict window, struct game *restrict game, int x, int y, int buttons);

struct game game_create(enum difficulty difficulty);

void game_destroy(struct game *restrict game);

enum game_state init_new_game(struct game *restrict game, enum difficulty difficulty);
