#include "containers/darray.h"
#include "glfw_internal_state.h"
#include "platform/platform.h"
#include "renderer/vulkan/vulkan_platform.h"

void platform_get_required_extension_names(const char ***names_darray) {
	u32 required_extension_count = 0;
	glfwGetRequiredInstanceExtensions(&required_extension_count);
	const char **required_extensions = glfwGetRequiredInstanceExtensions(&required_extension_count);
	for (u32 i = 0; i < required_extension_count; ++i) {
		darray_push(*names_darray, required_extensions[i]);
	}
}

b8 platform_get_surface(VkInstance instance, platform_state platform_state, VkSurfaceKHR *out_surface) {
	internal_state *state = (internal_state *)platform_state.internal_state;
	VkResult result = glfwCreateWindowSurface(instance, state->window, NULL, out_surface);
	if (result != VK_SUCCESS) {
		return false;
	}
	return true;
}
