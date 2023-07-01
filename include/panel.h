#pragma once

#include <stddef.h>
#include "tigr.h"

// TODO: implement panel own header. each panel should contain the images it is 'drawing'
struct panel {
  int x_begin;
  int x_end;
  int y_begin;
  int y_end;

  struct panel_properties {
    size_t cell_size;
    size_t spacing;
  } properties;

  size_t assets_amount;
  Tigr *assests[];
};

struct panel *panel_create(int x_begin,
                           int x_end,
                           int y_begin,
                           int y_end,
                           struct panel_properties properties,
                           size_t assets_amount,
                           ...);

void panel_destroy(struct panel *restrict panel);
