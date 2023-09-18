#include "platform/platform.h"
#include "platform/platform_window.h"
#include "renderer/renderer.h"

typedef struct state {
	platform_state platform;
	renderer_state renderer;
} state;

int main() {
	state state = {};

	if (!platform_init(&state.platform)) {
		return 1;
	}

	window_create(state.platform, 720, 480, "World Generator");

	if (!renderer_initialize(state.platform, &state.renderer)) {
		window_destroy(state.platform);
		platform_terminate(&state.platform);
		return 1;
	}

	while (!window_should_close(state.platform)) {
		window_update(state.platform);
	}

	renderer_shutdown(&state.renderer);

	window_destroy(state.platform);
	platform_terminate(&state.platform);
}
