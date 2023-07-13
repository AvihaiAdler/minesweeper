#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include "colors.h"
#include "properties.h"

#define ALERT_WIDTH 300
#define ALERT_HEIGHT 200

enum stats_panel_components {
  SC_MINES_COUNTER,
  SC_BUTTON,
  SC_CLOCK,
};

static inline char *vsprintf_wrapper(char *buf, size_t size, char const *fmt, va_list args) {
  if (!buf || !size) return NULL;

  va_list args_cpy;
  va_copy(args_cpy, args);

  int required_size = vsnprintf(NULL, 0, fmt, args_cpy);
  va_end(args_cpy);

  if (required_size + 1 >= (int)size) { return NULL; }

  vsprintf(buf, fmt, args);
  return buf;
}

static inline char *sprintf_wrapper(char *buf, size_t size, char const *fmt, ...) {
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

unsigned calculate_width(struct board const *restrict board) {
  return board_cols(board) * TILE_SIZE + LEFT_MARGIN + RIGHT_MARGIN;
}

unsigned calculate_height(struct board const *restrict board) {
  return board_rows(board) * TILE_SIZE + HEIGHT_NAV_PANE + HEIGHT_STAT_PANE;
}

TigrFont *load_font(char const *restrict font_path) {
  if (!font_path) return NULL;

  Tigr *font_png = tigrLoadImage(font_path);
  if (!font_png) return NULL;

  return tigrLoadFont(font_png, TCP_ASCII);
}

static inline TPixel numeric_color(int num) {
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

static inline Tigr *create_clock_asset(TigrFont *restrict font) {
  enum local_str_size {
    SIZE = 64,
  };
  char clock[SIZE];

  if (!sprintf_wrapper(clock, sizeof clock, "%02d:%02d", 0, 0)) return NULL;

  unsigned width = tigrTextWidth(font, clock);
  unsigned height = tigrTextHeight(font, clock);

  return tigrBitmap(width, height > TILE_SIZE ? height : TILE_SIZE);
}

static inline Tigr *create_mines_asset(TigrFont *restrict font) {
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

    if (!sprintf_wrapper(numeral, sizeof numeral, "%d", i - ASSET_ZERO)) continue;
    tigrPrint(bmp,
              font,
              bmp->w / 2 - tigrTextWidth(font, numeral) / 2,
              bmp->h / 2 - tigrTextHeight(font, numeral) / 2,
              numeric_color(i - ASSET_ZERO),
              numeral);

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

  return am;
}

static inline struct panel *create_navbar(struct assets_manager *restrict am, size_t width, size_t height) {
  if (!am) return NULL;

  struct component *humburger = component_create(0, 0, 0, ALIGN_LEFT, 1, am_get_at(am, ASSET_HUMBURGER));
  if (!humburger) return NULL;

  return panel_create(PANEL_NAVBAR, 0, 0, ALIGN_CENTER, width, height, 1, humburger);
}

// assumes component creation never fails
static inline struct panel *create_stats_panel(struct assets_manager *restrict am, size_t width, size_t height) {
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
static inline struct panel *create_main_panel(struct assets_manager *restrict am,
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

static inline unsigned max_text_width(TigrFont *restrict font) {
  char *difficulty_as_text[] = {"classic", "advanced", "expert"};

  enum local_str_size {
    SIZE = 64,
  };
  char text[SIZE];

  unsigned width = 0;
  for (size_t i = 0; i < sizeof difficulty_as_text / sizeof *difficulty_as_text; i++) {
    if (!sprintf_wrapper(text, sizeof text, "%s", difficulty_as_text[i])) continue;

    unsigned tmp_width = tigrTextWidth(font, text);
    if (width < tmp_width) width = tmp_width;
  }

  return width;
}

static inline unsigned max_text_height(TigrFont *restrict font) {
  return tigrTextHeight(font, "a");
}

static inline struct panel *create_menu(struct assets_manager *restrict *am,
                                        TigrFont *restrict font,
                                        size_t width,
                                        size_t height) {
  if (!am) return NULL;

  unsigned menu_width = max_text_width(font);
  unsigned menu_height = max_text_height(font);

  if (menu_width > width || menu_height > height) return NULL;

  struct panel *panel = panel_create(PANEL_MENU, LEFT_MARGIN, HEIGHT_NAV_PANE, ALIGN_LEFT, menu_width, menu_height, 0);
  if (!panel) return NULL;

  for (unsigned i = 0; i < MS_DIFFICULTIES; i++) {
    Tigr *bmp = tigrBitmap(menu_width, menu_height);
    if (!bmp) continue;

    *am = am_push(*am, asset_create(ASSET_EMPTY, bmp));

    panel =
      panel_add(panel, 1, component_create(i, 0, i * menu_height, ALIGN_CENTER, 1, am_get_free(*am, ASSET_EMPTY)));
  }

  return panel;
}

bool create_panels(struct panel **restrict panels,
                   size_t size,
                   struct assets_manager *restrict am,
                   struct game *restrict game,
                   TigrFont *restrict font,
                   size_t width) {
  if (!panels) return false;

  panels[PANEL_NAVBAR] = create_navbar(am, width, HEIGHT_NAV_PANE);
  panels[PANEL_STATS] = create_stats_panel(am, width, HEIGHT_STAT_PANE);
  panels[PANEL_BOARD] =
    create_main_panel(am, game, width, calculate_height(&game->board) - (HEIGHT_NAV_PANE + HEIGHT_STAT_PANE));
  // panels[PANEL_MENU] = create_menu(&am, font, width, height);

  for (size_t i = 0; i < size; i++) {
    if (!panels[i]) return false;
  }

  return true;
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

void draw_clock(struct window *restrict window, struct game *restrict game, TigrFont *restrict font) {
  if (!window || !game) return;

  if (!font) font = tfont;

  struct panel *stats = window->panels[PANEL_STATS];
  struct component *clock_component = stats->components[SC_CLOCK];

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
            asset->bmp->h / 2 /*- tigrTextHeight(font, time_as_str)*/,
            tigrRGB(RED),
            time_as_str);
}

void draw_mines_counter(struct window *restrict window, struct game *restrict game, TigrFont *restrict font) {
  if (!window || !game) return;

  if (!font) font = tfont;

  struct panel *stats = window->panels[PANEL_STATS];
  struct component *clock_component = stats->components[SC_MINES_COUNTER];

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
            asset->bmp->h / 2 /*- tigrTextHeight(font, mines_count_as_str)*/,
            tigrRGB(RED),
            mines_count_as_str);
}

void on_mouse_click(struct window *restrict window,
                    struct game *restrict game,
                    struct assets_manager *restrict am,
                    struct mouse_event mouse_event) {
  if (!window || !game) return;

  if (mouse_event.button == MOUSE_NONE) {
    component_pop(window->panels[PANEL_STATS]->components[SC_BUTTON]);
    component_push(window->panels[PANEL_STATS]->components[SC_BUTTON], am_get_at(am, ASSET_HAPPY));
    return;
  }

  component_pop(window->panels[PANEL_STATS]->components[SC_BUTTON]);
  component_push(window->panels[PANEL_STATS]->components[SC_BUTTON], am_get_at(am, ASSET_SHOCK));

  if (game->prev_buttons != MOUSE_NONE) return;

  struct panel *clicked_panel = window_get_panel(window, mouse_event.x, mouse_event.y);
  if (!clicked_panel) return;
#include <string.h>
  if (!clicked_panel) {
    puts("null panel");
    return;
  }
  switch (clicked_panel->id) {
    case PANEL_BOARD:
      puts("board");
      break;
    case PANEL_MENU:
      puts("menu");
      break;
    case PANEL_NAVBAR:
      puts("navbar");
      break;
    case PANEL_STATS:
      puts("stats");
      break;
    default:
      break;
  }

  struct component *clicked_componenet = window_get_component(window, mouse_event.x, mouse_event.y);
  if (!clicked_componenet) {
    puts("null component");
    return;
  }
  if (clicked_panel->id == PANEL_STATS) {
    switch (clicked_componenet->id) {
      case SC_CLOCK:
        puts("clock");
        break;
      case SC_MINES_COUNTER:
        puts("mines counter");
        break;
      case SC_BUTTON:
        puts("button");
        break;
      default:
        break;
    }
  }

  // reset button was clicked
  if (clicked_panel->id == PANEL_STATS && clicked_componenet->id == SC_BUTTON) {
    printf("reset\n");
    enum difficulty difficulty = game->board.difficulty;
    game_destroy(game);
    *game = game_create(difficulty);

    component_pop(window->panels[PANEL_STATS]->components[SC_BUTTON]);
    component_push(window->panels[PANEL_STATS]->components[SC_BUTTON], am_get_at(am, ASSET_HAPPY));
    return;
  }

  switch (game->state) {
    case STATE_PLAYING:
      break;
    case STATE_WON:
      // component_pop(window->panels[PANEL_STATS]->components[SC_BUTTON]);
      // component_push(window->panels[PANEL_STATS]->components[SC_BUTTON], am_get_at(am, ASSET_CHAD));
    case STATE_LOST:  // fallthrough
      // component_pop(window->panels[PANEL_STATS]->components[SC_BUTTON]);
      // component_push(window->panels[PANEL_STATS]->components[SC_BUTTON], am_get_at(am, ASSET_SAD));
      break;
    case STATE_INVALID:
    default:  // fallthrough
      break;
  }
}

void on_mouse_hover(struct window *restrict window,
                    struct game *restrict game,
                    struct assets_manager *restrict am,
                    struct mouse_event mouse_event) {
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
