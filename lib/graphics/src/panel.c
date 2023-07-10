#include "panel.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

struct panel *panel_create(unsigned id,
                           unsigned x,
                           unsigned y,
                           enum alignment alignment,
                           unsigned width,
                           unsigned height,
                           size_t components,
                           ...) {
  struct panel *panel = malloc(sizeof *panel + components * sizeof *panel->components);
  if (!panel) return NULL;

  Tigr *bmp = tigrBitmap(width, height);
  if (!bmp) {
    free(panel);
    return NULL;
  }

  *panel = (struct panel){.id = id,
                          .x_offset = x,
                          .y_offset = y,
                          .alignment = alignment,
                          .bmp = bmp,
                          .components_amount = components};

  va_list args;
  va_start(args, components);

  for (size_t i = 0; i < components; i++) {
    panel->components[i] = va_arg(args, struct component);
  }

  va_end(args);
  return panel;
}

struct panel *panel_add(struct panel *restrict panel, size_t components, ...) {
  if (!panel) return NULL;

  size_t old_components = panel->components_amount;

  struct panel *resized = realloc(panel, sizeof *panel + (old_components + components) * sizeof *panel->components);
  if (!resized) return panel;

  va_list args;
  va_start(args, components);

  for (size_t i = old_components; i < components; i++) {
    resized->components[i] = va_arg(args, struct component);
  }

  va_end(args);
  return resized;
}

void panel_destroy(struct panel *restrict panel) {
  if (!panel) return;

  if (panel->bmp) tigrFree(panel->bmp);

  for (size_t i = 0; i < panel->components_amount; i++) {
    component_destroy(&panel->components[i]);
  }
}

// panel, component and their internal bmps must not be NULL
static inline unsigned x_component(struct panel const *restrict panel, struct component const *restrict component) {
  switch (component->alignment) {
    case ALIGN_LEFT:
      return component->x_offset;
    case ALIGN_RIGHT:
      return panel->bmp->w - (component->bmp->w + component->x_offset);
    case ALIGN_CENTER:
    default:  // fallthrough
      return panel->bmp->w / 2 - (component->bmp->w / 2 + component->x_offset);
  }
}

// panel, component and their internal bmps must not be NULL
// componenet is _always_ center aligned w.r.t the y axis of a panel
static inline unsigned y_component(struct panel const *restrict panel, struct component const *restrict component) {
  return panel->bmp->h / 2 - (component->bmp->h / 2 + component->y_offset);
}

void panel_draw(struct panel *restrict panel, float alpha) {
  if (!panel || !panel->bmp) return;

  for (size_t i = 0; i < panel->components_amount; i++) {
    struct component *current = &panel->components[i];

    // 'empty' component
    if (!current->bmp) continue;

    tigrBlitAlpha(panel->bmp,
                  current->bmp,
                  x_component(panel, current),
                  y_component(panel, current),
                  0,
                  0,
                  current->bmp->w,
                  current->bmp->h,
                  alpha);
  }
}

void panel_clear(struct panel *restrict panel, TPixel color) {
  if (!panel || !panel->bmp) return;

  tigrClear(panel->bmp, color);
}

static inline bool within_component_boundries(struct panel const *restrict panel,
                                              struct component const *restrict component,
                                              unsigned x,
                                              unsigned y) {
  if (!panel || !panel->bmp) return false;

  if (!component || !component->bmp) return false;

  return x >= x_component(panel, component) && x <= x_component(panel, component) + component->bmp->w &&
         y >= y_component(panel, component) && y <= y_component(panel, component) + component->bmp->h;
}

struct component *panel_get_component(struct panel *restrict panel, unsigned x, unsigned y) {
  if (!panel || !panel->bmp) return NULL;

  for (size_t i = 0; i < panel->components_amount; i++) {
    struct component *current = &panel->components[i];

    if (within_component_boundries(panel, current, x, y)) { return current; }
  }
  return NULL;
}
