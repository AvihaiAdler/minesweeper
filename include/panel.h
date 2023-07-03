#pragma once

#include <stddef.h>
#include "tigr.h"

struct panel_properties {
  size_t cell_size;
  size_t spacing;
};

/**
 * offsets are the _fixed_ offset from the window border. *_begin & *_end changes based on window size
 */
struct panel {
  unsigned x_begin;
  unsigned x_end;
  unsigned y_begin;
  unsigned y_end;

  struct panel_properties properties;
  size_t assets_amount;
  Tigr *assests[];
};

// creates a panel with `assets_amount` assets. expects `assets_amount` of type `char const *`
struct panel *panel_create(unsigned x_begin,
                           unsigned x_end,
                           unsigned y_begin,
                           unsigned y_end,
                           struct panel_properties properties,
                           size_t assets_amount,
                           ...);

// expand an existing panel. if `assets_amount` > `panel::assets_amount` the new assets will be added to the existing
// ones. this function is differ from panel_create in that it expects `assets_amount` of type `Tigr *`
struct panel *panel_expand(struct panel *restrict panel, size_t assets_amount, ...);

void panel_destroy(struct panel *restrict panel);
