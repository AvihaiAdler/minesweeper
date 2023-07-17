#pragma once

#include <stddef.h>
#include "panel.h"
#include "tigr.h"

/**
 * @brief a graphical object with its own bitmap capabale of holding multiple panels. all previous mentioned panels will
 * be drawn on the window when one calls `window_draw`
 */
struct window {
  Tigr *window;

  size_t panels_amount;
  struct panel *panels[];
};

/**
 * @brief creates a window with/without panels. if `panels` isn't 0 - expects a list of `struct panel *`
 */
struct window *window_create(unsigned width,
                             unsigned height,
                             char const *restrict title,
                             int flags,
                             size_t panels,
                             ...);

/**
 * @brief adds panels into the window. expects a list of `struct panel *`
 */
struct window *window_push(struct window *restrict window, size_t panels, ...);

/**
 * @brief pops the last panel and returns it
 */
struct panel *window_pop(struct window *restrict window);

/**
 * @brief destroys the window and all of its panels
 */
void window_destroy(struct window *restrict window);

/**
 * @brief draws all the panels onto the window
 */
void window_draw(struct window *restrict window, float alpha);

/**
 * @brief clears the window to a color
 */
void window_clear(struct window *restrict window, TPixel color);

struct panel *window_panel_at(struct window *restrict window, unsigned idx);

/**
 * @brief returns the panel with the occupping the coordinates (x, y). if no such panel exists - returns `NULL`
 */
struct panel *window_get_panel(struct window *restrict window, unsigned x, unsigned y);

/**
 * @brief get a component based on its x, y values
 */
struct component *window_get_component(struct window *restrict window, unsigned x, unsigned y);

/**
 * @brief get the left most x value of a panel
 */
unsigned window_x_panel(struct window const *restrict window, struct panel const *restrict panel);

/**
 * @brief get the top most y value of a panel
 */
unsigned window_y_panel(struct panel const *restrict panel);
