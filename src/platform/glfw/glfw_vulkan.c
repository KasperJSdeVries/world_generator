#include "containers/darray.h"
#include "renderer/vulkan/vulkan_platform.h"

#include <GLFW/glfw3.h.>

void platform_get_required_extension_names(const char ***names_darray) {
	u32 required_extension_count = 0;
	glfwGetRequiredInstanceExtensions(&required_extension_count);
	const char **required_extensions =
		glfwGetRequiredInstanceExtensions(&required_extension_count);
	for (u32 i = 0; i < required_extension_count; ++i) {
		darray_push(*names_darray, required_extensions[i]);
	}
}
