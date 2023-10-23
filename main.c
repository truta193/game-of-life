#include "ametrine.h"

typedef union {
    unsigned char rgba[4];
    struct {
        unsigned char r, g, b, a;
    };
} Pixel;

Pixel grid[100][100] = {0};

am_id defaultShaderId;
am_id triangleVboId;
am_id triangleIboId;
am_id trianglePipelineId;
am_id defaultRenderPassId;

void init() {

    defaultShaderId = amgl_shader_create((amgl_shader_info) {
        .sources = (amgl_shader_source_info[]) {
            { .type = AMGL_SHADER_VERTEX, .path = "../shaders/default.vert" },
            { .type = AMGL_SHADER_FRAGMENT, .path = "../shaders/default.frag" }
        },
        .num_sources = 2
    });

    am_float32 triangleCoords[3*3] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f,0.0f,
            0.0f, 0.5f, 0.0f
    };
    triangleVboId = amgl_vertex_buffer_create((amgl_vertex_buffer_info){
            .data = triangleCoords,
            .size = sizeof(float)*3*3,
            .usage = AMGL_BUFFER_USAGE_STATIC,
    });

    triangleIboId = amgl_index_buffer_create((amgl_index_buffer_info){
            .data = (am_uint32[]){
                    0,1,2
            },
            .size = sizeof(am_uint32)*3,
            .offset = 0,
            .usage = AMGL_BUFFER_USAGE_STATIC
    });


    trianglePipelineId = amgl_pipeline_create((amgl_pipeline_info){
            .depth = {.func = AMGL_DEPTH_FUNC_LESS},
            .raster = {
                    .face_culling = AMGL_FACE_CULL_BACK,
                    .shader_id = defaultShaderId
            },
            .layout = {
                    .attributes = (amgl_vertex_buffer_attribute[]){
                            {.format = AMGL_VERTEX_BUFFER_ATTRIBUTE_FLOAT3, .offset = 0, .stride = 3*sizeof(float), .buffer_index = 0}
                    },
                    .num_attribs = 1
            }
    });

    defaultRenderPassId = amgl_render_pass_create((amgl_render_pass_info){0});
}

void update() {
    if (am_platform_key_pressed(AM_KEYCODE_ESCAPE)) am_engine_quit();
    am_platform *platform = am_engine_get_subsystem(platform);
    am_window *main = am_packed_array_get_ptr(platform->windows, 1);

    amgl_bindings_info triangleBinds = {
            .vertex_buffers = {
                    .size = sizeof(amgl_vertex_buffer_bind_info),
                    .info = (amgl_vertex_buffer_bind_info[]) {
                            {.vertex_buffer_id = triangleVboId}
                    }
            },
            .index_buffers = {.info = (amgl_index_buffer_bind_info[])
                    {{.index_buffer_id = triangleIboId}}},
            .uniforms = {
                    .size = 0
            }
    };

    amgl_start_render_pass(defaultRenderPassId);
    amgl_clear((amgl_clear_desc) {
            .color = {0.0f, 0.0f, 0.0f, 0.0f},
            .num = 2,
            .types = (amgl_clear_type[]){AMGL_CLEAR_COLOR, AMGL_CLEAR_DEPTH}
    });


    amgl_bind_pipeline(trianglePipelineId);
    amgl_set_bindings(&triangleBinds);
    amgl_draw((amgl_draw_info[]){{.start = 0, .count = 36}});


    amgl_end_render_pass(defaultRenderPassId);

}

void am_shutdown() {}

int main() {
    am_engine_create((am_engine_info) {
            .init = init,
            .update = update,
            .shutdown = am_shutdown,
            .is_running = true,
            .vsync_enabled = true,
            .desired_fps = 60,
            .win_fullscreen = false,
            .win_width = 800,
            .win_height = 800,
            .win_x = 50,
            .win_y = 50
    });

    while (am_engine_get_instance()->is_running) am_engine_frame();
    return 0;
}
