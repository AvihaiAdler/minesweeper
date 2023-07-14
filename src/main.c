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
    alert(tfont, "failed to load the font\n'%s'", FONT_PATH);
    goto end;
  }

  // assets
  struct assets_manager *am = am_create(ASSET_LOAD_AMOUNT,
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
  if (am->size == ASSET_LOAD_AMOUNT) {
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
  struct window *window = create_window(&game, am, font);
  if (!window) {
    alert(font, "falied to create game window");
    goto game_cleanup;
  }

  while (!tigrClosed(window->window)) {
    int x = 0;
    int y = 0;
    int buttons = 0;

    tigrMouse(window->window, &x, &y, &buttons);

    struct mouse_event mouse_event = mouse_event_create(x, y, buttons);

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

    on_mouse_hover(window, &game, am, font, mouse_event);
    window = on_mouse_click(window, &game, am, font, mouse_event);
    game.prev_buttons = mouse_event.button;

    draw_window(window, &game, am, font);
  }

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
