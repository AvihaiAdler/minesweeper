#pragma once

#include <stddef.h>
#include "alignment.h"
#include "component.h"
#include "tigr.h"

struct panel {
  unsigned id;

  unsigned x_offset;
  unsigned y_offset;
  enum alignment alignment;

  Tigr *bmp;

  size_t components_amount;
  struct component components[];
};

struct panel *panel_create(unsigned id,
                           unsigned x,
                           unsigned y,
                           enum alignment alignment,
                           unsigned width,
                           unsigned height,
                           size_t components,
                           ...);

struct panel *panel_add(struct panel *restrict panel, size_t components, ...);

void panel_destroy(struct panel *restrict panel);

void panel_draw(struct panel *restrict panel, float alpha);

/**
 * @brief clears the panel from all its assets to a color
 *
 * @param panel
 * @param color the color to clear the panel to
 */
void panel_clear(struct panel *restrict panel, TPixel color);

struct component *panel_get_component(struct panel *restrict panel, unsigned x, unsigned y);
