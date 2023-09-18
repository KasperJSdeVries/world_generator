#include "../renderer_backend.h"
#include "containers/darray.h"
#include "vulkan_context.h"
#include "vulkan_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

b8 create_instance(const char *application_name, VkInstance *out_instance);
#if defined(_DEBUG)
void create_debug_messenger(VkInstance instance,
							VkDebugUtilsMessengerEXT *out_debug_messenger);
#endif
b8 pick_physical_device(VkInstance instance, VkSurfaceKHR surface,
						VkPhysicalDevice *out_physical_device,
						swapchain_support_info *swapchain_info);

b8 backend_initialize(platform_state platform_state, void **out_renderer_context) {
	*out_renderer_context = malloc(sizeof(vulkan_context));
	vulkan_context *context = *out_renderer_context;

	if (!create_instance("World Generator", &context->instance)) {
		return false;
	}

#if defined(_DEBUG)
	create_debug_messenger(context->instance, &context->debug_messenger);
#endif

	if (!platform_get_surface(context->instance, platform_state, &context->surface)) {
		return false;
	}

	if (!pick_physical_device(context->instance, context->surface, &context->physical_device,
							  &context->swapchain_info)) {
		return false;
	}

	return true;
}

void backend_shutdown(void *renderer_context) {
	vulkan_context *context = renderer_context;

	vkDestroySurfaceKHR(context->instance, context->surface, NULL);

#if defined(_DEBUG)
	PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_function =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			context->instance, "vkDestroyDebugUtilsMessengerEXT");
	destroy_debug_messenger_function(context->instance,
									 context->debug_messenger, NULL);
#endif
	vkDestroyInstance(context->instance, NULL);
	free(context);
}

b8 create_instance(const char *application_name, VkInstance *out_instance) {
	VkApplicationInfo application_info = {};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion = VK_API_VERSION_1_3;
	application_info.pApplicationName = application_name;
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.pEngineName = "World Generator Engine";
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &application_info;

	const char **required_extensions = darray_create(const char *);
	platform_get_required_extension_names(&required_extensions);

#if defined(_DEBUG)
	darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	printf("Required extensions:\n");
	for (u32 i = 0; i < darray_length(required_extensions); ++i) {
		printf("\t%s\n", required_extensions[i]);
	}
#endif

	create_info.enabledExtensionCount = darray_length(required_extensions);
	create_info.ppEnabledExtensionNames = required_extensions;

	const char **required_validation_layer_names = 0;
	u32 required_validation_layer_count = 0;

#if defined(_DEBUG)
	required_validation_layer_names = darray_create(const char *);
	darray_push(required_validation_layer_names,
				&"VK_LAYER_KHRONOS_validation");
	required_validation_layer_count =
		darray_length(required_validation_layer_names);

	u32 available_layer_count = 0;
	vkEnumerateInstanceLayerProperties(&available_layer_count, NULL);
	VkLayerProperties *available_layers =
		darray_reserve(VkLayerProperties, available_layer_count);
	vkEnumerateInstanceLayerProperties(&available_layer_count,
									   available_layers);

	for (u32 i = 0; i < required_validation_layer_count; ++i) {
		b8 found = false;
		for (u32 j = 0; j < available_layer_count; ++j) {
			if (strcmp(required_validation_layer_names[i],
					   available_layers[j].layerName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "Required validation layer is missing: %s\n",
					required_validation_layer_names[i]);
			return false;
		}
	}
#endif

	create_info.enabledLayerCount = required_validation_layer_count;
	create_info.ppEnabledLayerNames = required_validation_layer_names;

	if (vkCreateInstance(&create_info, NULL, out_instance) != VK_SUCCESS) {
		return false;
	}
	return true;
}

#if defined(_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
				  VkDebugUtilsMessageTypeFlagBitsEXT message_types,
				  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
				  void *user_data) {
	switch (message_severity) {
	default:
		printf("%s\n", callback_data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		fprintf(stderr, "%s\n", callback_data->pMessage);
		break;
	}
	return VK_FALSE;
}

void create_debug_messenger(VkInstance instance,
							VkDebugUtilsMessengerEXT *out_debug_messenger) {
	u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	VkDebugUtilsMessengerCreateInfoEXT create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = log_severity;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = vk_debug_callback;
	create_info.pUserData = 0;

	PFN_vkCreateDebugUtilsMessengerEXT create_function =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
	if (create_function(instance, &create_info, NULL, out_debug_messenger) !=
		VK_SUCCESS) {
		fprintf(stderr, "Failed to create Debug Messenger!\n");
	}
}
#endif

b8 physical_device_meets_requirements(VkPhysicalDevice device, VkSurfaceKHR surface,
									  const VkPhysicalDeviceProperties *properties,
									  const VkPhysicalDeviceFeatures *features,
									  queue_family_info *out_queue_info,
									  swapchain_support_info *out_swapchain_support);

b8 pick_physical_device(VkInstance instance, VkSurfaceKHR surface,
						VkPhysicalDevice *out_physical_device,
						swapchain_support_info *swapchain_info) {
	u32 device_count;
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	VkPhysicalDevice physical_devices[device_count];
	vkEnumeratePhysicalDevices(instance, &device_count, physical_devices);
	for (u32 i = 0; i < device_count; ++i) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory_properties);

		queue_family_info queue_info = {};
		b8 result = physical_device_meets_requirements(physical_devices[i], surface, &properties,
													   &features, &queue_info, swapchain_info);
		if (result) {
			*out_physical_device = physical_devices[i];
			return true;
		}
	}
	return false;
}

void query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface,
							 swapchain_support_info *out_swapchain_support) {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
											  &out_swapchain_support->capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out_swapchain_support->format_count, 0);
	if (out_swapchain_support->format_count != 0) {
		if (!out_swapchain_support->formats) {
			out_swapchain_support->formats =
				malloc(sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
												 &out_swapchain_support->format_count,
												 out_swapchain_support->formats);
		}
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
											  &out_swapchain_support->present_mode_count, 0);
	if (out_swapchain_support->present_mode_count != 0) {
		if (!out_swapchain_support->present_modes) {
			out_swapchain_support->present_modes =
				malloc(sizeof(VkPresentModeKHR) * out_swapchain_support->format_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
													  &out_swapchain_support->present_mode_count,
													  out_swapchain_support->present_modes);
		}
	}
}

b8 physical_device_meets_requirements(VkPhysicalDevice device, VkSurfaceKHR surface,
									  const VkPhysicalDeviceProperties *properties,
									  const VkPhysicalDeviceFeatures *features,
									  queue_family_info *out_queue_info,
									  swapchain_support_info *out_swapchain_support) {

	out_queue_info->graphics_family_index = -1;
	out_queue_info->present_family_index = -1;
	out_queue_info->compute_family_index = -1;
	out_queue_info->transfer_family_index = -1;

	switch (properties->deviceType) {
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		break;
	default:
		return false;
	}

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	VkQueueFamilyProperties queue_families[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
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
				min_transfer_score = current_transfer_score;
				out_queue_info->transfer_family_index = i;
			}
		}

		VkBool32 supports_present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present);
		if (supports_present) {
			out_queue_info->present_family_index = i;
		}
	}

	if (!((out_queue_info->graphics_family_index != -1) &&
		  (out_queue_info->compute_family_index != -1) &&
		  (out_queue_info->transfer_family_index != -1) &&
		  (out_queue_info->present_family_index != -1))) {
		return false;
	}

	query_swapchain_support(device, surface, out_swapchain_support);

	if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
		if (out_swapchain_support->formats) {
			free(out_swapchain_support->formats);
		}
		if (out_swapchain_support->present_modes) {
			free(out_swapchain_support->present_modes);
		}
		return false;
	}

	u32 available_extension_count = 0;
	VkExtensionProperties *available_extensions = NULL;
	vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, NULL);
	if (available_extension_count != 0) {
		available_extensions = malloc(sizeof(VkExtensionProperties) * available_extension_count);
		vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count,
											 available_extensions);
		// TODO: Make configurable
		const char **required_extensions = darray_create(const char *);
		//darray_push(required_extensions, &VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		darray_push(required_extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		u32 required_extension_count = darray_length(required_extensions);
		for (u32 i = 0; i < required_extension_count; ++i) {
			b8 found = false;
			for (u32 j = 0; j < available_extension_count; ++j) {
				if (strcmp(required_extensions[i], available_extensions[j].extensionName) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				darray_destroy(required_extensions);
				free(available_extensions);
				return false;
			}
		}

		darray_destroy(required_extensions);
		free(available_extensions);
	}

	if (!features->samplerAnisotropy) {
		return false;
	}

	return true;
}
