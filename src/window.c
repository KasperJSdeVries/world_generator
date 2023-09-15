#include "window.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

struct window_state {
	GLFWwindow *window;
};

b8 window_create(window_state **state, int window_width, int window_height,
				 const char *window_name) {
	*state = malloc(sizeof(struct window_state));

	if (!glfwInit()) {
		return false;
	}

	(*state)->window =
		glfwCreateWindow(720, 480, "World Generator", NULL, NULL);
	if (!(*state)->window) {
		glfwTerminate();
		return false;
	}

	return true;
}

void window_destroy(window_state *state) {
	glfwDestroyWindow(state->window);

	glfwTerminate();
}

b8 window_should_close(window_state *state) {
	return glfwWindowShouldClose(state->window);
}

void window_update(window_state *state) {
	glfwPollEvents();

	glfwSwapBuffers(state->window);
}
