#include "../platform.h"
#include "../platform_window.h"
#include "GLFW/glfw3.h"
#include "glfw_internal_state.h"

b8 window_create(platform_state platform_state, int window_width,
				 int window_height, const char *window_name) {
	internal_state *state = (internal_state *)platform_state.internal_state;

	if (!glfwInit()) {
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	state->window =
		glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);
	if (!state->window) {
		glfwTerminate();
		return false;
	}

	return true;
}

void window_destroy(platform_state platform_state) {
	internal_state *state = (internal_state *)platform_state.internal_state;
	glfwDestroyWindow(state->window);

	glfwTerminate();
}

b8 window_should_close(platform_state platform_state) {
	internal_state *state = (internal_state *)platform_state.internal_state;
	return glfwWindowShouldClose(state->window);
}

void window_update(platform_state module_state) {
	internal_state *state = (internal_state *)module_state.internal_state;
	glfwPollEvents();

	glfwSwapBuffers(state->window);
}
