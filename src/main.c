#include "platform/platform_window.h"

typedef struct state {
	platform_state platform;
} state;

int main() {
	state state = {};

	if (!platform_init(&state.platform)) {
		return 1;
	}

	window_create(state.platform, 720, 480, "World Generator");

	while (!window_should_close(state.platform)) {
		window_update(state.platform);
	}

	window_destroy(state.platform);
	platform_terminate(&state.platform);
}
