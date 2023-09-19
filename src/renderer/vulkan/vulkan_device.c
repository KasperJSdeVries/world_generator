#include "vulkan_device.h"
#include "containers/darray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

typedef struct vulkan_physical_device_requirements {
	b8 graphics;
	b8 present;
	b8 compute;
	b8 transfer;

	const char **device_extension_names; // darray
	b8           sampler_anisotropy;
	b8           discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
	u32 graphics_family_index;
	u32 present_family_index;
	u32 compute_family_index;
	u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

b8 select_physical_device(VkInstance instance, VkSurfaceKHR surface, vulkan_device *out_device);
b8 physical_device_meets_requirements(
	VkPhysicalDevice                           device,
	VkSurfaceKHR                               surface,
	const VkPhysicalDeviceProperties          *properties,
	const VkPhysicalDeviceFeatures            *features,
	const vulkan_physical_device_requirements *requirements,
	vulkan_physical_device_queue_family_info  *out_queue_info,
	vulkan_swapchain_support_info             *out_swapchain_support
);

b8 vulkan_device_create(VkInstance instance, VkSurfaceKHR surface, vulkan_device *out_device) {
	if (!select_physical_device(instance, surface, out_device)) {
		return false;
	}

	b8 compute_shares_graphics_queue =
		out_device->graphics_queue_index == out_device->compute_queue_index;
	b8 present_shares_graphics_queue =
		out_device->graphics_queue_index == out_device->present_queue_index;
	b8 transfer_shares_graphics_queue =
		out_device->graphics_queue_index == out_device->transfer_queue_index;
	b8 present_shares_compute_queue =
		out_device->compute_queue_index == out_device->present_queue_index;
	b8 transfer_shares_compute_queue =
		out_device->compute_queue_index == out_device->transfer_queue_index;
	u32 index_count = 1;

	if (!compute_shares_graphics_queue) {
		index_count++;

		if (!present_shares_graphics_queue && !present_shares_compute_queue) {
			index_count++;
		}
		if (!transfer_shares_graphics_queue && !transfer_shares_compute_queue) {
			index_count++;
		}
	} else {
		if (!present_shares_graphics_queue) {
			index_count++;
		}
		if (!transfer_shares_graphics_queue) {
			index_count++;
		}
	}

	u32 indices[index_count];
	u8  index        = 0;
	indices[index++] = out_device->graphics_queue_index;
	if (!compute_shares_graphics_queue) {
		indices[index++] = out_device->compute_queue_index;

		if (!present_shares_graphics_queue && !present_shares_compute_queue) {
			indices[index++] = out_device->present_queue_index;
		}
		if (!transfer_shares_graphics_queue && !transfer_shares_compute_queue) {
			indices[index++] = out_device->transfer_queue_index;
		}
	} else {
		if (!present_shares_graphics_queue) {
			indices[index++] = out_device->present_queue_index;
		}
		if (!transfer_shares_graphics_queue) {
			indices[index++] = out_device->transfer_queue_index;
		}
	}

	VkDeviceQueueCreateInfo queue_create_infos[index_count];
	for (u32 i = 0; i < index_count; ++i) {
		queue_create_infos[i].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = indices[i];
		queue_create_infos[i].queueCount       = 1;
		if (indices[i] == out_device->graphics_queue_index) {
			queue_create_infos[i].queueCount = 2;
		}
		queue_create_infos[i].flags            = 0;
		queue_create_infos[i].pNext            = NULL;
		f32 queue_priority                     = 1.0f;
		queue_create_infos[i].pQueuePriorities = &queue_priority;
	}

	// TODO: Make configurable
	VkPhysicalDeviceFeatures device_features = {};
	device_features.samplerAnisotropy        = VK_TRUE;

	VkDeviceCreateInfo create_info      = {};
	create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount    = index_count;
	create_info.pQueueCreateInfos       = queue_create_infos;
	create_info.pEnabledFeatures        = &device_features;
	create_info.enabledExtensionCount   = 1;
	const char *extension_names         = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	create_info.ppEnabledExtensionNames = &extension_names;

	// Deprecated
	create_info.enabledLayerCount   = 0;
	create_info.ppEnabledLayerNames = NULL;

	if (vkCreateDevice(
			out_device->physical_device, &create_info, NULL, &out_device->logical_device
		) != VK_SUCCESS) {
		return false;
	}

	vkGetDeviceQueue(
		out_device->logical_device, out_device->graphics_queue_index, 0, &out_device->graphics_queue
	);

	vkGetDeviceQueue(
		out_device->logical_device, out_device->compute_queue_index, 0, &out_device->compute_queue
	);

	vkGetDeviceQueue(
		out_device->logical_device, out_device->present_queue_index, 0, &out_device->present_queue
	);

	vkGetDeviceQueue(
		out_device->logical_device, out_device->transfer_queue_index, 0, &out_device->transfer_queue
	);

	return true;
}

void vulkan_device_destroy(vulkan_device *device) {
	device->graphics_queue = NULL;
	device->transfer_queue = NULL;
	device->present_queue  = NULL;
	device->compute_queue  = NULL;

	vkDestroyDevice(device->logical_device, NULL);

	device->physical_device = NULL;

	if (device->swapchain_support.formats) {
		free(device->swapchain_support.formats);
		device->swapchain_support.formats      = NULL;
		device->swapchain_support.format_count = 0;
	}

	if (device->swapchain_support.present_modes) {
		free(device->swapchain_support.present_modes);
		device->swapchain_support.present_modes      = NULL;
		device->swapchain_support.present_mode_count = 0;
	}

	memset(
		&device->swapchain_support.capabilities, 0, sizeof(device->swapchain_support.capabilities)
	);

	device->graphics_queue_index = -1;
	device->present_queue_index  = -1;
	device->transfer_queue_index = -1;
	device->compute_queue_index  = -1;
}

void vulkan_device_query_swapchain_support(
	VkPhysicalDevice               physical_device,
	VkSurfaceKHR                   surface,
	vulkan_swapchain_support_info *out_support_info
) {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device, surface, &out_support_info->capabilities
	);

	vkGetPhysicalDeviceSurfaceFormatsKHR(
		physical_device, surface, &out_support_info->format_count, 0
	);
	if (out_support_info->format_count != 0) {
		if (!out_support_info->formats) {
			out_support_info->formats =
				malloc(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device, surface, &out_support_info->format_count, out_support_info->formats
			);
		}
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(
		physical_device, surface, &out_support_info->present_mode_count, 0
	);
	if (out_support_info->present_mode_count != 0) {
		if (!out_support_info->present_modes) {
			out_support_info->present_modes =
				malloc(sizeof(VkPresentModeKHR) * out_support_info->format_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				physical_device,
				surface,
				&out_support_info->present_mode_count,
				out_support_info->present_modes
			);
		}
	}
}

b8 vulkan_device_detect_depth_format(vulkan_device *device) {
	const u64 candidate_count = 3;
	VkFormat  candidates[3]   = { VK_FORMAT_D32_SFLOAT,
		                          VK_FORMAT_D32_SFLOAT_S8_UINT,
		                          VK_FORMAT_D24_UNORM_S8_UINT };

	u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	for (u64 i = 0; i < candidate_count; ++i) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

		if ((properties.linearTilingFeatures & flags) == flags) {
			device->depth_format = candidates[i];
			return true;
		} else if ((properties.optimalTilingFeatures & flags) == flags) {
			device->depth_format = candidates[i];
			return true;
		}
	}
	return false;
}

b8 select_physical_device(VkInstance instance, VkSurfaceKHR surface, vulkan_device *out_device) {
	u32 device_count;
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	VkPhysicalDevice physical_devices[device_count];
	vkEnumeratePhysicalDevices(instance, &device_count, physical_devices);
	for (u32 i = 0; i < device_count; ++i) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

		VkPhysicalDeviceMemoryProperties memory;
		vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

		// TODO: Make these configurable
		vulkan_physical_device_requirements requirements = {};
		requirements.graphics                            = true;
		requirements.present                             = true;
		requirements.compute                             = true;
		requirements.transfer                            = true;
		requirements.sampler_anisotropy                  = true;
		requirements.discrete_gpu                        = false;
		requirements.device_extension_names              = darray_create(const char *);
		darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vulkan_physical_device_queue_family_info queue_info = {};

		b8 result = physical_device_meets_requirements(
			physical_devices[i],
			surface,
			&properties,
			&features,
			&requirements,
			&queue_info,
			&out_device->swapchain_support
		);
		if (result) {
			printf("Selected device: '%s'.\n", properties.deviceName);
			switch (properties.deviceType) {
				default:
				case VK_PHYSICAL_DEVICE_TYPE_OTHER:
					printf("GPU type is Unknown.\n");
					break;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					printf("GPU type is Integrated.\n");
					break;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					printf("GPU type is Discrete.\n");
					break;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					printf("GPU type is Virtual.\n");
					break;
				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					printf("GPU type is CPU.\n");
					break;
			}

			printf(
				"GPU Driver version: %d.%d.%d\n",
				VK_VERSION_MAJOR(properties.driverVersion),
				VK_VERSION_MINOR(properties.driverVersion),
				VK_VERSION_PATCH(properties.driverVersion)
			);

			printf(
				"Vulkan API version: %d.%d.%d\n",
				VK_VERSION_MAJOR(properties.apiVersion),
				VK_VERSION_MINOR(properties.apiVersion),
				VK_VERSION_PATCH(properties.apiVersion)
			);

			for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
				f32 memory_size_gib =
					(((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
				if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
					printf("Local GPU memory: %.2f GiB\n", memory_size_gib);
				} else {
					printf("Shared System memory: %.2f GiB\n", memory_size_gib);
				}
			}

			out_device->physical_device      = physical_devices[i];
			out_device->graphics_queue_index = queue_info.graphics_family_index;
			out_device->compute_queue_index  = queue_info.compute_family_index;
			out_device->present_queue_index  = queue_info.present_family_index;
			out_device->transfer_queue_index = queue_info.transfer_family_index;

			out_device->properties = properties;
			out_device->features   = features;
			out_device->memory     = memory;

			break;
		}
	}

	if (!out_device->physical_device) {
		return false;
	}

	return true;
}

b8 physical_device_meets_requirements(
	VkPhysicalDevice                           device,
	VkSurfaceKHR                               surface,
	const VkPhysicalDeviceProperties          *properties,
	const VkPhysicalDeviceFeatures            *features,
	const vulkan_physical_device_requirements *requirements,
	vulkan_physical_device_queue_family_info  *out_queue_info,
	vulkan_swapchain_support_info             *out_swapchain_support
) {

	out_queue_info->graphics_family_index = -1;
	out_queue_info->present_family_index  = -1;
	out_queue_info->compute_family_index  = -1;
	out_queue_info->transfer_family_index = -1;

	if (requirements->discrete_gpu) {
		if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			// TODO: Error Message
			return false;
		}
	}

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	VkQueueFamilyProperties queue_families[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	printf("Graphics | Present | Compute | Transfer | Name\n");
	u8 min_transfer_score = 255;
	for (int i = 0; i < queue_family_count; ++i) {
		u8 current_transfer_score = 0;

		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			out_queue_info->graphics_family_index = i;
			++current_transfer_score;
		}

		if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			out_queue_info->compute_family_index = i;
			++current_transfer_score;
		}

		if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			if (current_transfer_score <= min_transfer_score) {
				min_transfer_score                    = current_transfer_score;
				out_queue_info->transfer_family_index = i;
			}
		}

		VkBool32 supports_present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present);
		if (supports_present) {
			out_queue_info->present_family_index = i;
		}
	}

	printf(
		"%8d | %7d | %7d | %8d | %s\n",
		out_queue_info->graphics_family_index != -1,
		out_queue_info->present_family_index != -1,
		out_queue_info->compute_family_index != -1,
		out_queue_info->transfer_family_index != -1,
		properties->deviceName
	);

	if ((!requirements->graphics ||
	     (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
	    (!requirements->present ||
	     (requirements->present && out_queue_info->present_family_index != -1)) &&
	    (!requirements->compute ||
	     (requirements->compute && out_queue_info->compute_family_index != -1)) &&
	    (!requirements->transfer ||
	     (requirements->transfer && out_queue_info->transfer_family_index != -1))) {
		// TODO: Print debug info about meeting queue requirements
	}

	vulkan_device_query_swapchain_support(device, surface, out_swapchain_support);

	if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
		if (out_swapchain_support->formats) {
			free(out_swapchain_support->formats);
		}
		if (out_swapchain_support->present_modes) {
			free(out_swapchain_support->present_modes);
		}
		// TODO: Print debug info
		return false;
	}

	if (requirements->device_extension_names) {
		u32                    available_extension_count = 0;
		VkExtensionProperties *available_extensions      = NULL;
		vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, NULL);
		if (available_extension_count != 0) {
			available_extensions =
				malloc(sizeof(VkExtensionProperties) * available_extension_count);
			vkEnumerateDeviceExtensionProperties(
				device, NULL, &available_extension_count, available_extensions
			);

			u32 required_extension_count = darray_length(requirements->device_extension_names);
			for (u32 i = 0; i < required_extension_count; ++i) {
				b8 found = false;
				for (u32 j = 0; j < available_extension_count; ++j) {
					if (strcmp(
							requirements->device_extension_names[i],
							available_extensions[j].extensionName
						) == 0) {
						found = true;
						break;
					}
				}
				if (!found) {
					// TODO: Print debug info
					free(available_extensions);
					return false;
				}
			}

			free(available_extensions);
		}
	}

	if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
		// TODO: Print debug info
		return false;
	}

	return true;
}
