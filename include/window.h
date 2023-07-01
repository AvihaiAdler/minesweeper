#pragma once
#include <stddef.h>
#include "game_properties.h"
#include "tigr.h"
#include "util.h"

enum panels {
  P_NAVBAR = 0,
  P_TOP,
  P_MAIN,
};

struct window {
  Tigr *bmp;

  size_t panels_amount;
  struct panel *panels[];
};

struct window *window_create(size_t width, size_t height, char *const title, int flags, size_t panels_amount, ...);

void window_destroy(struct window *window);

void draw_window(struct window *restrict window, struct game *restrict game, enum mouse_event mouse_event);

Tigr *draw_alert(char const *message);