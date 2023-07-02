#include "util.h"
#include <stdlib.h>
#include <time.h>
#include "window.h"

#define LMB(X) ((X)&1)
#define RMB(X) ((X)&2)
#define MMB(X) ((X)&4)

// get the row of a cell based on its y position
size_t cell_row(int y, int y_begin, size_t spacing, size_t cell_size, size_t offset) {
  return (y - y_begin - offset) / (cell_size + spacing);
}

// get the column of a cell based on its x potision
size_t cell_col(int x, int x_begin, size_t spacing, size_t cell_size, size_t offset) {
  return (x - x_begin - offset) / (cell_size + spacing);
}

/* returns the left most x position of a cell based on its index */
int cell_x_coord(int col, int x_begin, size_t spacing, size_t cell_size, size_t offset) {
  return col * (spacing + cell_size) + x_begin + offset;
}

/* returns the top most y position of a cell based on its index */
int cell_y_coord(int row, int y_begin, size_t spacing, size_t cell_size, size_t offset) {
  return row * (spacing + cell_size) + y_begin + offset;
}

size_t difficulty_index(enum difficulty difficulty) {
  return (unsigned)difficulty >> OCTET * 3;
}

// get the cell at position (x,y) on the panel
static inline struct cell *get_cell(struct board *restrict board, int x, int y, struct panel const *restrict panel) {
  if (!board || !panel) return NULL;

  if (x < panel->x_begin || x > panel->x_end) return NULL;
  if (y < panel->y_begin || y > panel->y_end) return NULL;

  /**
   * each row contains: col_mines * (cell_size + spacing) > x.end - x.begin
   * x = j * (cell_size + spacing) + x.begin + spacing / 2
   *
   * inverse:
   * x - offset - x.begin = j * (spacing + cell_size)
   * (x - offset - x.begin) / (spacing + cell_size) = j
   *
   * same goes for y:
   * row_mines * (cell_size + spacing) > y.end - y.begin
   * y = i * (cell_size + spacing) + y.begin + spacing / 2
   *
   * inverse:
   * y - offset - y.begin = i * (spacing + cell_size)
   * (y - offset - y.begin) / (spacing + cell size) = i
   */

  size_t offset = panel->properties.spacing / 2;
  size_t row = cell_row(y, panel->y_begin, panel->properties.spacing, panel->properties.cell_size, offset);
  size_t col = cell_col(x, panel->x_begin, panel->properties.spacing, panel->properties.cell_size, offset);

  if ((size_t)y > cell_y_coord(row, panel->y_begin, panel->properties.spacing, panel->properties.cell_size, offset) +
                    panel->properties.cell_size) {
    return NULL;
  }

  if ((size_t)x > cell_y_coord(col, panel->x_begin, panel->properties.spacing, panel->properties.cell_size, offset) +
                    panel->properties.cell_size) {
    return NULL;
  }

  return board_get_cell(board, row, col);
}

static inline bool reset_pressed(struct panel const *restrict top, int x, int y, int buttons) {
  if (!top) return false;
  if (!LMB(buttons)) { return false; }

  int mid_width = top->x_end / 2 - top->x_begin / 2 + top->x_begin;
  int mid_height = top->y_end / 2 - top->y_begin / 2 + top->y_begin;
  return x >= mid_width - (int)top->properties.cell_size / 2 && x <= mid_width + (int)top->properties.cell_size / 2 &&
         y >= mid_height - (int)top->properties.cell_size / 2 && y <= mid_height + (int)top->properties.cell_size / 2;
}

int panel_index(int x, int y, struct window *restrict window) {
  for (size_t i = 0; i < window->panels_amount; i++) {
    struct panel *current = window->panels[i];

    if (x >= current->x_begin && x <= current->x_end && y >= current->y_begin && y <= current->y_end) { return i; }
  }

  return -1;
}

void reveal_all_mines(struct board *restrict board) {
  if (!board) return;

  for (size_t row = 0; row < board_rows(board); row++) {
    for (size_t col = 0; col < board_cols(board); col++) {
      struct cell *cell = board_get_cell(board, row, col);
      if (cell && cell->mine) cell->revealed = true;
    }
  }
}

static inline size_t sum_adjacent_flags(struct cell const *restrict cells,
                                        size_t row,
                                        size_t col,
                                        size_t rows,
                                        size_t cols) {
  size_t prev_row = row - 1;
  size_t last_row = row + 1;
  size_t prev_col = col - 1;
  size_t last_col = col + 1;

  if (row == 0) { prev_row = row; }
  if (row == rows - 1) { last_row = row; }

  if (col == 0) { prev_col = col; }
  if (col == cols - 1) { last_col = col; }

  size_t adjacent_flags = 0;
  for (size_t curr_row = prev_row; curr_row <= last_row; curr_row++) {
    for (size_t curr_col = prev_col; curr_col <= last_col; curr_col++) {
      if (cells[curr_row * cols + curr_col].flagged) adjacent_flags++;
    }
  }
  return adjacent_flags;
}

static inline void reveal_next_cell(struct board *restrict board, int row, int col, enum game_state *state) {
  if (!board || !board->cells) return;

  if (!state || *state == STATE_LOST) return;

  // boundry check
  int rows = board_rows(board);
  int cols = board_cols(board);
  if (row < 0 || row >= rows) return;
  if (col < 0 || col >= cols) return;

  struct cell *current_cell = board_get_cell(board, row, col);
  if (!current_cell) return;
  if (current_cell->revealed || current_cell->flagged) return;

  board_reveal_cell(board, row, col);
  if (current_cell->mine) *state = STATE_LOST;

  size_t adjacent_flags = sum_adjacent_flags(board->cells, row, col, rows, cols);
  if (adjacent_flags < current_cell->adjacent_mines) return;

  // try to go up
  reveal_next_cell(board, row - 1, col, state);
  // try to go top right
  reveal_next_cell(board, row - 1, col + 1, state);
  // try to go right
  reveal_next_cell(board, row, col + 1, state);
  // try to go bottom right
  reveal_next_cell(board, row + 1, col + 1, state);
  // try to go down
  reveal_next_cell(board, row + 1, col, state);
  // try to go bottom left
  reveal_next_cell(board, row + 1, col - 1, state);
  // try to go left
  reveal_next_cell(board, row, col - 1, state);
  // try to go top left
  reveal_next_cell(board, row - 1, col - 1, state);
}

static inline void reveal_all_cells(struct board *restrict board) {
  if (!board) return;

  for (size_t i = 0; i < board_rows(board); i++) {
    for (size_t j = 0; j < board_cols(board); j++) {
      struct cell *cell = board_get_cell(board, i, j);
      if (cell) cell->revealed = true;
    }
  }
}

static inline bool difficulty_pressed(int x, int y, struct panel const *restrict panel) {
  return x >= panel->x_begin && x <= panel->x_begin + panel->assests[panel->assets_amount - 1]->w &&
         y >= panel->y_begin + panel->assests[panel->assets_amount - 1]->h / 2 &&
         y <= panel->y_begin + panel->assests[panel->assets_amount - 1]->h +
                panel->assests[panel->assets_amount - 1]->h / 2;
}

void on_mouse_click(struct window *restrict window, struct game *restrict game, int x, int y, int buttons) {
  if (!window || !game) return;

  if (!buttons) return;

  // prevent the case of 'holding mouse down'
  if (game->prev_event == MOUSE_DOWN) return;

  enum panels p_idx = panel_index(x, y, window);

  switch (p_idx) {
    case P_NAVBAR: {
      enum difficulty new_difficulty = difficulties[(difficulty_index(game->difficulty) + 1) % 3];
      game->game_state = init_new_game(game, new_difficulty);

    } break;
    case P_TOP: {
      game->game_state = init_new_game(game, game->difficulty);
      break;
      case P_MAIN:
        if (game->game_state != STATE_PLAYING) { return; }

        struct cell *current_cell = get_cell(&game->board, x, y, window->panels[P_MAIN]);
        if (!current_cell) return;

        struct panel *panel = window->panels[P_MAIN];
        size_t offset = panel->properties.spacing / 2;
        size_t row = cell_row(y, panel->y_begin, panel->properties.spacing, panel->properties.cell_size, offset);
        size_t col = cell_col(x, panel->x_begin, panel->properties.spacing, panel->properties.cell_size, offset);

        if (MMB(buttons)) {  // click on a revealed cell
          if (!current_cell->revealed) return;

          // discover all cells in range
          // questionable _at best_ but hey! it works!
          game->board.revealed_cells--;
          current_cell->revealed = false;
          reveal_next_cell(&game->board, row, col, &game->game_state);
        } else if (LMB(buttons)) {
          if (current_cell->flagged) { return; }

          if (current_cell->mine) { game->game_state = STATE_LOST; }

          if (!current_cell->mine && !current_cell->adjacent_mines) {  // cell has a numeric value of 0
            reveal_next_cell(&game->board, row, col, &game->game_state);
          } else {
            board_reveal_cell(&game->board, row, col);
          }

        } else if (RMB(buttons)) {
          current_cell->flagged = !current_cell->flagged;
          current_cell->flagged ? game->mines_counter-- : game->mines_counter++;
        }

        // check win condition
        if (board_revealed_cells(&game->board) && game->game_state == STATE_PLAYING) {
          game->game_state = STATE_WON;
          game->mines_counter = 0;
        }
    } break;
    default:
      break;
  }
}

struct game game_create(enum difficulty difficulty) {
  struct board board;
  if (!board_create(&board, difficulty)) { return (struct game){.game_state = STATE_INVALID_STATE}; }

  return (
    struct game){.board = board, .game_clock = {-1, -1}, .difficulty = difficulty, .game_state = STATE_INVALID_STATE};
}

void game_destroy(struct game *restrict game) {
  if (!game) return;
  board_destroy(&game->board);
}

enum game_state init_new_game(struct game *restrict game, enum difficulty difficulty) {
  if (!game) return STATE_INVALID_STATE;

  if (!board_init(&game->board, difficulty)) {
    game->game_state = STATE_INVALID_STATE;
    return STATE_INVALID_STATE;
  }

  /**
   * game::prev_event are initialized in game_create
   */
  game->game_clock = (struct game_clock){.start = time(NULL), .end = (time_t)-1};
  game->game_state = STATE_PLAYING;
  game->prev_event = MOUSE_UP;
  game->difficulty = difficulty;
  game->mines_counter = board_get_mines_amount(&game->board);

  return STATE_PLAYING;
}
