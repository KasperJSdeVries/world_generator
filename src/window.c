#include "window.h"
#include "module.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

typedef struct internal_state {
	GLFWwindow *window;
} internal_state;

b8 window_create(module_state *module_state, int window_width,
				 int window_height, const char *window_name) {
	module_state->internal_state = malloc(sizeof(struct internal_state));
	internal_state *state = (internal_state *)module_state->internal_state;

	if (!glfwInit()) {
		return false;
	}

	state->window = glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);
	if (!state->window) {
		glfwTerminate();
		return false;
	}

	return true;
}

void window_destroy(module_state module_state) {
	internal_state *state = (internal_state *)module_state.internal_state;
	glfwDestroyWindow(state->window);

	glfwTerminate();
}

b8 window_should_close(module_state module_state) {
	internal_state *state = (internal_state *)module_state.internal_state;
	return glfwWindowShouldClose(state->window);
}

void window_update(module_state module_state) {
	internal_state *state = (internal_state *)module_state.internal_state;
	glfwPollEvents();

	glfwSwapBuffers(state->window);
}
