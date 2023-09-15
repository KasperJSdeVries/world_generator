#include "types.h"

typedef struct window_state window_state;

b8 window_create(window_state **state, int window_width, int window_height,
				 const char *window_name);

void window_destroy(window_state *state);

b8 window_should_close(window_state *state);

void window_update(window_state *state);
