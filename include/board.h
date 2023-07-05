#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define OCTET 8

struct cell {
  bool mine;
  bool flagged;
  bool revealed;
  size_t adjacent_mines;
};

// difficulty packed values accross bytes, each value gets its own byte.
// obviously - the system must have sizeof(int) == 4. index | col | rows | number of mines
enum difficulty {
  MS_CLASSIC = 9 << OCTET * 2 | 9 << OCTET | 10,
  MS_ADVANCED = 1 << OCTET * 3 | 16 << OCTET * 2 | 16 << OCTET | 40,
  MS_EXPERT = 2 << OCTET * 3 | 30 << OCTET * 2 | 16 << OCTET | 99,
  MS_DIFFICULTIES = 3,
};

struct board {
  enum difficulty difficulty;

  size_t revealed_cells;

  struct cell *cells;
};

/* difficulty _must_ be one of the enum values above. otherwise - its potential UB */
bool board_create(struct board *restrict board, enum difficulty difficulty);

void board_destroy(struct board *restrict board);

bool generate_mines(struct board *restrict board);

bool board_init(struct board *restrict board, enum difficulty difficulty);

int board_get_mines_amount(struct board *restrict board);

size_t board_rows(struct board *restrict board);

size_t board_cols(struct board *restrict board);

bool board_revealed_cells(struct board *restrict board);

void board_reveal_cell(struct board *restrict board, size_t row, size_t col);

struct cell *board_get_cell(struct board *restrict board, size_t row, size_t col);

extern enum difficulty difficulties[MS_DIFFICULTIES];
