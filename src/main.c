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
  size_t window_width = (CELL_SIZE + CELL_SPACING) * board_cols(&game.board) + LEFT_MARGIN + RIGHT_MARGIN;
  size_t window_height = (CELL_SIZE + CELL_SPACING) * board_rows(&game.board) + TOP_MARGIN + BOTTOM_MARGIN + NAVBAR_END;

  // spacing should be something meaninful should one want to add something to the navbar
  struct panel *p_navbar = panel_create(LEFT_MARGIN,
                                        window_width - RIGHT_MARGIN,
                                        0,
                                        NAVBAR_END,
                                        (struct panel_properties){
                                          .cell_size = CELL_SIZE,
                                          .spacing = NAVBAR_SPACING,
                                        },
                                        4,
                                        "assets/classic",
                                        "assets/advanced",
                                        "assets/expert",
                                        "assets/button");
  if (!p_navbar) {
    tigrError(NULL, "failed to create the navbar");
    goto game_cleanup;
  }

  struct panel *p_top = panel_create(LEFT_MARGIN,
                                     window_width - RIGHT_MARGIN,
                                     NAVBAR_END,
                                     TOP_MARGIN + NAVBAR_END,
                                     (struct panel_properties){.cell_size = CELL_SIZE, .spacing = CELL_SPACING},
                                     5,
                                     "assets/em_happy",
                                     "assets/em_shock",
                                     "assets/em_sad",
                                     "assets/em_chad",
                                     "assets/cell");
  if (!p_top) {
    tigrError(NULL, "failed to create top panel");
    panel_destroy(p_navbar);
    goto game_cleanup;
  }

  struct panel *p_main = panel_create(LEFT_MARGIN,
                                      window_width - RIGHT_MARGIN,
                                      TOP_MARGIN + NAVBAR_END,
                                      window_height - BOTTOM_MARGIN,
                                      (struct panel_properties){.cell_size = CELL_SIZE, .spacing = CELL_SPACING},
                                      3,
                                      "assets/flag",
                                      "assets/mine",
                                      "assets/cell");

  if (!p_main) {
    tigrError(NULL, "failed to create main panel");
    panel_destroy(p_top);
    panel_destroy(p_navbar);
    goto game_cleanup;
  }

  struct window *window =
    window_create(window_width, window_height, "minesweeper", TIGR_AUTO | TIGR_2X, 3, p_navbar, p_top, p_main);
  if (!window) {
    tigrError(NULL, "failed to create game window");
    panel_destroy(p_top);
    panel_destroy(p_main);
    goto game_cleanup;
  }

  Tigr *alert_window = NULL;

  // main event loop
  while (!tigrClosed(window->bmp)) {
    if (alert_window && tigrClosed(alert_window)) { alert_window = NULL; }

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
        tigrClear(alert_window, tigrRGBA(BOARD_COLORS));
        alert_window = draw_alert("minesweeper encountered an unexpected error");
    }

    tigrUpdate(window->bmp);
  }

  if (alert_window) tigrFree(alert_window);

  window_destroy(window);
game_cleanup:
  game_destroy(&game);
}
