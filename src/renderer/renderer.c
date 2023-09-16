#include "renderer.h"
#include "renderer_backend.h"

b8 renderer_initialize(renderer_state *out_renderer_state) {
	backend_initialize(&out_renderer_state->renderer_context);
}

void renderer_shutdown(renderer_state *renderer_state) {
	backend_shutdown(renderer_state->renderer_context);
}

b8 renderer_draw_frame(renderer_state *renderer_state,
					   render_packet *render_packet) {}
