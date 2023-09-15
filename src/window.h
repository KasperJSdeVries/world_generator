#include "module.h"
#include "types.h"

typedef struct internal_state internal_state;

b8 window_create(module_state *state, int window_width, int window_height,
				 const char *window_name);

void window_destroy(module_state state);

b8 window_should_close(module_state state);

void window_update(module_state state);
