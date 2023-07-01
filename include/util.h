#pragma once

#include <stddef.h>

#include "board.h"
#include "game_properties.h"
#include "panel.h"
#include "window.h"

struct window;

size_t cell_row(int y, int y_begin, size_t spacing, size_t cell_size, size_t offset);

size_t cell_col(int x, int x_begin, size_t spacing, size_t cell_size, size_t offset);

int cell_y_coord(int row, int y_begin, size_t spacing, size_t cell_size, size_t offset);

int cell_x_coord(int col, int x_begin, size_t spacing, size_t cell_size, size_t offset);

void reveal_all_mines(struct board *restrict board);

void on_mouse_click(struct window *restrict window, struct game *restrict game, int x, int y, int buttons);

struct game game_create(enum difficulty difficulty);

void game_destroy(struct game *restrict game);

enum game_state init_new_game(struct game *restrict game, enum difficulty difficulty);