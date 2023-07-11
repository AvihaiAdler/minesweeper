#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "assets.h"
#include "board.h"
#include "game.h"
#include "mouse_event.h"
#include "panel.h"
#include "tigr.h"
#include "window.h"

unsigned calculate_width(struct board const *restrict board);

unsigned calculate_height(struct board const *restrict board);

TigrFont *load_font(char const *restrict font_path);

struct assets_manager *create_assets(struct assets_manager *am);

bool create_panels(struct panel **restrict panels, size_t size, unsigned width, unsigned height);

void reveal_mines(struct board *restrict board);

void on_mouse_click(struct window *restrict window, struct game *restrict game, struct mouse_event mouse_event);

void on_mouse_hover(struct window *restrict window, struct game *restrict game, struct mouse_event mouse_event);

void alert(TigrFont *restrict font, char const *fmt, ...);
