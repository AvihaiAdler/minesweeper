#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "game_properties.h"
#include "panel.h"
#include "tigr.h"

enum panels {
  P_NAVBAR = 0,
  P_TOP,
  P_MAIN,
};

struct window {
  Tigr *bmp;
  bool navbar_toggled;

  size_t panels_amount;
  struct panel *panels[];
};

struct window *window_create(size_t width, size_t height, char *const title, int flags, size_t panels_amount, ...);

void window_resize(struct window *restrict window, size_t width, size_t height, char const *title, int flags);

void window_destroy(struct window *window);

void draw_window(struct window *restrict window, struct game *restrict game, enum mouse_event mouse_event);

void draw_alert(char const *message);

size_t calculate_window_width(struct board *restrict board);

size_t calculate_window_height(struct board *restrict board);
