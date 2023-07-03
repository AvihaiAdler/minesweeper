#include "window.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED 240, 0, 0
#define BLACK 0, 0, 0
#define GREY 120, 120, 120
#define CELL_ALPHA 200
#define DEFAULT_BLIT_ALPHA 1.0f

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

void window_resize(struct window *restrict window, size_t width, size_t height, char const *title, int flags) {
  if (!window) return;

  Tigr *new_bmp = tigrWindow(width, height, title, flags);
  if (!new_bmp) return;

  // re-calculate the new panels offsets
  for (size_t i = 0; i < window->panels_amount; i++) {
    struct panel *current = window->panels[i];

    current->x_end = new_bmp->w - (window->bmp->w - current->x_end);
    current->y_end = new_bmp->h - (window->bmp->h - current->y_end);
  }
  tigrFree(window->bmp);
  window->bmp = new_bmp;
}

void window_destroy(struct window *window) {
  if (!window) return;

  tigrFree(window->bmp);
  for (size_t i = 0; i < window->panels_amount; i++) {
    panel_destroy(window->panels[i]);
  }

  free(window);
}

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

static inline char const *difficulty_as_str(enum difficulty difficulty) {
  switch (difficulty) {
    case MS_CLASSIC:
      return "classic";
    case MS_ADVANCED:
      return "advanced";
    case MS_EXPERT:
      return "expert";
    default:
      return "custom";
  }
}

static inline void draw_navbar_panel(Tigr *restrict bmp,
                                     struct panel const *restrict panel,
                                     struct game const *restrict game) {
  if (!bmp || !panel || !game) return;

  tigrBlitAlpha(bmp,
                panel->assests[panel->assets_amount - 1],
                panel->x_begin,
                (panel->y_end - panel->y_begin) / 2 - panel->assests[panel->assets_amount - 1]->h / 2,
                0,
                0,
                panel->assests[panel->assets_amount - 1]->w,
                panel->assests[panel->assets_amount - 1]->h,
                DEFAULT_BLIT_ALPHA);
#ifdef MS_DEBUG
  tigrRect(bmp,
           panel->x_begin,
           panel->y_begin,
           panel->x_end - panel->x_begin,
           panel->y_end - panel->y_begin,
           tigrRGB(RED));
#endif
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

#ifdef MS_DEBUG
  tigrRect(bmp,
           panel->x_begin,
           panel->y_begin,
           panel->x_end - panel->x_begin,
           panel->y_end - panel->y_begin,
           tigrRGB(RED));
#endif
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
#ifdef MS_DEBUG
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
#endif
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

#ifdef MS_DEBUG
  tigrRect(bmp,
           panel->x_begin,
           panel->y_begin,
           panel->x_end - panel->x_begin,
           panel->y_end - panel->y_begin,
           tigrRGB(RED));
#endif
}

void draw_window(struct window *restrict window, struct game *restrict game, enum mouse_event mouse_event) {
  if (!window || !game) return;

  draw_navbar_panel(window->bmp, window->panels[P_NAVBAR], game);
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
