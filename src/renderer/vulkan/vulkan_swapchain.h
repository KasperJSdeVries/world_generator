#ifndef WORLD_GENERATOR_VULKAN_SWAPCHAIN_H
#define WORLD_GENERATOR_VULKAN_SWAPCHAIN_H

#include "types.h"
#include "vulkan_context.h"

void vulkan_swapchain_create(
	vulkan_context   *device,
	u32               width,
	u32               height,
	vulkan_swapchain *out_swapchain
);

void vulkan_swapchain_recreate(
	vulkan_context   *device,
	u32               width,
	u32               height,
	vulkan_swapchain *swapchain
);

void vulkan_swapchain_destroy(vulkan_context *device, vulkan_swapchain *swapchain);

b8 vulkan_swapchain_acquire_next_image_index(
	vulkan_context   *context,
	vulkan_swapchain *swapchain,
	u64               timeout_ns,
	VkSemaphore       image_available_semaphore,
	VkFence           fence,
	u32			  *out_image_index
);

void vulkan_swapchain_present(
	vulkan_context   *context,
	vulkan_swapchain *swapchain,
	VkQueue           graphics_queue,
	VkQueue           present_queue,
	VkSemaphore       render_complete_semaphore,
	u32               present_image_index
);

#endif // !WORLD_GENERATOR_VULKAN_SWAPCHAIN_H
