#ifndef WORLD_GENERATOR_VULKAN_CONTEXT_H
#define WORLD_GENERATOR_VULKAN_CONTEXT_H

#include <vulkan/vulkan.h>

typedef struct queue_family_info {
	u32 graphics_family_index;
	u32 present_family_index;
	u32 compute_family_index;
	u32 transfer_family_index;
} queue_family_info;

typedef struct swapchain_support_info {
	VkSurfaceCapabilitiesKHR capabilities;
	u32 format_count;
	VkSurfaceFormatKHR *formats;
	u32 present_mode_count;
	VkPresentModeKHR *present_modes;
} swapchain_support_info;

typedef struct vulkan_context {
	VkInstance instance;

#if defined(_DEBUG)
	VkDebugUtilsMessengerEXT debug_messenger;
#endif

	VkSurfaceKHR surface;

	swapchain_support_info swapchain_info;

	VkPhysicalDevice physical_device;
	VkDevice device;
} vulkan_context;

#endif // WORLD_GENERATOR_VULKAN_CONTEXT_H
