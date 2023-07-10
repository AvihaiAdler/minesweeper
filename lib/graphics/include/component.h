#pragma once

#include "alignment.h"
#include "tigr.h"

/**
 * @brief a 'virtual entity' which represent a graphical component.
 *  component::bmp must be free'd extenrally (to save the creation of multiple copies of the same bitmaps. in other
 * words - multiple components can share the same bitmap)
 */
struct component {
  unsigned id;

  unsigned x_offset;
  unsigned y_offset;
  enum alignment alignment;

  Tigr *bmp;
};

// bmp can be NULL. in this case an 'empty' component will be created. such a component must be supplied with a Tigr *
// later in order to be 'visible'
struct component component_create(unsigned id, unsigned x, unsigned y, Tigr *bmp, enum alignment alignment);

void component_blit(struct component *restrict component, Tigr *bmp, float alpha);

void component_clear(struct component *restrict component, TPixel color);
