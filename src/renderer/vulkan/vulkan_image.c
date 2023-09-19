#include "vulkan_image.h"

#include "vulkan_device.h"
#include <vulkan/vulkan_core.h>

void vulkan_image_create(
	vulkan_context       *context,
	VkImageType           image_type,
	u32                   width,
	u32                   height,
	VkFormat              format,
	VkImageTiling         tiling,
	VkImageUsageFlags     usage,
	VkMemoryPropertyFlags memory_flags,
	b32                   create_view,
	VkImageAspectFlags    view_aspect_flags,
	vulkan_image         *out_image
) {
	out_image->width  = width;
	out_image->height = height;

	VkImageCreateInfo image_create_info = {
		.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
        .extent    = { .width = width, .height = height, .depth = 1, // TODO: Support configurable depth
            },
        .mipLevels = 4, // TODO: Support mip mapping
        .arrayLayers = 1, // TODO: Support number of layers in the image
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = VK_SAMPLE_COUNT_1_BIT, // TODO: Configurable sample count
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // TODO: Configurable sharing mode
	};

	vkCreateImage(context->device.logical_device, &image_create_info, NULL, &out_image->handle);

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(
		context->device.logical_device, out_image->handle, &memory_requirements
	);

	i32 memory_type =
		context->find_memory_index(context, memory_requirements.memoryTypeBits, memory_flags);
	if (memory_type == -1) {
		// TODO: Error message
	}

	VkMemoryAllocateInfo memory_allocate_info = {
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = memory_requirements.size,
		.memoryTypeIndex = memory_type,
	};
	vkAllocateMemory(
		context->device.logical_device, &memory_allocate_info, NULL, &out_image->memory
	);

	vkBindImageMemory(
		context->device.logical_device, out_image->handle, out_image->memory, 0
	); // TODO: Configurable memory offset
}

void vulkan_image_view_create(
	vulkan_context    *context,
	VkFormat           format,
	vulkan_image      *image,
	VkImageAspectFlags aspect_flags
) {
	VkImageViewCreateInfo view_create_info = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		                                       .image = image->handle,
		                                       .viewType =
		                                           VK_IMAGE_VIEW_TYPE_2D, // TODO: Make configurable
		                                       .format           = format,
		                                       .subresourceRange = {
												   .aspectMask = aspect_flags,

												   // TODO: Make configurable
												   .baseMipLevel   = 0,
												   .levelCount     = 1,
												   .baseArrayLayer = 0,
												   .layerCount     = 1,
											   }, };

	vkCreateImageView(context->device.logical_device, &view_create_info, NULL, &image->view);
}

void vulkan_image_destroy(vulkan_context *context, vulkan_image *image) {
	if (image->view) {
		vkDestroyImageView(context->device.logical_device, image->view, NULL);
		image->view = NULL;
	}
	if (image->memory) {
		vkFreeMemory(context->device.logical_device, image->memory, NULL);
		image->memory = NULL;
	}
	if (image->handle) {
		vkDestroyImage(context->device.logical_device, image->handle, NULL);
		image->handle = NULL;
	}
}
