#include "window.h"

typedef struct state {
	module_state window;
} state;

int main() {
	state state = {};

	window_create(&state.window, 720, 480, "World Generator");

	while (!window_should_close(state.window)) {
		window_update(state.window);
	}

	window_destroy(state.window);
}
