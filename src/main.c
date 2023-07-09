#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "game_properties.h"
#include "panel.h"
#include "tigr.h"
#include "util.h"
#include "window.h"

int main(void) {
  srand((unsigned)time(NULL));

  // game vars
  struct game game = game_create(MS_CLASSIC);
  // init game
  if (init_new_game(&game, MS_CLASSIC) == STATE_INVALID_STATE) {
    draw_alert("failed to create a game");
    goto game_cleanup;
  }

  // graphics vars
  size_t window_width = calculate_window_width(&game.board);
  size_t window_height = calculate_window_height(&game.board);

  // font
  Tigr *font_bmp = tigrLoadImage("resources/font/pixellari.png");
  if (!font_bmp) {
    draw_alert("failed to load a font bitmap");
    goto game_cleanup;
  }

  TigrFont *retro_gaming = tigrLoadFont(font_bmp, TCP_ASCII);
  if (!retro_gaming) {
    draw_alert("failed to load a font");
    goto game_cleanup;
  }

  // spacing should be something meaninful should one want to add something to the navbar
  struct panel *p_navbar = panel_create(LEFT_MARGIN,
                                        0,
                                        window_width - (RIGHT_MARGIN + LEFT_MARGIN),
                                        NAVBAR_HEIGHT,
                                        (struct panel_properties){
                                          .cell_size = CELL_SIZE * 3,
                                          .spacing = NAVBAR_SPACING,
                                          .font = retro_gaming,
                                        },
                                        1,
                                        "resources/assets/humburger");
  if (!p_navbar) {
    draw_alert("failed to create the navbar");
    goto font_bmp_cleanup;
  }

  p_navbar = create_difficulties_assets(p_navbar);

  struct panel *p_top = panel_create(LEFT_MARGIN,
                                     NAVBAR_HEIGHT,
                                     window_width - (RIGHT_MARGIN + LEFT_MARGIN),
                                     NAVBAR_HEIGHT,
                                     (struct panel_properties){.cell_size = CELL_SIZE, .spacing = CELL_SPACING},
                                     5,
                                     "resources/assets/em_happy",
                                     "resources/assets/em_shock",
                                     "resources/assets/em_sad",
                                     "resources/assets/em_chad",
                                     "resources/assets/cell");
  if (!p_top) {
    draw_alert("failed to create top panel");
    goto navbar_cleanup;
  }

  struct panel *p_main = panel_create(LEFT_MARGIN,
                                      NAVBAR_HEIGHT + TOP_PANEL_HEIGHT,
                                      window_width - (RIGHT_MARGIN + LEFT_MARGIN),
                                      (CELL_SIZE + CELL_SPACING) * board_rows(&game.board),
                                      (struct panel_properties){.cell_size = CELL_SIZE, .spacing = CELL_SPACING},
                                      3,
                                      "resources/assets/flag",
                                      "resources/assets/mine",
                                      "resources/assets/cell");

  if (!p_main) {
    draw_alert("failed to create main panel");
    goto top_panel_cleanup;
  }

  struct window *window =
    window_create(window_width, window_height, "minesweeper", TIGR_AUTO | TIGR_2X, 3, p_navbar, p_top, p_main);
  if (!window) {
    draw_alert("failed to create game window");
    goto main_panel_cleanup;
  }

  // main event loop
  while (!tigrClosed(window->bmp)) {
    int x_coord = 0;
    int y_coord = 0;
    int buttons = 0;

    tigrClear(window->bmp, tigrRGBA(BOARD_COLORS));
    tigrMouse(window->bmp, &x_coord, &y_coord, &buttons);

    draw_window(window, &game, buttons ? MOUSE_DOWN : MOUSE_UP);
    on_mouse_click(window, &game, x_coord, y_coord, buttons);

    // maintain an internal state of the last mouse event to prevent mouse hold down
    game.prev_event = buttons ? MOUSE_DOWN : MOUSE_UP;

    switch (game.game_state) {
      case STATE_LOST:  // fallthrough
      case STATE_WON:
        reveal_all_mines(&game.board);
        break;
      case STATE_PLAYING:
        game.game_clock.end = time(NULL);
        break;
      default:
        break;
    }

    tigrUpdate(window->bmp);
  }

  window_destroy(window);
game_cleanup:
  game_destroy(&game);
  return 0;

main_panel_cleanup:
  panel_destroy(p_main);
top_panel_cleanup:
  panel_destroy(p_top);
navbar_cleanup:
  panel_destroy(p_navbar);
font_bmp_cleanup:
  tigrFree(font_bmp);
  game_destroy(&game);
}
