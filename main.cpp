#include <iostream>
#include "ametrine.h"

typedef union {
    unsigned char rgba[4];
    struct {
        unsigned char r, g, b, a;
    };
} Pixel;

Pixel grid[100][100] = {0};

am_id defaultShaderId;
am_id defaultTextureId;
am_id samplerId;
am_id cubeVboId;
am_id cubeIboId;
am_id cubePipelineId;
am_id viewMatrixId;
am_id cameraPosId;
am_id defaultRenderPassId;
am_camera camera;
am_mat4 cameraView;

void updateCamera(am_camera *camera) {
    am_platform *platform = am_engine_get_subsystem(platform);
    am_float64 dt = platform->time.delta;
    am_vec2 dm = am_platform_mouse_get_delta();

    am_camera_offset_orientation(camera, -0.1f * dm.x, -0.1f * dm.y);

    am_vec3 vel = {0};
    if (am_platform_key_down(AM_KEYCODE_W)) vel = am_vec3_add(vel, am_camera_forward(camera));
    if (am_platform_key_down(AM_KEYCODE_S)) vel = am_vec3_add(vel, am_camera_backward(camera));
    if (am_platform_key_down(AM_KEYCODE_A)) vel = am_vec3_add(vel, am_camera_left(camera));
    if (am_platform_key_down(AM_KEYCODE_D)) vel = am_vec3_add(vel, am_camera_right(camera));
    if (am_platform_key_down(AM_KEYCODE_SPACE)) vel = am_vec3_add(vel, am_camera_up(camera));
    if (am_platform_key_down(AM_KEYCODE_LEFT_CONTROL)) vel = am_vec3_add(vel, am_camera_down(camera));

    camera->transform.position = am_vec3_add(camera->transform.position, am_vec3_scale((am_float32)(5000.0f * dt), am_vec3_norm(vel)));
};

void init() {
    camera = am_camera_perspective();
    camera.transform.position = am_vec3_create(0.0f, 1.0f, 3.0f);
    am_platform_mouse_lock(true);

    defaultShaderId = amgl_shader_create((amgl_shader_info) {
        .sources = (amgl_shader_source_info[]) {
            { .type = AMGL_SHADER_VERTEX, .path = "../shaders/default.vert" },
            { .type = AMGL_SHADER_FRAGMENT, .path = "../shaders/default.frag" }
        },
        .num_sources = 2
    });

    am_float32 cubeCoords[12*6] = {
            //Pos coords
            -1.0f, 1.0f, 1.0f,
            1.0f,1.0f,1.0f,
            1.0f,1.0f,-1.0f,
            -1.0f,1.0f,-1.0f,

            -1.0f,1.0f,-1.0f,
            1.0f,1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,

            1.0f,1.0f,-1.0f,
            1.0f,1.0f,1.0f,
            1.0f,-1.0f,1.0f,
            1.0f,-1.0f,-1.0f,

            -1.0f,1.0f,1.0f,
            -1.0f,1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f,1.0f,

            1.0f,1.0f,1.0f,
            -1.0f,1.0f,1.0f,
            -1.0f,-1.0f,1.0f,
            1.0f,-1.0f,1.0f,

            1.0f,-1.0f,-1.0f,
            1.0f,-1.0f,1.0f,
            -1.0f,-1.0f,1.0f,
            -1.0f,-1.0f,-1.0f
    };
    cubeVboId = amgl_vertex_buffer_create((amgl_vertex_buffer_info){
            .data = cubeCoords,
            .size = sizeof(float)*12*6,
            .usage = AMGL_BUFFER_USAGE_STATIC,
    });

    cubeIboId = amgl_index_buffer_create((amgl_index_buffer_info){
            .data = (am_uint32[]){
                    0,1,2,0,2,3,
                    4,5,6,4,6,7,
                    8,9,10,8,10,11,
                    12,13,14,12,14,15,
                    16,17,18,16, 18,19,
                    20, 21, 22, 20, 22, 23
            },
            .size = sizeof(am_uint32)*36,
            .offset = 0,
            .usage = AMGL_BUFFER_USAGE_STATIC
    });

    viewMatrixId = amgl_uniform_create((amgl_uniform_info){
            .name = "uView",
            .type = AMGL_UNIFORM_TYPE_MAT4
    });
    cameraPosId = amgl_uniform_create((amgl_uniform_info){
            .name = "uCameraPos",
            .type = AMGL_UNIFORM_TYPE_VEC3
    });

    cubePipelineId = amgl_pipeline_create((amgl_pipeline_info){
            .depth = {.func = AMGL_DEPTH_FUNC_LESS},
            .raster = {
                    .face_culling = AMGL_FACE_CULL_BACK,
                    .shader_id = defaultShaderId
            },
            .layout = {
                    .attributes = (amgl_vertex_buffer_attribute[]){
                            {.format = AMGL_VERTEX_BUFFER_ATTRIBUTE_FLOAT3, .offset = 0, .stride = 3*sizeof(float), .buffer_index = 0},
                            {.format = AMGL_VERTEX_BUFFER_ATTRIBUTE_FLOAT2, .offset = 0, .stride = 2*sizeof(float), .buffer_index = 1},
                            {.format = AMGL_VERTEX_BUFFER_ATTRIBUTE_FLOAT3, .offset = 0, .stride = 3*sizeof(float), .buffer_index = 2}
                    },
                    .num_attribs = 3
            }
    });

    defaultRenderPassId = amgl_render_pass_create((amgl_render_pass_info){0});
}

void update() {
    if (am_platform_key_pressed(AM_KEYCODE_ESCAPE)) am_engine_quit();
    am_platform *platform = am_engine_get_subsystem(platform);
    am_window *main = am_packed_array_get_ptr(platform->windows, 1);

    updateCamera(&camera);
    cameraView = am_camera_get_view_projection(&camera, (am_int32)main->width, (am_int32)main->height);

    amgl_bindings_info cubeBinds = {
            .vertex_buffers = {
                    .size = sizeof(amgl_vertex_buffer_bind_info),
                    .info = (amgl_vertex_buffer_bind_info[]) {
                            {.vertex_buffer_id = cubeVboId}
                    }
            },
            .index_buffers = {.info = (amgl_index_buffer_bind_info[])
                    {{.index_buffer_id = cubeIboId}}},
            .uniforms = {
                    .size = 2*sizeof(amgl_uniform_bind_info),
                    .info = (amgl_uniform_bind_info[]) {
                            {.uniform_id = viewMatrixId, .data = cameraView.elements},
                            {.uniform_id = cameraPosId, .data = camera.transform.position.xyz},
                    }
            }
    };

    amgl_start_render_pass(defaultRenderPassId);
    amgl_clear((amgl_clear_desc) {
            .color = {0.0f, 0.0f, 0.0f, 0.0f},
            .num = 2,
            .types = (amgl_clear_type[]){AMGL_CLEAR_COLOR, AMGL_CLEAR_DEPTH}
    });


    amgl_bind_pipeline(cubePipelineId);
    amgl_set_bindings(&cubeBinds);
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
