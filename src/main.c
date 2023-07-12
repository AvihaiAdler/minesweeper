#include <time.h>
#include "assets.h"
#include "colors.h"
#include "game.h"
#include "mouse_event.h"
#include "properties.h"
#include "util.h"
#include "window.h"

int main(void) {
  // font
  TigrFont *font = load_font(FONT_PATH);
  if (!font) {
    alert(tfont, "failed to load the font '%s'", FONT_PATH);
    goto end;
  }

  // assets
  struct assets_manager *am = am_create(ASSET_AMOUNT,
                                        "resources/assets/tile",
                                        "resources/assets/flag",
                                        "resources/assets/mine",
                                        "resources/assets/humburger",
                                        "resources/assets/em_happy",
                                        "resources/assets/em_sad",
                                        "resources/assets/em_chad",
                                        "resources/assets/em_shock");
  if (!am) {
    alert(font, "falied to load game assets");
    goto font_cleanup;
  }

  am = create_assets(am, font);
  if (am->size == ASSET_AMOUNT) {
    alert(font, "failed to create game assets");
    goto assets_cleanup;
  }

  // game
  struct game game = game_create(MS_CLASSIC);
  if (game.state == STATE_INVALID) {
    alert(font, "failed to create a new game");
    goto assets_cleanup;
  }

  // window
  size_t window_width = calculate_width(&game.board);
  size_t window_height = calculate_height(&game.board);
  struct window *window = window_create(window_width, window_height, "Minesweeper", TIGR_AUTO | TIGR_2X, 0);
  if (!window) {
    alert(font, "falied to create game window");
    goto game_cleanup;
  }

  // panels
  struct panel *panels[PANEL_AMOUNT] = {0};
  if (!create_panels(panels, sizeof panels, am, &game, font, window_width, window_height)) {
    alert(font, "failed to create window's panels");
    goto window_cleanup;
  }

  for (size_t i = 0; i < (size_t)PANEL_AMOUNT; i++) {
    window = window_push(window, 1, panels[i]);
  }

  while (!tigrClosed(window->window) || !tigrKeyDown(window->window, TK_ESCAPE)) {
    int x = 0;
    int y = 0;
    int buttons = 0;

    tigrMouse(window->window, &x, &y, &buttons);
    struct mouse_event mouse_event = mouse_event_create(x, y, buttons);

    on_mouse_click(window, &game, mouse_event);
    on_mouse_hover(window, &game, mouse_event);

    switch (game.state) {
      case STATE_INVALID:
        alert(font, "an unexpected error occurred");
        break;
      case STATE_PLAYING:
        game.clock.end = time(NULL);  // update the clock
        break;
      case STATE_WON:
      case STATE_LOST:  // fallthrough
        reveal_mines(&game.board);
      default:  // fallthrough
        break;
    }

    window_clear(window, tigrRGBA(BOARD_COLOR));
    window_draw(window, ALPHA);
  }

window_cleanup:
  window_destroy(window);
game_cleanup:
  game_destroy(&game);
assets_cleanup:
  am_destroy(am);
font_cleanup:
  tigrFreeFont(font);
end:
  return 0;
}
