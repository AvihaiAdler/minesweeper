#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "tigr.h"

/**
 * @brief represent an asset. an asset has an id and an immutable bitmap
 */
struct asset {
  int id;
  Tigr *bmp;
};

/**
 * @brief manages all the assets. a simple vector holding `struct asset`s
 */
struct assets_manager {
  size_t capacity;
  size_t size;
  struct asset assets[];
};

/**
 * @brief creates an assets_manage with/without assets. if `count` isn't `0` expects a list of `char const *` (paths to
 * `count` assets)
 */
struct assets_manager *am_create(size_t count, ...);

/**
 * @brief destroys an asset_manager and all of its asset. this should be the _only_ way to free assets
 */
void am_destroy(struct assets_manager *restrict assets_manager);

/**
 * @brief pushes an asset into assets
 */
struct assets_manager *am_push(struct assets_manager *restrict assets_manager, struct asset *restrict asset);

/**
 * @brief pops the last asset in assets
 */
void am_pop(struct assets_manager *restrict assets_manager);

/**
 * @brief removes the assets with the id `id`
 */
void am_remove(struct assets_manager *restrict assets_manager, int id);

/**
 * @brief returns a ptr to the desired asset or NULL. said asset must not be free'd
 */
struct asset *am_get(struct assets_manager *restrict assets_manager, int id);
