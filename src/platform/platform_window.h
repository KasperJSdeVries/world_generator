#include "platform.h"
#include "types.h"

b8 window_create(
	platform_state platform_state,
	int            window_width,
	int            window_height,
	const char    *window_name
);

void window_destroy(platform_state platform_state);

b8 window_should_close(platform_state platform_state);

void window_update(platform_state state);
