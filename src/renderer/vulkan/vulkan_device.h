#ifndef WORLD_GENERATOR_VULKAN_DEVICE_H
#define WORLD_GENERATOR_VULKAN_DEVICE_H

#include "types.h"
#include "vulkan_context.h"

b8 vulkan_device_create(VkInstance instance, VkSurfaceKHR surface, vulkan_device *out_device);

void vulkan_device_destroy(vulkan_device *device);

void vulkan_device_query_swapchain_support(
	VkPhysicalDevice               physical_device,
	VkSurfaceKHR                   surface,
	vulkan_swapchain_support_info *out_support_info
);

b8 vulkan_device_detect_depth_format(vulkan_device *device);

#endif // WORLD_GENERATOR_VULKAN_DEVICE_H
