#pragma once

#include <stddef.h>
#include "alignment.h"
#include "assets.h"
#include "tigr.h"

/**
 * @brief a 'virtual entity' which represent a graphical component. a component holds 'assets'. all assets are blit
 * together on the panel when one calls `panel_draw`
 */
struct component {
  unsigned id;

  unsigned x_offset;
  unsigned y_offset;
  enum alignment alignment;

  size_t capacity;
  size_t size;
  struct asset assets[];
};

/**
 * @brief creates a component. if count isn't 0 expects a list of struct asset *
 */
struct component *component_create(unsigned id, unsigned x, unsigned y, enum alignment alignment, size_t count, ...);

/**
 * @brief destroys a component. _doesn't_ destroy the assets a component holds
 */
void component_destroy(struct component *restrict component);

/**
 * @brief adds an asset to a component
 */
struct component *component_push(struct component *restrict component, struct asset *restrict asset);

/**
 * @brief removes an asset with the id `id` from a component
 */
void component_remove(struct component *restrict component, int id);

/**
 * @brief returns the max width of a bitmap a component holds
 */
unsigned component_width(struct component const *restrict component);

/**
 * @brief returns the max height of a bitmap a component holds
 */
unsigned component_height(struct component const *restrict component);
