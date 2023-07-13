#include "window.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

struct window *window_create(unsigned width,
                             unsigned height,
                             char const *restrict title,
                             int flags,
                             size_t panels,
                             ...) {
  struct window *window = malloc(sizeof *window + panels * sizeof *window->panels);
  if (!window) return NULL;

  Tigr *bmp = tigrWindow(width, height, title ? title : "", flags);
  if (!bmp) {
    free(window);
    return NULL;
  }

  *window = (struct window){.window = bmp, .panels_amount = panels};

  va_list args;
  va_start(args, panels);

  for (size_t i = 0; i < window->panels_amount; i++) {
    window->panels[i] = va_arg(args, struct panel *);
  }

  va_end(args);
  return window;
}

struct window *window_push(struct window *restrict window, size_t panels, ...) {
  if (!window) return NULL;

  size_t old_panels = window->panels_amount;

  struct window *resized = realloc(window, sizeof *window + (old_panels + panels) * sizeof *window->panels);
  if (!resized) return window;

  va_list args;
  va_start(args, panels);

  for (size_t i = old_panels; i < old_panels + panels; i++) {
    resized->panels[i] = va_arg(args, struct panel *);
    resized->panels_amount++;
  }

  va_end(args);
  return resized;
}

struct panel *window_pop(struct window *restrict window) {
  if (!window) return NULL;

  if (window->panels_amount) {
    window->panels_amount--;
    return window->panels[window->panels_amount];
  }

  return NULL;
}

void window_destroy(struct window *restrict window) {
  if (!window) return;

  if (window->window) tigrFree(window->window);

  for (size_t i = 0; i < window->panels_amount; i++) {
    panel_destroy(window->panels[i]);
  }

  free(window);
}

// window, panel and both window::window and panel::bmp must not be NULL
static inline unsigned x_panel(struct window const *restrict window, struct panel const *restrict panel) {
  switch (panel->alignment) {
    case ALIGN_LEFT:
      return panel->x_offset;
    case ALIGN_RIGHT:
      return window->window->w - (panel->bmp->w + panel->x_offset);
    case ALIGN_CENTER:
    default:  // fallthrough
      return window->window->w / 2 - panel->bmp->w / 2 + panel->x_offset;
  }
}

// panel and panel::bmp must not be NULL
// the panel is always top aligned w.r.t to its window y axis
static inline unsigned y_panel(struct panel const *restrict panel) {
  return panel->y_offset;
}

void window_draw(struct window *restrict window, float alpha) {
  if (!window || !window->window) return;

  for (size_t i = 0; i < window->panels_amount; i++) {
    struct panel *current = window->panels[i];

    panel_draw(current, alpha);
    tigrBlitAlpha(window->window,
                  current->bmp,
                  x_panel(window, current),
                  y_panel(current),
                  0,
                  0,
                  current->bmp->w,
                  current->bmp->h,
                  alpha);

    // #ifdef MS_DEBUG
    TPixel color = tigrRGB(255, 0, 0);
    switch (i) {
      case 1:
        color = tigrRGB(0, 255, 0);
        break;
      case 2:
        color = tigrRGB(0, 0, 0);
        break;
    }
    tigrRect(window->window, x_panel(window, current), y_panel(current), current->bmp->w, current->bmp->h, color);
    // #endif
  }

  tigrUpdate(window->window);
}

void window_clear(struct window *restrict window, TPixel color) {
  if (!window || !window->window) return;

  tigrClear(window->window, color);
}

static inline bool within_panel_bounderies(struct window const *restrict window,
                                           struct panel const *restrict panel,
                                           unsigned x,
                                           unsigned y) {
  if (!window || !window->window) return false;

  if (!panel || !panel->bmp) return false;

  return x >= x_panel(window, panel) && x <= x_panel(window, panel) + panel->bmp->w && y >= y_panel(panel) &&
         y <= y_panel(panel) + panel->bmp->h;
}

struct panel *window_get_panel(struct window *restrict window, unsigned x, unsigned y) {
  if (!window || !window->window) return NULL;

  for (size_t i = 0; i < window->panels_amount; i++) {
    if (within_panel_bounderies(window, window->panels[i], x, y)) { return window->panels[i]; }
  }

  return NULL;
}

struct component *window_get_component(struct window *restrict window, unsigned x, unsigned y) {
  if (!window || !window->window) return NULL;

  struct panel *panel = window_get_panel(window, x, y);
  if (!panel) return NULL;

  return panel_get_component(panel, x - x_panel(window, panel), y - y_panel(panel));
}
