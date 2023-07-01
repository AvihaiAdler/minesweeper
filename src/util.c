#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RED 240, 0, 0
#define BLACK 0, 0, 0
#define GREY 120, 120, 120
#define CELL_ALPHA 200
#define DEFAULT_BLIT_ALPHA 1.0f

#define LMB(X) ((X)&1)
#define RMB(X) ((X)&2)
#define MMB(X) ((X)&4)

enum panels {
  P_TOP = 0,
  P_MAIN,
};

static inline char *sprintf_wrapper(char *buf, size_t size, char const *fmt, ...) {
  if (!buf || !size) return NULL;

  va_list args;
  va_start(args, fmt);

  va_list args_cpy;
  va_copy(args_cpy, args);

  int required_size = vsnprintf(NULL, 0, fmt, args_cpy);
  va_end(args_cpy);
  if (required_size + 1 >= (int)size) {
    va_end(args);
    return NULL;
  }

  vsnprintf(buf, size, fmt, args);
  va_end(args);
  return buf;
}

struct window *window_create(size_t width, size_t height, char *const title, int flags, size_t panels_amount, ...) {
  Tigr *bmp = tigrWindow(width, height, title, flags);
  if (!bmp) return NULL;

  struct window *window = malloc(sizeof *window + panels_amount * sizeof *window->panels);
  if (!window) {
    tigrFree(bmp);
    return NULL;
  }

  window->bmp = bmp;
  window->panels_amount = panels_amount;

  va_list args;
  va_start(args, panels_amount);

  for (size_t i = 0; i < panels_amount; i++) {
    window->panels[i] = va_arg(args, struct panel *);
  }

  va_end(args);

  return window;
}

void window_destroy(struct window *window) {
  if (!window) return;

  tigrFree(window->bmp);
  for (size_t i = 0; i < window->panels_amount; i++) {
    panel_destroy(window->panels[i]);
  }

  free(window);
}

static inline Tigr *get_emojii(struct panel const *restrict panel, size_t idx) {
  if (idx >= panel->assets_amount) return NULL;

  return panel->assests[idx];
}

static inline void draw_button(Tigr *restrict bmp,
                               struct panel const *restrict panel,
                               enum game_state state,
                               enum mouse_event event) {
  if (!bmp || !panel) return;

  enum emojiis {
    EM_HAPPY,
    EM_SHOCK,
    EM_SAD,
    EM_CHAD,
    EM_BACKGROUND,
  };

  Tigr *emojii = NULL;
  switch (state) {
    case STATE_LOST:
      emojii = get_emojii(panel, EM_SAD);
      break;
    case STATE_WON:
      emojii = get_emojii(panel, EM_CHAD);
      break;
    case STATE_PLAYING:
      emojii = event == MOUSE_DOWN ? get_emojii(panel, EM_SHOCK) : get_emojii(panel, EM_HAPPY);
      break;
    default:
      break;
  }

  int width = panel->x_end - panel->x_begin;
  int height = panel->y_end - panel->y_begin;
  // background
  tigrBlitAlpha(bmp,
                panel->assests[EM_BACKGROUND],
                width / 2 - panel->assests[EM_BACKGROUND]->w / 2 + panel->x_begin,
                height / 2 - panel->assests[EM_BACKGROUND]->h / 2 + panel->y_begin,
                0,
                0,
                panel->assests[EM_BACKGROUND]->w,
                panel->assests[EM_BACKGROUND]->h,
                DEFAULT_BLIT_ALPHA);

  if (emojii) {
    tigrBlitAlpha(bmp,
                  emojii,
                  width / 2 - emojii->w / 2 + panel->x_begin,
                  height / 2 - emojii->h / 2 + panel->y_begin,
                  0,
                  0,
                  emojii->w,
                  emojii->h,
                  DEFAULT_BLIT_ALPHA);
  }
}

static inline void draw_mines_count(Tigr *restrict bmp, struct panel const *restrict panel, int leftover_mines) {
  if (!bmp || !panel) return;

  enum local_str_size { SIZE = 256 };
  char mines_as_str[SIZE];

  if (!sprintf_wrapper(mines_as_str, sizeof mines_as_str, "%02d", leftover_mines)) { strcpy(mines_as_str, "#INVALID"); }

  int text_height = tigrTextHeight(tfont, mines_as_str);

  size_t height = panel->y_end - panel->y_begin;
  tigrPrint(bmp, tfont, panel->x_begin, panel->y_begin + height / 2 - text_height / 2, tigrRGB(RED), mines_as_str);
}

static inline void draw_timer(Tigr *restrict bmp, struct panel const *restrict panel, time_t start, time_t end) {
  if (!bmp || !panel) return;

  int seconds_passed = (int)difftime(end, start);

  enum local_str_size { SIZE = 256 };
  char seconds_as_str[SIZE] = "00:00";

  sprintf_wrapper(seconds_as_str, sizeof seconds_as_str, "%02d:%02d", seconds_passed / 100, seconds_passed % 100);

  int text_height = tigrTextHeight(tfont, seconds_as_str);
  int text_width = tigrTextWidth(tfont, seconds_as_str);

  size_t height = panel->y_end - panel->y_begin;
  tigrPrint(bmp,
            tfont,
            panel->x_end - text_width,
            panel->y_begin + height / 2 - text_height / 2,
            tigrRGB(RED),
            seconds_as_str);
}

/* draws the top panel even when the game is in unplayable state */
static inline void draw_top_panel(Tigr *restrict bmp,
                                  struct panel const *restrict panel,
                                  struct game const *restrict game,
                                  enum mouse_event mouse_event) {
  if (!bmp || !panel || !game) return;

  draw_button(bmp, panel, game->game_state, mouse_event);
  draw_timer(bmp, panel, game->game_clock.start, game->game_clock.end);
  draw_mines_count(bmp, panel, game->mines_counter);
}

/* returns the left most x position of a cell based on its index */
static inline int cell_x_coord(int col, int x_begin, size_t spacing, size_t cell_size, size_t offset) {
  return col * (spacing + cell_size) + x_begin + offset;
}

/* returns the top most y position of a cell based on its index */
static inline int cell_y_coord(int row, int y_begin, size_t spacing, size_t cell_size, size_t offset) {
  return row * (spacing + cell_size) + y_begin + offset;
}

static inline TPixel cell_numeric_color(size_t adjacent_mines) {
#define NUMERIC_BLUE 65, 105, 255
#define NUMERIC_GREEN 0, 120, 0
#define NUMERIC_RED 220, 0, 0
#define NUMERIC_DARK_BLUE 25, 25, 112
#define NUMERIC_BROWN 128, 0, 0
#define NUMERIC_TEAL 32, 178, 170
#define NUMERIC_BLACK 0, 0, 0
#define NUMERIC_GREY 192, 192, 192

  switch (adjacent_mines) {
    case 1:
      return tigrRGBA(NUMERIC_BLUE, 255);
    case 2:
      return tigrRGBA(NUMERIC_GREEN, 255);
    case 3:
      return tigrRGBA(NUMERIC_RED, 255);
    case 4:
      return tigrRGBA(NUMERIC_DARK_BLUE, 255);
    case 5:
      return tigrRGBA(NUMERIC_BROWN, 255);
    case 6:
      return tigrRGBA(NUMERIC_TEAL, 255);
    case 7:
      return tigrRGBA(NUMERIC_BLACK, 255);
    case 8:
      return tigrRGBA(NUMERIC_GREY, 255);
    default:
      return tigrRGBA(NUMERIC_BLACK, 255);
  }
}

/* only draws the main panel if the game is in playable state */
void draw_main_panel(Tigr *restrict bmp, struct panel const *restrict panel, struct game *restrict game) {
  if (!bmp || !panel) return;
  if (!game || game->game_state == STATE_INVALID_STATE) return;

  enum asssets {
    AS_FLAG,
    AS_MINE,
    AS_CELL,
  };
  /**
   * each row contains: col_mines * (cell_size + spacing) > x.end - x.begin
   * x = j * (cell_size + spacing) + x.begin + spacing / 2
   *
   * same goes for y:
   * row_mines * (cell_size + spacing) > y.end - y.begin
   * y = i * (cell_size + spacing) + y.begin + spacing / 2
   */

  size_t offset = panel->properties.spacing / 2;
  for (int i = 0; i < (int)board_rows(&game->board); i++) {
    for (int j = 0; j < (int)board_cols(&game->board); j++) {
      int x = cell_x_coord(j, panel->x_begin, panel->properties.spacing, panel->properties.cell_size, offset);
      int y = cell_y_coord(i, panel->y_begin, panel->properties.spacing, panel->properties.cell_size, offset);

      struct cell *cell = board_get_cell(&game->board, i, j);
      if (!cell) continue;

      if (cell->revealed && cell->mine) {  // mine was stepped on
        // cell border
        tigrRect(bmp, x, y, panel->properties.cell_size, panel->properties.cell_size, tigrRGBA(GREY, CELL_ALPHA));

        // cell
        tigrBlitAlpha(bmp,
                      panel->assests[AS_MINE],
                      x,
                      y,
                      0,
                      0,
                      panel->assests[AS_MINE]->w,
                      panel->assests[AS_MINE]->h,
                      DEFAULT_BLIT_ALPHA);
      } else if (cell->revealed) {
        if (cell->adjacent_mines) {  // print the numeric value of a cell if the value is 0 - simply draw an 'empty cell
          enum local_str_size { SIZE = 5 };
          char adjacent_mines_as_str[SIZE];

          if (!sprintf_wrapper(adjacent_mines_as_str, sizeof adjacent_mines_as_str, "%zu", cell->adjacent_mines)) {
            strcpy(adjacent_mines_as_str, "!");
          }

          int text_width = tigrTextWidth(tfont, adjacent_mines_as_str);
          int text_height = tigrTextHeight(tfont, adjacent_mines_as_str);
          if (text_height % 2 == 0) text_height--;

          // cell numeric value
          tigrPrint(bmp,
                    tfont,
                    x + panel->properties.cell_size / 2 - text_width / 2,
                    y + panel->properties.cell_size / 2 - text_height / 2,
                    cell_numeric_color(cell->adjacent_mines),
                    adjacent_mines_as_str);
        }

        // cell border
        tigrRect(bmp, x, y, panel->properties.cell_size, panel->properties.cell_size, tigrRGBA(GREY, CELL_ALPHA));
      } else {  // regular cell
        tigrBlitAlpha(bmp,
                      panel->assests[AS_CELL],
                      x,
                      y,
                      0,
                      0,
                      panel->assests[AS_CELL]->w,
                      panel->assests[AS_CELL]->h,
                      DEFAULT_BLIT_ALPHA);

        // debug
        // enum local_str_size { SIZE = 5 };
        // char adjacent_mines_as_str[SIZE];

        // if (!sprintf_wrapper(adjacent_mines_as_str, sizeof adjacent_mines_as_str, "%zu", cell->adjacent_mines)) {
        //   strcpy(adjacent_mines_as_str, "!");
        // }

        // int text_width = tigrTextWidth(tfont, adjacent_mines_as_str);
        // int text_height = tigrTextHeight(tfont, adjacent_mines_as_str);
        // if (text_height % 2 == 0) text_height--;

        // // cell numeric value
        // tigrPrint(bmp,
        //           tfont,
        //           x + panel->properties.cell_size / 2 - text_width / 2,
        //           y + panel->properties.cell_size / 2 - text_height / 2,
        //           cell_numeric_color(cell->adjacent_mines),
        //           adjacent_mines_as_str);
        // end debug

        if (cell->flagged) {  // cell is flagged - blit the flag on top of the cell
          tigrBlitAlpha(bmp,
                        panel->assests[AS_FLAG],
                        x,
                        y,
                        0,
                        0,
                        panel->assests[AS_FLAG]->w,
                        panel->assests[AS_FLAG]->h,
                        DEFAULT_BLIT_ALPHA);
        }
      }
    }
  }
}

void draw_window(struct window *restrict window, struct game *restrict game, enum mouse_event mouse_event) {
  if (!window || !game) return;

  draw_top_panel(window->bmp, window->panels[P_TOP], game, mouse_event);
  draw_main_panel(window->bmp, window->panels[P_MAIN], game);
}

Tigr *draw_alert(char const *message) {
  if (!message) return NULL;

  Tigr *alert_window = tigrWindow(ALERT_WIDTH, ALERT_HEIGHT, "alert", TIGR_AUTO);
  if (!alert_window) return NULL;

  int text_width = tigrTextWidth(tfont, message);
  int text_height = tigrTextHeight(tfont, message);
  tigrPrint(alert_window,
            tfont,
            alert_window->w / 2 - text_width / 2,
            alert_window->h / 2 + text_height / 2,
            tigrRGB(BLACK),
            message);

  return alert_window;
}

static inline bool reset_pressed(struct panel const *restrict top, int x, int y, int buttons) {
  if (!top) return false;
  if (!LMB(buttons)) { return false; }

  int mid_width = top->x_end / 2 - top->x_begin / 2 + top->x_begin;
  int mid_height = top->y_end / 2 - top->y_begin / 2 + top->y_begin;
  return x >= mid_width - (int)top->properties.cell_size / 2 && x <= mid_width + (int)top->properties.cell_size / 2 &&
         y >= mid_height - (int)top->properties.cell_size / 2 && y <= mid_height + (int)top->properties.cell_size / 2;
}

// get the row of a cell based on its y position
static inline size_t cell_row(int y, int y_begin, size_t spacing, size_t cell_size, size_t offset) {
  return (y - y_begin - offset) / (cell_size + spacing);
}

// get the column of a cell based on its x potision
static inline size_t cell_col(int x, int x_begin, size_t spacing, size_t cell_size, size_t offset) {
  return (x - x_begin - offset) / (cell_size + spacing);
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
  // size_t i = (y - panel->y_begin - offset) / (panel->properties.cell_size + panel->properties.spacing);
  size_t row = cell_row(y, panel->y_begin, panel->properties.spacing, panel->properties.cell_size, offset);
  // size_t j = (x - panel->x_begin - offset) / (panel->properties.cell_size + panel->properties.spacing);
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

static inline bool in_panel(int x, int y, struct panel const *restrict panel) {
  if (!panel) return false;
  return x >= panel->x_begin && x <= panel->x_end && y >= panel->y_begin && y <= panel->y_end;
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

void on_mouse_click(struct window *restrict window, struct game *restrict game, int x, int y, int buttons) {
  if (!window || !game) return;

  if (!buttons) return;

  // prevent the case of 'holding mouse down'
  if (game->prev_event == MOUSE_DOWN) return;

  // clicked the top panel
  if (in_panel(x, y, window->panels[P_TOP]) && reset_pressed(window->panels[P_TOP], x, y, buttons)) {
    // init_new_game
    game->game_state = init_new_game(game, game->difficulty);
    return;
  }

  // clicked the main panel
  if (in_panel(x, y, window->panels[P_MAIN])) {
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
      // game->game_state = reveal_adjacent_cells(&game->board, row, col);
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
    if (board_revealed_cells(&game->board) && game->game_state == STATE_PLAYING) { game->game_state = STATE_WON; }
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
   * game::difficulty && game::prev_event are initialized in game_create
   */
  game->game_clock = (struct game_clock){.start = time(NULL), .end = (time_t)-1};
  game->game_state = STATE_PLAYING;
  game->prev_event = MOUSE_UP;
  game->mines_counter = board_get_mines_amount(&game->board);

  return STATE_PLAYING;
}
