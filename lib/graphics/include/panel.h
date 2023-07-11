#pragma once

#include <stddef.h>
#include "alignment.h"
#include "component.h"
#include "tigr.h"

/**
 * @brief represents a panel. a panel is a graphical entity with its own bitmap one can draw on. the panel may hold
 * multiple components, all of which will be drawn on the panel when one calls `panel_draw.
 */
struct panel {
  unsigned id;

  unsigned x_offset;
  unsigned y_offset;
  enum alignment alignment;

  Tigr *bmp;

  size_t components_amount;
  struct component *components[];
};

/**
 * @brief creates a panel with/out components. if `components` isn't 0 - expects a list of struct component *
 */
struct panel *panel_create(unsigned id,
                           unsigned x,
                           unsigned y,
                           enum alignment alignment,
                           unsigned width,
                           unsigned height,
                           size_t components,
                           ...);

/**
 * @brief adds one or more components to a panel. expects struct component *
 */
struct panel *panel_add(struct panel *restrict panel, size_t components, ...);

/**
 * @brief destroys a panel and all of its components
 */
void panel_destroy(struct panel *restrict panel);

/**
 * @brief draws all the components a panel holds
 */
void panel_draw(struct panel *restrict panel, float alpha);

/**
 * @brief clears the panel from all its assets to a color
 */
void panel_clear(struct panel *restrict panel, TPixel color);

/**
 * @brief returns the component occuping the coodinates (x, y). if no such component exists - returns NULL
 */
struct component *panel_get_component(struct panel *restrict panel, unsigned x, unsigned y);
