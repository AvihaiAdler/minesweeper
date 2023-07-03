#include "panel.h"
#include <stdarg.h>
#include <stdlib.h>

struct panel *panel_create(unsigned x_begin,
                           unsigned x_end,
                           unsigned y_begin,
                           unsigned y_end,
                           struct panel_properties properties,
                           size_t assets_amount,
                           ...) {
  struct panel *panel = malloc(sizeof *panel + assets_amount * sizeof *panel->assests);
  if (!panel) { return NULL; }

  *panel = (struct panel){.x_begin = x_begin,
                          .x_end = x_end,
                          .y_begin = y_begin,
                          .y_end = y_end,
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

struct panel *panel_expand(struct panel *restrict panel, size_t assets_amount, ...) {
  if (!panel) return NULL;

  size_t old_assests_amount = panel->assets_amount;
  struct panel *expanded = realloc(panel, sizeof *panel * assets_amount * sizeof *panel->assests);
  if (!expanded) return panel;

  va_list args;
  va_start(args, assets_amount);

  for (size_t i = old_assests_amount; i < assets_amount; i++) {
    expanded->assests[i] = va_arg(args, Tigr *);
  }

  va_end(args);

  return expanded;
}

void panel_destroy(struct panel *restrict panel) {
  if (!panel) return;

  for (size_t i = 0; i < panel->assets_amount; i++) {
    tigrFree(panel->assests[i]);
  }
  free(panel);
}
