#include "panel.h"
#include <stdarg.h>
#include <stdlib.h>

struct panel *panel_create(unsigned x_begin,
                           unsigned y_begin,
                           unsigned width,
                           unsigned height,
                           struct panel_properties properties,
                           size_t assets_amount,
                           ...) {
  struct panel *panel = malloc(sizeof *panel + assets_amount * sizeof *panel->assests);
  if (!panel) { return NULL; }

  *panel = (struct panel){.x_begin = x_begin,
                          .y_begin = y_begin,
                          .width = width,
                          .height = height,
                          .properties = properties,
                          .assets_amount = assets_amount};

  va_list args;
  va_start(args, assets_amount);

  size_t idx = 0;
  while (idx < assets_amount) {
    char *const assest_path = va_arg(args, char *);
    Tigr *asset = tigrLoadImage(assest_path);
    if (!asset) {
      tigrError(NULL, "fail to load %s", assest_path);
      panel->assets_amount--;
      continue;
    }

    panel->assests[idx] = asset;
    idx++;
  }

  va_end(args);

  return panel;
}

struct panel *panel_add_assets(struct panel *restrict panel, size_t assets_amount, ...) {
  if (!panel) return NULL;

  size_t old_assests_amount = panel->assets_amount;
  struct panel *expanded =
    realloc(panel, sizeof *panel * (assets_amount + old_assests_amount) * sizeof *panel->assests);
  if (!expanded) return panel;

  va_list args;
  va_start(args, assets_amount);

  for (size_t i = 0; i < assets_amount; i++) {
    expanded->assests[i + old_assests_amount] = va_arg(args, Tigr *);
  }

  va_end(args);

  expanded->assets_amount = old_assests_amount + assets_amount;
  return expanded;
}

void panel_destroy(struct panel *restrict panel) {
  if (!panel) return;

  for (size_t i = 0; i < panel->assets_amount; i++) {
    tigrFree(panel->assests[i]);
  }
  free(panel);
}

/* returns the starting x coordinate of a panel (the left most x point */
unsigned x_begin(struct panel const *restrict panel, size_t width) {
  return width / 2 - panel->width / 2 + panel->x_begin;
}

unsigned y_begin(struct panel const *restrict panel, size_t height) {
  (void)height;
  return /*height / 2 - */ panel->height / 2 + panel->y_begin;
}
