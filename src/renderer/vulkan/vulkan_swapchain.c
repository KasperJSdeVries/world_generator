#include "vulkan_swapchain.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_image.h"
#include "types.h"
#include "vulkan_device.h"
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

void create(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *out_swapchain);
void destroy(vulkan_context *context, vulkan_swapchain *swapchain);

void vulkan_swapchain_create(
	vulkan_context   *context,
	u32               width,
	u32               height,
	vulkan_swapchain *out_swapchain
) {
	create(context, width, height, out_swapchain);
}

void vulkan_swapchain_recreate(
	vulkan_context   *context,
	u32               width,
	u32               height,
	vulkan_swapchain *swapchain
) {
	destroy(context, swapchain);
	create(context, width, height, swapchain);
}

void vulkan_swapchain_destroy(vulkan_context *context, vulkan_swapchain *swapchain) {
	destroy(context, swapchain);
}

b8 vulkan_swapchain_acquire_next_image_index(
	vulkan_context   *context,
	vulkan_swapchain *swapchain,
	u64               timeout_ns,
	VkSemaphore       image_available_semaphore,
	VkFence           fence,
	u32			  *out_image_index
) {
	VkResult result = vkAcquireNextImageKHR(
		context->device.logical_device,
		swapchain->handle,
		timeout_ns,
		image_available_semaphore,
		fence,
		out_image_index
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		vulkan_swapchain_recreate(
			context, context->framebuffer_width, context->framebuffer_height, swapchain
		);
		return false;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		return false;
	}

	return true;
}

void vulkan_swapchain_present(
	vulkan_context   *context,
	vulkan_swapchain *swapchain,
	VkQueue           graphics_queue,
	VkQueue           present_queue,
	VkSemaphore       render_complete_semaphore,
	u32               present_image_index
) {
	VkPresentInfoKHR present_info = {
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = &render_complete_semaphore,
		.swapchainCount     = 1,
		.pSwapchains        = &swapchain->handle,
		.pImageIndices      = &present_image_index,
		.pResults           = 0,
	};

	VkResult result = vkQueuePresentKHR(present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		vulkan_swapchain_recreate(
			context, context->framebuffer_width, context->framebuffer_height, swapchain
		);
	} else if (result != VK_SUCCESS) {
		// TODO: Display debug message.
	}
}

void create(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *swapchain) {
	VkExtent2D swapchain_extent     = { width, height };
	swapchain->max_frames_in_flight = 2;

	b8 found = false;
	for (u32 i = 0; i < context->device.swapchain_support.format_count; ++i) {
		VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];

		if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
		    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			swapchain->image_format = format;
			found                   = true;
			break;
		}
	}

	if (!found) {
		swapchain->image_format = context->device.swapchain_support.formats[0];
	}

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (u32 i = 0; i < context->device.swapchain_support.present_mode_count; ++i) {
		VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = mode;
			break;
		}
	}

	vulkan_device_query_swapchain_support(
		context->device.physical_device, context->surface, &context->device.swapchain_support
	);

	if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
		swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
	}

	VkExtent2D min          = context->device.swapchain_support.capabilities.minImageExtent;
	VkExtent2D max          = context->device.swapchain_support.capabilities.maxImageExtent;
	swapchain_extent.width  = CLAMP(swapchain_extent.width, min.width, max.width);
	swapchain_extent.height = CLAMP(swapchain_extent.height, min.height, max.height);

	u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
	if (context->device.swapchain_support.capabilities.maxImageCount > 0 &&
	    image_count > context->device.swapchain_support.capabilities.maxImageCount) {
		image_count = context->device.swapchain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface                  = context->surface;
	create_info.minImageCount            = image_count;
	create_info.imageFormat              = swapchain->image_format.format;
	create_info.imageColorSpace          = swapchain->image_format.colorSpace;
	create_info.imageExtent              = swapchain_extent;
	create_info.imageArrayLayers         = 1;
	create_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (context->device.graphics_queue_index != context->device.present_queue_index) {
		u32 queueFamilyIndices[]          = { (u32)context->device.graphics_queue_index,
			                                  (u32)context->device.present_queue_index };
		create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices   = queueFamilyIndices;
	} else {
		create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices   = 0;
	}

	create_info.preTransform   = context->device.swapchain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode    = present_mode;
	create_info.clipped        = VK_TRUE;
	create_info.oldSwapchain   = 0;

	// TODO: Error handling
	vkCreateSwapchainKHR(context->device.logical_device, &create_info, NULL, &swapchain->handle);

	context->current_frame = 0;

	swapchain->image_count = 0;
	vkGetSwapchainImagesKHR(
		context->device.logical_device, swapchain->handle, &swapchain->image_count, NULL
	);
	if (!swapchain->images) {
		swapchain->images = malloc(sizeof(VkImage) * swapchain->image_count);
	}
	if (!swapchain->views) {
		swapchain->views = malloc(sizeof(VkImageView) * swapchain->image_count);
	}
	vkGetSwapchainImagesKHR(
		context->device.logical_device,
		swapchain->handle,
		&swapchain->image_count,
		swapchain->images
	);

	for (u32 i = 0; i < swapchain->image_count; ++i) {
		VkImageViewCreateInfo view_info = (VkImageViewCreateInfo){
			.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image    = swapchain->images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format   = swapchain->image_format.format,
			.subresourceRange =
				(VkImageSubresourceRange){
					.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel   = 0,
					.levelCount     = 1,
					.baseArrayLayer = 0,
					.layerCount     = 1,
				},
		};

		vkCreateImageView(context->device.logical_device, &view_info, NULL, &swapchain->views[i]);
	}

	if (!vulkan_device_detect_depth_format(&context->device)) {
		context->device.depth_format = VK_FORMAT_UNDEFINED;
		// TODO: Add debug log
	}

	vulkan_image_create(
		context,
		VK_IMAGE_TYPE_2D,
		swapchain_extent.width,
		swapchain_extent.height,
		context->device.depth_format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_TRUE,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		&swapchain->depth_attachment
	);
}

void destroy(vulkan_context *context, vulkan_swapchain *swapchain) {
	vulkan_image_destroy(context, &swapchain->depth_attachment);

	for (u32 i = 0; i < swapchain->image_count; ++i) {
		vkDestroyImageView(context->device.logical_device, swapchain->views[i], NULL);
	}

	vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, NULL);
}
