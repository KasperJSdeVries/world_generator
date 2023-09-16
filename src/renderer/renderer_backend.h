#ifndef WORLD_GENERATOR_RENDERER_BACKEND_H
#define WORLD_GENERATOR_RENDERER_BACKEND_H

#include "types.h"

b8 backend_initialize(void **out_renderer_context);
void backend_shutdown(void *renderer_context);

#endif // WORLD_GENERATOR_RENDERER_BACKEND_H
