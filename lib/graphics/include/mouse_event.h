#pragma once

/**
 * @brief represets mouse events
 */

/**
 * @brief represents mouse buttons
 */
enum mouse_button {
  MOUSE_NONE,
  MOUSE_LEFT = 1,
  MOUSE_RIGHT = 2,
  MOUSE_MIDDLE = 4,
};

/**
 * @brief represents the complete event. the pressed button & its coordinates
 */
struct mouse_event {
  enum mouse_button button;

  unsigned x;
  unsigned y;
};

/**
 * @brief processes _one_ mouse event. isn't capable to process multiple key presses
 *
 * @param x
 * @param y
 * @param buttons
 * @return struct mouse_event
 */
struct mouse_event mouse_event_create(unsigned x, unsigned y, unsigned buttons);