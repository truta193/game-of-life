#include "ametrine.h"

typedef union {
    unsigned char rgba[4];
    struct {
        unsigned char r, g, b, a;
    };
} Pixel;

Pixel grid[100][100] = {0};

am_id defaultShaderId;
am_id rectangleVBOId;
am_id rectangleIBOId;
am_id rectanglePipelineId;
am_id defaultRenderPassId;
am_id textureId;
am_id uniformSamplerId;

void init() {

    defaultShaderId = amgl_shader_create((amgl_shader_info) {
        .sources = (amgl_shader_source_info[]) {
            { .type = AMGL_SHADER_VERTEX, .path = "../resources/shaders/default.vert" },
            { .type = AMGL_SHADER_FRAGMENT, .path = "../resources/shaders/default.frag" }
        },
        .num_sources = 2
    });

    am_float32 rectangleCoords[4 * 5] = {
            //XYZ coords                        //UV coords
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f,0.0f,  1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f , 0.0f, 1.0f
    };
    rectangleVBOId = amgl_vertex_buffer_create((amgl_vertex_buffer_info){
            .data = rectangleCoords,
            .size = sizeof(float)*4*5,
            .usage = AMGL_BUFFER_USAGE_STATIC,
    });

    rectangleIBOId = amgl_index_buffer_create((amgl_index_buffer_info){
            .data = (am_uint32[]){
                    0,1,2,
                    0,2,3
            },
            .size = sizeof(am_uint32)*6,
            .offset = 0,
            .usage = AMGL_BUFFER_USAGE_STATIC
    });


    rectanglePipelineId = amgl_pipeline_create((amgl_pipeline_info){
            .depth = {.func = AMGL_DEPTH_FUNC_LESS},
            .raster = {
                    .face_culling = AMGL_FACE_CULL_BACK,
                    .shader_id = defaultShaderId
            },
            .layout = {
                    .attributes = (amgl_vertex_buffer_attribute[]){
                            {.format = AMGL_VERTEX_BUFFER_ATTRIBUTE_FLOAT3, .offset = 0, .stride = 5*sizeof(float)},
                            {.format = AMGL_VERTEX_BUFFER_ATTRIBUTE_FLOAT2, .offset = 3*sizeof(float), .stride = 5*sizeof(float)}
                    },
                    .num_attribs = 2
            }
    });

    textureId = amgl_texture_create((amgl_texture_info){
            .path = "../resources/images/test.png",
            .format = AMGL_TEXTURE_FORMAT_RGBA,
            .wrap_s = AMGL_TEXTURE_WRAP_REPEAT,
            .wrap_t = AMGL_TEXTURE_WRAP_REPEAT,
            .min_filter = AMGL_TEXTURE_FILTER_LINEAR,
            .mag_filter = AMGL_TEXTURE_FILTER_LINEAR
    });

    uniformSamplerId = amgl_uniform_create((amgl_uniform_info){
            .name = "uSampler",
            .type = AMGL_UNIFORM_TYPE_SAMPLER2D
    });

    defaultRenderPassId = amgl_render_pass_create((amgl_render_pass_info){0});
}

void update() {
    amgl_set_viewport( 0,0, am_platform_window_get_size(1).x, am_platform_window_get_size(1).y);
    if (am_platform_key_pressed(AM_KEYCODE_ESCAPE)) am_engine_quit();
    am_platform *platform = am_engine_get_subsystem(platform);
    am_window *main = am_packed_array_get_ptr(platform->windows, 1);

    amgl_bindings_info triangleBinds = {
            .vertex_buffers = {
                    .size = sizeof(amgl_vertex_buffer_bind_info),
                    .info = (amgl_vertex_buffer_bind_info[]) {
                            {.vertex_buffer_id = rectangleVBOId}
                    }
            },
            .index_buffers = {.info = (amgl_index_buffer_bind_info[])
                    {{.index_buffer_id = rectangleIBOId}}},
            .uniforms = {
                    .size = sizeof(amgl_uniform_bind_info),
                    .info = (amgl_uniform_bind_info[]) {
                            {.uniform_id = uniformSamplerId, .data = &textureId}
                    }
            }
    };

    amgl_start_render_pass(defaultRenderPassId);
    amgl_clear((amgl_clear_desc) {
            .color = {0.0f, 0.0f, 0.0f, 0.0f},
            .num = 2,
            .types = (amgl_clear_type[]){AMGL_CLEAR_COLOR, AMGL_CLEAR_DEPTH}
    });


    amgl_bind_pipeline(rectanglePipelineId);
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
