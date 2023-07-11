#include "assets.h"
#include <stdarg.h>
#include <stdlib.h>

#define INIT_CAPACITY 16
#define GROWTH_FACTOR 1

static inline void asset_destroy(struct asset *restrict asset) {
  if (!asset) return;

  tigrFree(asset->bmp);
}

struct assets_manager *am_create(size_t count, ...) {
  if (!count) return NULL;

  size_t capacity = INIT_CAPACITY;
  if (count > capacity) { capacity = count; }

  struct assets_manager *assets = malloc(sizeof *assets + sizeof *assets->assets * capacity);
  if (!assets) return NULL;

  *assets = (struct assets_manager){.capacity = capacity, .size = count};

  va_list args;
  va_start(args, count);

  int id = 0;
  for (size_t i = 0; i < count; i++) {
    char const *asset_path = va_arg(args, char const *);
    if (!asset_path) continue;

    Tigr *bmp = tigrLoadImage(asset_path);
    if (!bmp) continue;

    assets->assets[i] = (struct asset){.id = id, .bmp = bmp};
    id++;
  }

  va_end(args);
  return assets;
}

void am_destroy(struct assets_manager *restrict assets_manager) {
  if (!assets_manager) return;

  for (size_t i = 0; i < assets_manager->size; i++) {
    asset_destroy(&assets_manager->assets[i]);
  }

  free(assets_manager);
}

static inline bool resize(struct assets_manager *restrict *assets_manager) {
  struct assets_manager *tmp = *assets_manager;

  size_t capacity = tmp->capacity << GROWTH_FACTOR;
  if (capacity < tmp->capacity) return false;

  struct assets_manager *resized = realloc(tmp, sizeof *resized + sizeof *tmp->assets * capacity);
  if (!resized) return false;

  resized->capacity = capacity;
  *assets_manager = resized;
  return true;
}

struct assets_manager *am_push(struct assets_manager *restrict assets_manager, struct asset *restrict asset) {
  if (!assets_manager || !asset) goto push_end;

  if (assets_manager->capacity <= assets_manager->size) {
    if (!resize(&assets_manager)) goto push_end;
  }

  assets_manager->assets[assets_manager->size] = *asset;
  assets_manager->size++;

push_end:
  return assets_manager;
}

void am_pop(struct assets_manager *restrict assets_manager) {
  if (!assets_manager) return;

  if (assets_manager->size) assets_manager->size--;
}

void am_remove(struct assets_manager *restrict assets_manager, int id) {
  if (!assets_manager) return;

  for (size_t i = 0; i < assets_manager->size; i++) {
    if (assets_manager->assets[i].id == id) {
      asset_destroy(&assets_manager->assets[i]);
      assets_manager->assets[i] = assets_manager->assets[assets_manager->size - 1];
      assets_manager->size--;
      break;
    }
  }
}

struct asset *am_get(struct assets_manager *restrict assets_manager, int id) {
  if (!assets_manager) return NULL;

  struct asset *needle = NULL;
  for (size_t i = 0; i < assets_manager->size; i++) {
    if (assets_manager->assets[i].id == id) {
      needle = &assets_manager->assets[i];
      break;
    }
  }

  return needle;
}
