#ifndef WORLD_GENERATOR_RENDERER_H
#define WORLD_GENERATOR_RENDERER_H

#include "types.h"

typedef struct renderer_state {
	void *renderer_context;
} renderer_state;

typedef struct render_packet {
	f32 delta_time;
} render_packet;

b8 renderer_initialize(renderer_state *out_renderer_state);
void renderer_shutdown(renderer_state *renderer_state);
b8 renderer_draw_frame(renderer_state *renderer_state,
					   render_packet *render_packet);

#endif // WORLD_GENERATOR_RENDERER_H
