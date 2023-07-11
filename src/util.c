#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include "colors.h"
#include "properties.h"

#define ALERT_WIDTH 300
#define ALERT_HEIGHT 200

static inline char *vsprintf_wrapper(char *buf, size_t size, char const *fmt, va_list args) {
  if (!buf || !size) return NULL;

  va_list args_cpy;
  va_copy(args_cpy, args);

  int required_size = vsnprintf(NULL, 0, fmt, args);
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

struct assets_manager *create_assets(struct assets_manager *am) {
  if (!am) return NULL;

  for (size_t i = ASSET_ONE; i < ASSET_EIGHT + 1; i++) {
    Tigr *bmp = tigrBitmap(TILE_SIZE, TILE_SIZE);
    if (!bmp) return am;

    am = am_push(am, &(struct asset){.id = (int)i, .bmp = bmp});
  }

  return am;
}

// TODO: complete the creation of each individual panel
bool create_panels(struct panel **restrict panels, size_t size, unsigned width, unsigned height) {
  (void)panels;
  (void)size;
  (void)width;
  (void)height;
  return false;
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

void on_mouse_click(struct window *restrict window, struct game *restrict game, struct mouse_event mouse_event);

void on_mouse_hover(struct window *restrict window, struct game *restrict game, struct mouse_event mouse_event);

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
  tigrPrint(alert_window, font, x, y, tigrRGB(RED), buf);

  while (!tigrClosed(alert_window) || !tigrKeyDown(alert_window, TK_ESCAPE)) {
    tigrClear(alert_window, tigrRGBA(BOARD_COLOR));
    tigrUpdate(alert_window);
  }

  tigrFree(alert_window);
}
