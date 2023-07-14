#pragma once

/**
 * @brief represents mouse buttons
 */

#ifdef _WIN32
enum mouse_button {
  MOUSE_NONE,
  MOUSE_LEFT = 1,
  MOUSE_MIDDLE = 2,
  MOUSE_RIGHT = 4,
};
#else
enum mouse_button {
  MOUSE_NONE,
  MOUSE_LEFT = 1,
  MOUSE_RIGHT = 2,
  MOUSE_MIDDLE = 4,
};
#endif

/**
 * @brief represents the complete event. the pressed button & its coordinates
 */
struct mouse_event {
  enum mouse_button button;

  int x;
  int y;
};

/**
 * @brief processes _one_ mouse event. isn't capable to process multiple key presses
 */
struct mouse_event mouse_event_create(unsigned x, unsigned y, unsigned buttons);
