#include "board.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*generates a random value between [min, max) */
static inline int rng(int min, int max) {
  int range = max - (min + 1);

  double rand_val = rand() / (1.0 + RAND_MAX);
  return (int)(rand_val * range + min);
}

static inline struct cell cell(struct cell *cells, size_t row, size_t col, size_t rows_bound, size_t cols_bound) {
  // the cell at (row, col) is a mine
  if (cells[row * cols_bound + col].mine) return cells[row * cols_bound + col];

  size_t prev_row = row - 1;
  size_t last_row = row + 1;
  size_t prev_col = col - 1;
  size_t last_col = col + 1;

  if (row == 0) { prev_row = row; }
  if (row == rows_bound - 1) { last_row = row; }

  if (col == 0) { prev_col = col; }
  if (col == cols_bound - 1) { last_col = col; }

  size_t adjacent_mines = 0;
  for (size_t curr_row = prev_row; curr_row <= last_row; curr_row++) {
    for (size_t curr_col = prev_col; curr_col <= last_col; curr_col++) {
      if (cells[curr_row * cols_bound + curr_col].mine) adjacent_mines++;
    }
  }

  return (struct cell){.adjacent_mines = adjacent_mines};
}

static inline bool set_cells_values(struct board *restrict board) {
  if (!board || !board->cells) return false;

  for (size_t row = 0; row < board_rows(board); row++) {
    for (size_t col = 0; col < board_cols(board); col++) {
      board->cells[row * board_cols(board) + col] = cell(board->cells, row, col, board_rows(board), board_cols(board));
    }
  }

  return true;
}

bool board_create(struct board *restrict board, enum difficulty difficulty) {
  if (!board) return false;

  size_t rows = (difficulty >> OCTET * 2) & 0xff;
  size_t cols = (difficulty >> OCTET) & 0xff;

  struct cell *cells = malloc(sizeof *cells * rows * cols);
  if (!cells) {
    *board = (struct board){0};
    return false;
  }

  *board = (struct board){.difficulty = difficulty, .revealed_cells = 0, .cells = cells};
  return true;
}

void board_destroy(struct board *restrict board) {
  if (!board || !board->cells) return;

  free(board->cells);
}

bool generate_mines(struct board *restrict board) {
  if (!board || !board->cells) return false;

  int total_mines = board_mines(board);
  int i = 0;
  while (i < total_mines) {
    size_t row = rng(0, board_rows(board));
    size_t col = rng(0, board_cols(board));

    if (!board->cells[row * board_cols(board) + col].mine) {
      board->cells[row * board_cols(board) + col] = (struct cell){.mine = true};
      i++;
    }
  }

  return true;
}

bool board_init(struct board *restrict board, enum difficulty difficulty) {
  if (!board || !board->cells) return false;

  // board changed size / mines
  if (difficulty != board->difficulty) {
    board_destroy(board);
    if (!board_create(board, difficulty)) { return false; }
  }

  // board remains the same
  // reset all cells
  memset(board->cells, 0, sizeof *board->cells * board_rows(board) * board_cols(board));
  board->revealed_cells = 0;

  if (!generate_mines(board)) return false;
  if (!set_cells_values(board)) return false;

  return true;
}

unsigned board_mines(struct board const *restrict board) {
  if (!board) return 0;
  return board->difficulty & 0xff;
}

size_t board_rows(struct board const *restrict board) {
  if (!board) return 0;

  return (board->difficulty >> OCTET) & 0xff;
}

size_t board_cols(struct board const *restrict board) {
  if (!board) return 0;

  return (board->difficulty >> OCTET * 2) & 0xff;
}

bool board_revealed_cells(struct board *restrict board) {
  if (!board) return false;
  return board->revealed_cells == board_rows(board) * board_cols(board) - board_mines(board);
}

void board_reveal_cell(struct board *restrict board, size_t row, size_t col) {
  if (!board) return;

  if (row >= board_rows(board) || col >= board_cols(board)) return;

  board->cells[row * board_cols(board) + col].revealed = true;
  board->revealed_cells++;
}

struct cell *board_cell(struct board *restrict board, size_t row, size_t col) {
  if (!board || !board->cells) return NULL;
  if (row >= board_rows(board) || col >= board_cols(board)) return NULL;

  return &board->cells[row * board_cols(board) + col];
}
