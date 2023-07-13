#include "component.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "alignment.h"

#define INIT_CAPACITY 8
#define GROWTH_FACTOR 1

struct component *component_create(unsigned id, unsigned x, unsigned y, enum alignment alignment, size_t count, ...) {
  size_t capacity = INIT_CAPACITY;
  if (count > capacity) capacity = count;

  struct component *component = malloc(sizeof *component * sizeof *component->assets * capacity);
  if (!component) return NULL;

  *component = (struct component){.id = id, .x_offset = x, .y_offset = y, .alignment = alignment, .capacity = capacity};

  va_list args;
  va_start(args, count);

  for (size_t i = 0; i < count; i++) {
    memcpy(&component->assets[i], va_arg(args, struct asset *), sizeof *component->assets);
  }

  va_end(args);

  component->size = count;
  return component;
}

void component_destroy(struct component *restrict component) {
  if (!component) return;

  free(component);
}

static inline bool resize(struct component *restrict *component) {
  struct component *tmp = *component;

  size_t capacity = tmp->capacity << GROWTH_FACTOR;
  if (capacity < tmp->capacity) return false;  // overflowed

  struct component *resized = realloc(tmp, sizeof *resized + sizeof *tmp->assets * capacity);
  if (!resized) return false;

  resized->capacity = capacity;
  *component = resized;
  return true;
}

struct component *component_push(struct component *restrict component, struct asset *restrict asset) {
  if (!component || !asset) goto push_end;

  if (component->capacity <= component->size) {
    if (!resize(&component)) goto push_end;
  }

  component->assets[component->size] = *asset;
  component->size++;

push_end:
  return component;
}

void component_pop(struct component *restrict component) {
  if (!component) return;

  if (component->size) component->size--;
}

void component_remove(struct component *restrict component, int id) {
  if (!component) return;

  for (size_t i = 0; i < component->size; i++) {
    if (component->assets[i].id == id) {
      component->assets[i] = component->assets[component->size - 1];
      component->size--;
      break;
    }
  }
}

unsigned component_width(struct component const *restrict component) {
  if (!component || !component->size) return 0;

  unsigned width = component->assets[0].bmp->w;
  for (size_t i = 1; i < component->size; i++) {
    if ((unsigned)component->assets[i].bmp->w > width) width = component->assets[i].bmp->w;
  }

  return width;
}

unsigned component_height(struct component const *restrict component) {
  if (!component || !component->size) return 0;

  unsigned height = component->assets[0].bmp->h;
  for (size_t i = 1; i < component->size; i++) {
    if ((unsigned)component->assets[i].bmp->h > height) height = component->assets[i].bmp->h;
  }

  return height;
}
