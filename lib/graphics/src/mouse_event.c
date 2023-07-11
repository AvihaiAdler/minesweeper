#include "mouse_event.h"

struct mouse_event mouse_event_create(unsigned x, unsigned y, unsigned buttons) {
  unsigned button = buttons & 0xf;
  return (struct mouse_event){.x = x, .y = y, .button = button};
}
