#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "tigr.h"

/**
 * @brief represent an asset. an asset has an id and an immutable bitmap. ref_count represent the number of 'things'
 * holding the asset. do note that that number _isn't_ accurate and shouldn't be relied upon. multiple components might
 * 'borrow' an asset only to be destroyed afterwards without calling `asset_return`. this number however can be relied
 * opun if one is looking for an asset 'free' (an asset no one holds yet)
 */
struct asset {
  int id;
  unsigned ref_count;
  Tigr *bmp;
};

/**
 * @brief manages all the assets. a simple vector holding `struct asset`s. yes a hash table would be better, no i'm not
 * using one
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
struct assets_manager *am_push(struct assets_manager *restrict assets_manager, struct asset asset);

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

struct asset *am_get_at(struct assets_manager *restrict assets_manager, unsigned index);

/**
 * @brief returns the first asset with id `id` whos `ref_count` is 0
 */
struct asset *am_get_free(struct assets_manager *restrict assets_manager, int id);

void am_return(struct assets_manager *restrict assets_manager, struct asset *restrict asset);

/**
 * @brief creates an asset. if id is -1 - the assets_manager will assign it with its own 'unique' id
 */
struct asset asset_create(int id, Tigr *restrict bmp);
