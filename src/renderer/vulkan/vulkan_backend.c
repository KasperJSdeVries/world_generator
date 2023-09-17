#include "../renderer_backend.h"
#include "containers/darray.h"
#include "vulkan_context.h"
#include "vulkan_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

b8 create_instance(vulkan_context *context, const char *application_name);

b8 backend_initialize(void **out_renderer_context) {
	*out_renderer_context = malloc(sizeof(vulkan_context));
	vulkan_context *context = *out_renderer_context;

	if (!create_instance(context, "World Generator")) {
		return false;
	}

	return true;
}

void backend_shutdown(void *renderer_context) {
	vulkan_context *context = renderer_context;

	vkDestroyInstance(context->instance, NULL);
	free(context);
}

b8 create_instance(vulkan_context *context, const char *application_name) {
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

	if (vkCreateInstance(&create_info, NULL, &context->instance) !=
		VK_SUCCESS) {
		return false;
	}
	return true;
}
