#include "game.h"

struct game game_create(enum difficulty difficulty) {
  struct game game = {.state = STATE_INVALID};

  if (!board_create(&game.board, difficulty)) return game;
  if (!board_init(&game.board, difficulty)) return game;

  return (struct game){
    .state = STATE_PLAYING,
    .clock = {time(NULL), -1},
    .board = game.board,
    .mines = board_mines(&game.board),
    .prev_buttons = 0,
    .menu_toggled = false,
  };
}

struct game game_restart(struct game *restrict game, enum difficulty difficulty) {
  if (!game) return (struct game){.state = STATE_INVALID};

  if (!board_init(&game->board, difficulty)) {
    board_destroy(&game->board);
    return (struct game){.state = STATE_INVALID};
  }

  return (struct game){.board = game->board,
                       .clock = {time(NULL), -1},
                       .mines = board_mines(&game->board),
                       .state = STATE_PLAYING};
}

void game_destroy(struct game *restrict game) {
  if (!game) return;

  board_destroy(&game->board);
}
