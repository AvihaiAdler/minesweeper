#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include "colors.h"
#include "properties.h"

#define ALERT_WIDTH 350
#define ALERT_HEIGHT 200

static enum difficulty difficulties[] = {MS_CLASSIC, MS_ADVANCED, MS_EXPERT};

enum stats_panel_components {
  SC_MINES_COUNTER,
  SC_BUTTON,
  SC_CLOCK,
};

static char *vsprintf_wrapper(char *buf, size_t size, char const *fmt, va_list args) {
  if (!buf || !size) return NULL;

  va_list args_cpy;
  va_copy(args_cpy, args);

  int required_size = vsnprintf(NULL, 0, fmt, args_cpy);
  va_end(args_cpy);

  if (required_size + 1 >= (int)size) { return NULL; }

  vsprintf(buf, fmt, args);
  return buf;
}

static char *sprintf_wrapper(char *buf, size_t size, char const *fmt, ...) {
  if (!buf || !size) return NULL;

  va_list args;
  va_start(args, fmt);

  if (!vsprintf_wrapper(buf, size, fmt, args)) {
    va_end(args);
    return NULL;
  }

  va_end(args);
  return buf;
}

static char const *difficulties_as_str(enum difficulty difficulty) {
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

static void print_difficulty(Tigr *restrict bmp, TigrFont *restrict font, enum difficulty difficulty) {
  if (!bmp) return;

  if (!font) font = tfont;

  tigrClear(bmp, tigrRGB(192, 192, 192));

  tigrPrint(bmp, font, 0, 0, tigrRGB(BLACK), difficulties_as_str(difficulty));
}

static unsigned max_text_width(TigrFont *restrict font) {
  enum local_str_size {
    SIZE = 64,
  };
  char text[SIZE];

  unsigned width = 0;
  for (size_t i = 0; i < MS_DIFFICULTIES; i++) {
    if (!sprintf_wrapper(text, sizeof text, "%s", difficulties_as_str(difficulties[i]))) continue;

    unsigned tmp_width = tigrTextWidth(font, text);
    if (width < tmp_width) width = tmp_width;
  }

  return width;
}

static unsigned max_text_height(TigrFont *restrict font) {
  return tigrTextHeight(font, "a");
}

static unsigned window_width(void) {
  return (MS_EXPERT >> (OCTET * 2)) * TILE_SIZE + LEFT_MARGIN + RIGHT_MARGIN;
}

static unsigned window_height(void) {
  return ((MS_EXPERT >> OCTET) & 0xff) * TILE_SIZE + HEIGHT_NAV_PANE + HEIGHT_STAT_PANE;
}

static unsigned panel_width(struct board *restrict board) {
#ifdef DEBUG
#include <stdio.h>
  printf("panel width: %zu\n", board_cols(board));
#endif
  return board_cols(board) * TILE_SIZE;
}

static unsigned panel_height(struct board *restrict board) {
#ifdef DEBUG
#include <stdio.h>
  printf("panel height: %zu\n", board_rows(board));
#endif
  return board_rows(board) * TILE_SIZE;
}

TigrFont *load_font(char const *restrict font_path) {
  if (!font_path) return NULL;

  Tigr *font_png = tigrLoadImage(font_path);
  if (!font_png) return NULL;

  return tigrLoadFont(font_png, TCP_ASCII);
}

static TPixel numeric_color(int num) {
  switch (num) {
    case 1:
      return tigrRGB(NUMERIC_BLUE);
    case 2:
      return tigrRGB(NUMERIC_GREEN);
    case 3:
      return tigrRGB(NUMERIC_RED);
    case 4:
      return tigrRGB(NUMERIC_DARK_BLUE);
    case 5:
      return tigrRGB(NUMERIC_BROWN);
    case 6:
      return tigrRGB(NUMERIC_TEAL);
    case 7:
      return tigrRGB(NUMERIC_BLACK);
    case 8:
      return tigrRGB(NUMERIC_GREY);
    default:
      return tigrRGB(NUMERIC_BLACK);
  }
}

static Tigr *create_clock_asset(TigrFont *restrict font) {
  enum local_str_size {
    SIZE = 64,
  };
  char clock[SIZE];

  if (!sprintf_wrapper(clock, sizeof clock, "%02d:%02d", 0, 0)) return NULL;

  unsigned width = tigrTextWidth(font, clock);
  unsigned height = tigrTextHeight(font, clock);

  return tigrBitmap(width, height > TILE_SIZE ? height : TILE_SIZE);
}

static Tigr *create_mines_asset(TigrFont *restrict font) {
  enum local_str_size {
    SIZE = 64,
  };
  char clock[SIZE];

  if (!sprintf_wrapper(clock, sizeof clock, "%02d", 0)) return NULL;

  unsigned width = tigrTextWidth(font, clock);
  unsigned height = tigrTextHeight(font, clock);

  return tigrBitmap(width, height > TILE_SIZE ? height : TILE_SIZE);
}

struct assets_manager *create_assets(struct assets_manager *restrict am, TigrFont *restrict font) {
  if (!am) return NULL;

  if (!font) font = tfont;

  // create numeral assets 0 - 8
  for (int i = ASSET_ZERO; i < ASSET_EIGHT + 1; i++) {
    Tigr *bmp = tigrBitmap(TILE_SIZE, TILE_SIZE);
    if (!bmp) return am;

    tigrClear(bmp, tigrRGBA(BOARD_COLOR));
    tigrRect(bmp, 0, 0, bmp->w, bmp->h, tigrRGB(GREY));

    enum local_str_size {
      SIZE = 256,
    };
    char numeral[SIZE];

    if (i - ASSET_ZERO) {
      if (!sprintf_wrapper(numeral, sizeof numeral, "%d", i - ASSET_ZERO)) continue;
      tigrPrint(bmp,
                tfont,
                bmp->w / 2 - tigrTextWidth(tfont, numeral) / 2,
                bmp->h / 2 - tigrTextHeight(tfont, numeral) / 2,
                numeric_color(i - ASSET_ZERO),
                numeral);
    }

#ifdef DEBUG
    if (i - ASSET_ZERO == 0) {
      if (!sprintf_wrapper(numeral, sizeof numeral, "%d", i - ASSET_ZERO)) continue;
      tigrPrint(bmp,
                tfont,
                bmp->w / 2 - tigrTextWidth(tfont, numeral) / 2,
                bmp->h / 2 - tigrTextHeight(tfont, numeral) / 2,
                numeric_color(i - ASSET_ZERO),
                numeral);
    }
#endif

    am = am_push(am, asset_create(i, bmp));
  }

  // create an asset for the clock
  Tigr *clock_asset = create_clock_asset(font);
  if (!clock_asset) return am;

  am = am_push(am, asset_create(ASSET_CLOCK, clock_asset));

  // create an asset for the mine counter
  Tigr *mines_asset = create_mines_asset(font);
  if (!mines_asset) return am;

  am = am_push(am, asset_create(ASSET_MINES_COUNTER, mines_asset));

  // create 3 emtpy assets for the menu
  for (unsigned i = 0; i < MS_DIFFICULTIES; i++) {
    Tigr *bmp = tigrBitmap(max_text_width(font), max_text_height(font));
    if (!bmp) continue;

    am = am_push(am, asset_create(ASSET_EMPTY, bmp));
  }

  return am;
}

static struct panel *create_navbar(struct assets_manager *restrict am, size_t width, size_t height) {
  if (!am) return NULL;

  struct component *humburger = component_create(0, 0, 0, ALIGN_LEFT, 1, am_get_at(am, ASSET_HUMBURGER));
  if (!humburger) return NULL;

  return panel_create(PANEL_NAVBAR, 0, 0, ALIGN_CENTER, width, height, 1, humburger);
}

// assumes component creation never fails
static struct panel *create_stats_panel(struct assets_manager *restrict am, size_t width, size_t height) {
  if (!am) return NULL;

  return panel_create(
    PANEL_STATS,
    0,
    HEIGHT_NAV_PANE,
    ALIGN_CENTER,
    width,
    height,
    3,
    component_create(SC_MINES_COUNTER, 0, 0, ALIGN_LEFT, 1, am_get_at(am, ASSET_MINES_COUNTER)),
    component_create(SC_BUTTON, 0, 0, ALIGN_CENTER, 2, am_get_at(am, ASSET_TILE), am_get_at(am, ASSET_HAPPY)),
    component_create(SC_CLOCK, 0, 0, ALIGN_RIGHT, 1, am_get_at(am, ASSET_CLOCK)));
}

// assumes component creation never fails
static struct panel *create_main_panel(struct assets_manager *restrict am,
                                       struct game *restrict game,
                                       size_t width,
                                       size_t height) {
  if (!am || !game) return NULL;

  struct panel *panel =
    panel_create(PANEL_BOARD, 0, HEIGHT_NAV_PANE + HEIGHT_STAT_PANE, ALIGN_CENTER, width, height, 0);
  if (!panel) return NULL;

  unsigned offset = 0;  // (panel->bmp->w - board_cols(&game->board) * TILE_SIZE) / 2;
  for (size_t row = 0; row < board_rows(&game->board); row++) {
    for (size_t col = 0; col < board_cols(&game->board); col++) {
      unsigned pos = row * board_cols(&game->board) + col;

      panel = panel_add(
        panel,
        1,
        component_create(pos, col * TILE_SIZE + offset, row * TILE_SIZE, ALIGN_LEFT, 1, am_get_at(am, ASSET_TILE)));
    }
  }

  return panel;
}

static struct panel *create_menu(struct assets_manager *restrict *am,
                                 TigrFont *restrict font,
                                 size_t width,
                                 unsigned x_offset) {
  if (!am) return NULL;

  unsigned menu_width = max_text_width(font);
  unsigned menu_height = max_text_height(font);

  if (menu_width > width) return NULL;

  struct panel *panel = panel_create(PANEL_MENU, x_offset, TILE_SIZE, ALIGN_LEFT, menu_width, menu_height * 3, 0);
  if (!panel) return NULL;
  panel_clear(panel, tigrRGB(192, 192, 192));

  for (unsigned i = 0; i < MS_DIFFICULTIES; i++) {
    struct asset *empty = am_get_free(*am, ASSET_EMPTY);
    if (!empty) { continue; }

    print_difficulty(empty->bmp, font, difficulties[i]);
    panel = panel_add(panel, 1, component_create(difficulties[i], 0, i * menu_height, ALIGN_CENTER, 1, empty));
  }

  panel->visible = false;
  panel->blend = false;
  return panel;
}

static bool create_panels(struct window *restrict window,
                          struct panel **restrict panels,
                          size_t size,
                          struct assets_manager *restrict am,
                          struct game *restrict game,
                          TigrFont *restrict font) {
  if (!panels || !am || !game) return false;

  if (PANEL_MENU >= size) return false;

  if (!font) font = tfont;

  panels[PANEL_NAVBAR] = create_navbar(am, panel_width(&game->board), HEIGHT_NAV_PANE);
  panels[PANEL_STATS] = create_stats_panel(am, panel_width(&game->board), HEIGHT_STAT_PANE);
  panels[PANEL_BOARD] = create_main_panel(am, game, panel_width(&game->board), panel_height(&game->board));
  panels[PANEL_MENU] = create_menu(&am, font, panel_width(&game->board), window_x_panel(window, panels[PANEL_NAVBAR]));

  for (size_t i = 0; i < size; i++) {
    if (!panels[i]) {
      for (size_t j = 0; j < i; j++) {
        panel_destroy(panels[i]);
      }
      return false;
    }
  }

  return true;
}

struct window *create_window(struct game *restrict game, struct assets_manager *restrict am, TigrFont *restrict font) {
  if (!game || !am || !font) return NULL;

  struct window *window = window_create(window_width(), window_height(), "Minesweeper", TIGR_FIXED, 0);
  if (!window) { return NULL; }

  // panels
  struct panel *panels[PANEL_AMOUNT] = {0};
  if (!create_panels(window, panels, sizeof panels / sizeof *panels, am, game, font)) {
    window_destroy(window);
    return NULL;
  }

  for (size_t i = 0; i < sizeof panels / sizeof *panels; i++) {
    window = window_push(window, 1, panels[i]);
  }
  return window;
}

void reveal_mines(struct board *restrict board) {
  if (!board) return;

  for (size_t row = 0; row < board_rows(board); row++) {
    for (size_t col = 0; col < board_cols(board); col++) {
      struct cell *current = board_cell(board, row, col);
      if (!current) continue;

      if (current->mine) current->revealed = true;
    }
  }
}

static void draw_clock(struct panel *restrict panel, struct game *restrict game, TigrFont *restrict font) {
  if (!panel || !game) return;

  if (!font) font = tfont;

  struct component *clock_component = panel->components[SC_CLOCK];

  enum str_local_size {
    SIZE = 64,
  };
  char time_as_str[SIZE] = "00:00";

  double seconds = difftime(game->clock.end, game->clock.start);
  sprintf_wrapper(time_as_str, sizeof time_as_str, "%02d:%02d", ((int)seconds / 60) % 60, (int)seconds % 60);

  struct asset *asset = clock_component->assets;
  tigrClear(asset->bmp, tigrRGBA(BOARD_COLOR));
  tigrPrint(asset->bmp,
            font,
            asset->bmp->w / 2 - tigrTextWidth(font, time_as_str) / 2,
            asset->bmp->h / 2 - tigrTextHeight(font, time_as_str) / 2,
            tigrRGB(RED),
            time_as_str);
}

static void draw_mines_counter(struct panel *restrict panel, struct game *restrict game, TigrFont *restrict font) {
  if (!panel || !game) return;

  if (!font) font = tfont;

  struct component *clock_component = panel->components[SC_MINES_COUNTER];

  enum str_local_size {
    SIZE = 64,
  };
  char mines_count_as_str[SIZE] = "00";

  sprintf_wrapper(mines_count_as_str, sizeof mines_count_as_str, "%02d", game->mines);

  struct asset *asset = clock_component->assets;
  tigrClear(asset->bmp, tigrRGBA(BOARD_COLOR));
  tigrPrint(asset->bmp,
            font,
            asset->bmp->w / 2 - tigrTextWidth(font, mines_count_as_str) / 2,
            asset->bmp->h / 2 - tigrTextHeight(font, mines_count_as_str) / 2,
            tigrRGB(RED),
            mines_count_as_str);
}

static void reset_board(struct panel *restrict panel, struct assets_manager *restrict am) {
  if (!panel || !am) { return; }

  for (size_t i = 0; i < panel->components_amount; i++) {
    struct component *current = panel_component_at(panel, i);

    component_clear(current);
    component_push(current, am_get_at(am, ASSET_TILE));
  }
}

static void draw_board(struct panel *restrict panel, struct game *restrict game, struct assets_manager *restrict am) {
  if (!panel || !game || !am) { return; }

  for (size_t row = 0; row < board_rows(&game->board); row++) {
    for (size_t col = 0; col < board_cols(&game->board); col++) {
      struct cell *cell = board_cell(&game->board, row, col);
      if (!cell) continue;

      struct component *component = panel_component_at(panel, row * board_cols(&game->board) + col);
      if (cell->flagged) {
        component_remove(component, ASSET_FLAG);
        component_push(component, am_get(am, ASSET_FLAG));
      } else if (cell->mine && cell->revealed) {
        component_clear(component);
        component_push(component, am_get(am, ASSET_MINE));
      } else if (cell->revealed) {
        component_clear(component);
        component_push(component, am_get(am, ASSET_ZERO + cell->adjacent_mines));
      } else {
        component_clear(component);
        component_push(component, am_get(am, ASSET_TILE));
      }
    }
  }
}

static void draw_button(struct panel *restrict panel, struct game *restrict game, struct assets_manager *restrict am) {
  if (!panel || !game || !am) { return; }

  struct component *button = panel_component_at(panel, SC_BUTTON);
  if (!button) { return; }

  switch (game->state) {
    case STATE_WON:
      component_pop(button);
      component_push(button, am_get_at(am, ASSET_CHAD));
      break;
    case STATE_LOST:
      component_pop(button);
      component_push(button, am_get_at(am, ASSET_SAD));
      break;
    case STATE_PLAYING:
    default:  // fallthrough
      break;
  }
}

void draw_window(struct window *restrict window,
                 struct game *restrict game,
                 struct assets_manager *restrict am,
                 TigrFont *restrict font) {
  if (!window || !game) { return; }

  if (!font) font = tfont;

  window_clear(window, tigrRGBA(BOARD_COLOR));

  struct panel *stats = window_panel_at(window, PANEL_STATS);
  draw_clock(stats, game, font);
  draw_mines_counter(stats, game, font);
  draw_button(stats, game, am);
  draw_board(window_panel_at(window, PANEL_BOARD), game, am);

  window_draw(window, ALPHA);
}

static void toggle_menu(struct window *restrict window) {
  if (!window) return;

  struct panel *menu = window_panel_at(window, PANEL_MENU);
  if (!menu) return;

  menu->visible = !menu->visible;
}

static void toggle_menu_off(struct window *restrict window) {
  if (!window) return;

  struct panel *menu = window_panel_at(window, PANEL_MENU);
  if (!menu) return;

  if (menu->visible) menu->visible = false;
}

static size_t sum_adjacent_flags(struct cell const *restrict cells, size_t row, size_t col, size_t rows, size_t cols) {
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

static void reveal_next_cell(struct game *restrict game, unsigned row, unsigned col) {
  if (!game) return;

  if (game->state == STATE_LOST) return;

  // boundry check
  size_t rows = board_rows(&game->board);
  size_t cols = board_cols(&game->board);
  if (row >= rows) return;
  if (col >= cols) return;

  struct cell *current_cell = board_cell(&game->board, row, col);
  if (!current_cell) return;
  if (current_cell->revealed || current_cell->flagged) return;

  board_reveal_cell(&game->board, row, col);
  if (current_cell->mine) game->state = STATE_LOST;

  size_t adjacent_flags = sum_adjacent_flags(game->board.cells, row, col, rows, cols);
  if (adjacent_flags < current_cell->adjacent_mines) return;

  // try to go up
  reveal_next_cell(game, row - 1, col);
  // try to go top right
  reveal_next_cell(game, row - 1, col + 1);
  // try to go right
  reveal_next_cell(game, row, col + 1);
  // try to go bottom right
  reveal_next_cell(game, row + 1, col + 1);
  // try to go down
  reveal_next_cell(game, row + 1, col);
  // try to go bottom left
  reveal_next_cell(game, row + 1, col - 1);
  // try to go left
  reveal_next_cell(game, row, col - 1);
  // try to go top left
  reveal_next_cell(game, row - 1, col - 1);
}

static int flag(struct cell *restrict cell, int mines) {
  if (!cell) return mines;

  if (cell->revealed) return mines;

  cell->flagged = !cell->flagged;
  if (cell->flagged) { return mines - 1; }

  return mines + 1;
}

static void react(struct window *window, struct game *restrict game, struct mouse_event mouse_event) {
  if (!window || !game) { return; }

  if (game->state != STATE_PLAYING) { return; }

  struct component *clicked = window_get_component(window, mouse_event.x, mouse_event.y);
  if (!clicked) { return; }

  /*
    id = row * board::cols + col

    col = id % board::cols
    row = id / board::cols
  */
  unsigned col = clicked->id % board_cols(&game->board);
  unsigned row = clicked->id / board_cols(&game->board);

  struct cell *cell = board_cell(&game->board, row, col);
  if (!cell) { return; }

  switch (mouse_event.button) {
    case MOUSE_LEFT:
      if (cell->mine) {
        game->state = STATE_LOST;
      } else if (!cell->adjacent_mines) {
        reveal_next_cell(game, row, col);
      } else {
        board_reveal_cell(&game->board, row, col);
      }
      break;
    case MOUSE_RIGHT:
      game->mines = flag(cell, game->mines);
      break;
    case MOUSE_MIDDLE:
      if (!cell->revealed) { break; }
      // hack
      cell->revealed = false;
      game->board.revealed_cells--;
      // end of hack

      reveal_next_cell(game, row, col);
      break;
    default:
      break;
  }

  // check win condition
  if (board_revealed_cells(&game->board) && game->state == STATE_PLAYING) {
    game->state = STATE_WON;
    game->mines = 0;
  }
}

static void toggle_emoji(struct component *restrict component, struct asset *restrict asset) {
  if (!component || !asset) return;

  component_pop(component);
  component_push(component, asset);
}

struct window *on_mouse_click(struct window *restrict window,
                              struct game *restrict game,
                              struct assets_manager *restrict am,
                              TigrFont *restrict font,
                              struct mouse_event mouse_event) {
  if (!window) { goto end; }

  if (!game || !am) { goto end; }

  if (!font) font = tfont;

  // change the smiley when mouse button up
  if (mouse_event.button == MOUSE_NONE) {
    struct panel *stats = window_panel_at(window, PANEL_STATS);
    if (!STATE_LOST) { goto end; }

    toggle_emoji(panel_component_at(stats, SC_BUTTON), am_get_at(am, ASSET_HAPPY));
    goto end;
  }

  // change the smiley when mouse button down
  struct panel *stats = window_panel_at(window, PANEL_STATS);
  if (!stats) { goto end; }
  toggle_emoji(panel_component_at(stats, SC_BUTTON), am_get_at(am, ASSET_SHOCK));

  // prevent mouse button hold down
  if (game->prev_buttons != MOUSE_NONE) { goto end; }

  // get the components one clicked on
  struct panel *clicked_panel = window_get_panel(window, mouse_event.x, mouse_event.y);
  if (!clicked_panel) { goto end; }

  struct component *clicked_component = window_get_component(window, mouse_event.x, mouse_event.y);
  if (!clicked_component) { goto end; }

  if (clicked_panel) {
    switch (clicked_panel->id) {
      case PANEL_NAVBAR:
        if (!clicked_component) { break; }

        toggle_menu(window);
        break;
      case PANEL_BOARD:
        react(window, game, mouse_event);
        toggle_menu_off(window);
        break;
      case PANEL_STATS:
        toggle_menu_off(window);

        // reset button was clicked
        if (clicked_component->id == SC_BUTTON) {
          toggle_emoji(clicked_component, am_get_at(am, ASSET_HAPPY));

          reset_board(window_panel_at(window, PANEL_BOARD), am);
          *game = game_restart(game, game->board.difficulty);
        }
        break;
      case PANEL_MENU:
        // destroy the assets the menu consist of
        for (size_t i = 0; i < clicked_panel->components_amount; i++) {
          struct component *current = panel_component_at(clicked_panel, i);
          if (!current) continue;

          for (size_t j = 0; j < current->size; j++) {
            am_return(am, &current->assets[j]);

#ifdef DEBUG
#include <stdio.h>
            printf("asset: {id: %s, ref_count: %d}\n",
                   current->assets[j].id == ASSET_EMPTY ? "empty" : "?",
                   current->assets[j].ref_count);
#endif
          }
        }

        *game = game_restart(game, clicked_component->id);
        window_destroy(window);
        window = create_window(game, am, font);

        break;
    }
  }
end:
  return window;
}

void on_mouse_hover(struct window *restrict window,
                    struct game *restrict game,
                    struct assets_manager *restrict am,
                    TigrFont *restrict font,
                    struct mouse_event mouse_event) {
  if (!window || !game || !am) { return; }

  struct panel *menu = window_panel_at(window, PANEL_MENU);
  if (!menu || !menu->visible) { return; }

  struct panel *hovered_panel = window_get_panel(window, mouse_event.x, mouse_event.y);
  if (!hovered_panel || hovered_panel->id != PANEL_MENU) { return; }

  struct component *hovered_component = window_get_component(window, mouse_event.x, mouse_event.y);
  if (!hovered_component) { return; }

  for (size_t i = 0; i < hovered_panel->components_amount; i++) {
    if (hovered_panel->components[i] == hovered_component) continue;

    print_difficulty(hovered_panel->components[i]->assets->bmp, font, hovered_panel->components[i]->id);
  }

  tigrRect(hovered_component->assets->bmp,
           0,
           0,
           hovered_component->assets->bmp->w,
           hovered_component->assets->bmp->h,
           tigrRGB(BLACK));
}

void alert(TigrFont *restrict font, char const *fmt, ...) {
  if (!font) font = tfont;

  enum local_str_size {
    STR_SIZE = 256,
  };
  char buf[STR_SIZE];

  va_list args;
  va_start(args, fmt);

  if (!vsprintf_wrapper(buf, sizeof buf, fmt, args)) {
    vprintf(fmt, args);
    va_end(args);
    return;
  }

  Tigr *alert_window = tigrWindow(ALERT_WIDTH, ALERT_HEIGHT, "Alert", TIGR_AUTO | TIGR_2X);
  if (!alert_window) {
    printf("%s\n", buf);
    return;
  }

  unsigned x = alert_window->w / 2 - tigrTextWidth(font, buf) / 2;
  unsigned y = alert_window->h / 2 - tigrTextHeight(font, buf) / 2;
  tigrClear(alert_window, tigrRGBA(BOARD_COLOR));
  tigrPrint(alert_window, font, x, y, tigrRGB(RED), buf);

  while (!tigrClosed(alert_window)) {
    tigrUpdate(alert_window);
  }

  tigrFree(alert_window);
}
