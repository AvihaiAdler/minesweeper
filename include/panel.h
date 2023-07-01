#pragma once

#include <stddef.h>
#include "tigr.h"

struct panel_properties {
  size_t cell_size;
  size_t spacing;
};

struct panel {
  int x_begin;
  int x_end;
  int y_begin;
  int y_end;

  struct panel_properties properties;
  size_t assets_amount;
  Tigr *assests[];
};

// creates a panel with `assets_amount` assets. expects `assets_amount` of type `char const *`
struct panel *panel_create(int x_begin,
                           int x_end,
                           int y_begin,
                           int y_end,
                           struct panel_properties properties,
                           size_t assets_amount,
                           ...);

// expand an existing panel. if `assets_amount` > `panel::assets_amount` the new assets will be added to the existing
// ones. this function is differ from panel_create in that it expects `assets_amount` of type `Tigr *`
struct panel *panel_expand(struct panel *restrict panel, size_t assets_amount, ...);

void panel_destroy(struct panel *restrict panel);
