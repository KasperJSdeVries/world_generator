#include "../platform.h"
#include "glfw_internal_state.h"

#include <stdlib.h>

b8 platform_init(platform_state *state) {
	if (!glfwInit()) {
		return false;
	}

	state->internal_state = malloc(sizeof(internal_state));

	return true;
}

void platform_terminate(platform_state *state) {
	glfwTerminate();
	free(state->internal_state);
}
