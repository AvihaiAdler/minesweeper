#include "component.h"
#include "alignment.h"

struct component component_create(unsigned id, unsigned x, unsigned y, Tigr *bmp, enum alignment alignment) {
  return (struct component){.id = id, .x_offset = x, .y_offset = y, .alignment = alignment, .bmp = bmp};
}

void component_blit(struct component *restrict component, Tigr *bmp, float alpha) {
  if (!component || !component->bmp) return;

  if (!bmp) return;

  tigrBlitAlpha(component->bmp, bmp, 0, 0, 0, 0, -1, -1, alpha);
}

void component_clear(struct component *restrict component, TPixel color) {
  if (!component || !component->bmp) return;

  tigrClear(component->bmp, color);
}
