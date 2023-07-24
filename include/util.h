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

TigrFont *load_font(char const *restrict font_path);

struct assets_manager *create_assets(struct assets_manager *restrict am, TigrFont *restrict font);

struct window *create_window(struct game *restrict game, struct assets_manager *restrict am, TigrFont *restrict font);

void reveal_mines(struct board *restrict board);

void draw_window(struct window *restrict window,
                 struct game *restrict game,
                 struct assets_manager *restrict am,
                 TigrFont *restrict font);

void on_mouse_click(struct window *restrict window,
                    struct game *restrict game,
                    struct assets_manager *restrict am,
                    TigrFont *restrict font,
                    struct mouse_event mouse_event);

void on_mouse_hover(struct window *restrict window,
                    struct game *restrict game,
                    struct assets_manager *restrict am,
                    TigrFont *restrict font,
                    struct mouse_event mouse_event);

void alert(TigrFont *restrict font, char const *fmt, ...);
