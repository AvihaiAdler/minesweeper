#pragma once

#include <stddef.h>
#include "panel.h"
#include "tigr.h"

struct window {
  Tigr *window;

  size_t panels_amount;
  struct panel *panels[];
};

struct window *window_create(unsigned width,
                             unsigned height,
                             char const *restrict title,
                             int flags,
                             size_t panels,
                             ...);

struct window *window_add(struct window *restrict window, size_t panels, ...);

void window_destroy(struct window *restrict window);

void window_draw(struct window *restrict window, float alpha);

void window_clear(struct window *restrict window, TPixel color);

struct panel *window_get_panel(struct window *restrict window, unsigned x, unsigned y);
