#pragma once

enum asset_ids {
  ASSET_TILE = 0,
  ASSET_FLAG,
  ASSET_QUESTION,
  ASSET_MINE,
  ASSET_HUMBURGER,
  ASSET_HAPPY,
  ASSET_SAD,
  ASSET_CHAD,
  ASSET_SHOCK,
  ASSET_LOAD_AMOUNT,
  ASSET_ZERO = ASSET_LOAD_AMOUNT,
  ASSET_ONE,
  ASSET_TWO,
  ASSET_THREE,
  ASSET_FOUR,
  ASSET_FIVE,
  ASSET_SIX,
  ASSET_SEVEN,
  ASSET_EIGHT,
  ASSET_CLOCK,
  ASSET_MINES_COUNTER,
  ASSET_EMPTY,
  ASSET_AMOUNT,
};

enum panels {
  PANEL_NAVBAR,
  PANEL_STATS,
  PANEL_BOARD,
  PANEL_MENU,
  PANEL_AMOUNT,
};

enum panels_heights {
  HEIGHT_NAV_PANE = 30,
  HEIGHT_STAT_PANE = 30,
};

// colors
#define ALPHA 1.0f

// font
#define FONT_PATH "resources/font/retron.png"

// margin
#define LEFT_MARGIN 20
#define RIGHT_MARGIN 20
#define TILE_SIZE 20
