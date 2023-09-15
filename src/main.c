#include "types.h"
#include "window.h"
#include <stdlib.h>

typedef struct state {
	window_state *window;
} state;

int main() {
	state *state = malloc(sizeof(struct state));

	window_create(&state->window, 720, 480, "World Generator");

	while (!window_should_close(state->window)) {
		window_update(state->window);
	}

	window_destroy(state->window);

	free(state);
}
