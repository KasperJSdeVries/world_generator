#include <GLFW/glfw3.h>
#include <stdlib.h>

typedef struct window {
	GLFWwindow *window;
} window;

int main() {
	window *window = malloc(sizeof(struct window));

	if (!glfwInit()) {
		return 1;
	}

	window->window = glfwCreateWindow(720, 480, "World Generator", NULL, NULL);

	while (!glfwWindowShouldClose(window->window)) {
		glfwPollEvents();

		glfwSwapBuffers(window->window);
	}

	glfwDestroyWindow(window->window);

	glfwTerminate();
}
