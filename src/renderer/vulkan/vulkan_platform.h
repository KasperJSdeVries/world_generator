#ifndef WORLD_GENERATOR_VULKAN_PLATFORM_H
#define WORLD_GENERATOR_VULKAN_PLATFORM_H

#include "types.h"

void platform_get_required_extension_names(const char ***names_darray);

b8 platform_get_surface(
	VkInstance     instance,
	platform_state platform_state,
	VkSurfaceKHR  *out_surface
);

#endif // WORLD_GENERATOR_VULKAN_PLATFORM_H
