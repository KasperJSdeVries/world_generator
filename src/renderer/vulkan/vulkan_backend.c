#include "../renderer_backend.h"
#include "vulkan_context.h"
#include <stdlib.h>

b8 backend_initialize(void **out_renderer_context) {
	*out_renderer_context = malloc(sizeof(vulkan_context));
	vulkan_context *context = *out_renderer_context;
	if (vkCreateInstance(NULL, NULL, &context->instance) != VK_SUCCESS) {
		return false;
	}
	return true;
}

void backend_shutdown(void *renderer_context) {
	vulkan_context *context = renderer_context;

	vkDestroyInstance(context->instance, NULL);
	free(context);
}
