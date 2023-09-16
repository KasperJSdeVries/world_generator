#ifndef WORLD_GENERATOR_VULKAN_CONTEXT_H
#define WORLD_GENERATOR_VULKAN_CONTEXT_H

#include <vulkan/vulkan.h>

typedef struct vulkan_context {
	VkInstance instance;
#if defined(_DEBUG)
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
} vulkan_context;

#endif // WORLD_GENERATOR_VULKAN_CONTEXT_H
