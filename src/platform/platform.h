#ifndef WORLD_GENERATOR_PLATFORM_H
#define WORLD_GENERATOR_PLATFORM_H

#include "types.h"

typedef struct platform_state {
	void *internal_state;
} platform_state;

b8 platform_init(platform_state *state);

void platform_terminate(platform_state *state);

#endif // WORLD_GENERATOR_PLATFORM_H
