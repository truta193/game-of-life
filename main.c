#include "ametrine.h"

#define RGBA(r,g,b,a) (Pixel){r,g,b,a}
/**
 * Pixel struct that represents a single pixel in the frameGrid.
 */
typedef union {
    unsigned char rgba[4];
    struct {
        unsigned char r, g, b, a;
    };
} Pixel;

/**
 * Cell struct that represents a single cell in the frameGrid.
 */
typedef struct cell {
    int is_alive;
} Cell;

/**
 * Game state struct that represents the state of the game.
 * It contains the width and height of the frameGrid, the frameGrid itself, and the display.
 * Contains 2 grids because the game uses the double buffering technique.
 *
 */
typedef struct gamestate {
    int width;
    int height;
    Pixel *display;
    Cell *frameGrid;
    Cell *computeGrid;
} GameState;

am_id defaultShaderId;
am_id rectangleVBOId;
am_id rectangleIBOId;
am_id rectanglePipelineId;
am_id defaultRenderPassId;
am_id textureId;
am_id uniformSamplerId;

GameState gameState = {0};

am_bool runGame = false;

/**
 * Initializes the game state.
 * @param width Game width (number of cells)
 * @param height Game height (number of cells)
 */
void initGameState(int width, int height);

/**
 * Updates the game state.
 * This function is called every frame.
 */
void updateGameState();

/**
 * Updates the cells in the frameGrid.
 */
void updateCells();

void initGameState(int width, int height) {
    gameState.width = width;
    gameState.height = height;
    gameState.display = calloc(sizeof(Pixel), width * height);
    gameState.frameGrid = calloc(sizeof(Cell), width * height);
    gameState.computeGrid = calloc(sizeof(Cell), width * height);
}


void updateGameState() {
    for (int i = 0; i < gameState.width; i++) {
        for (int j = 0; j < gameState.height; j++) {
            int index = i * gameState.width + j;
            gameState.display[index] = gameState.frameGrid[index].is_alive ? RGBA(255,255,255,255) : RGBA(0,0,0,255);
        }
    }
    updateCells();
}

void updateCells() {
    memcpy(gameState.computeGrid, gameState.frameGrid, sizeof(Cell) * gameState.width * gameState.height);
    for (int i = 0; i < gameState.width; i++) {
        for (int j = 0; j < gameState.height; j++) {
            int index = i * gameState.width + j;
            int num_neighbors = 0;
            if (i > 0) {
                num_neighbors += gameState.frameGrid[(i - 1) * gameState.width + j].is_alive;
                if (j > 0) num_neighbors += gameState.frameGrid[(i - 1) * gameState.width + j - 1].is_alive;
                if (j < gameState.height - 1) num_neighbors += gameState.frameGrid[(i - 1) * gameState.width + j + 1].is_alive;
            }
            if (i < gameState.width - 1) {
                num_neighbors += gameState.frameGrid[(i + 1) * gameState.width + j].is_alive;
                if (j > 0) num_neighbors += gameState.frameGrid[(i + 1) * gameState.width + j - 1].is_alive;
                if (j < gameState.height - 1) num_neighbors += gameState.frameGrid[(i + 1) * gameState.width + j + 1].is_alive;
            }
            if (j > 0) num_neighbors += gameState.frameGrid[i * gameState.width + j - 1].is_alive;
            if (j < gameState.height - 1) num_neighbors += gameState.frameGrid[i * gameState.width + j + 1].is_alive;

            if (gameState.frameGrid[index].is_alive) {
                if (num_neighbors < 2 || num_neighbors > 3) gameState.computeGrid[index].is_alive = 0;
                else gameState.computeGrid[index].is_alive = 1;
            } else {
                if (num_neighbors == 3) gameState.computeGrid[index].is_alive = 1;
                else gameState.computeGrid[index].is_alive = 0;
            }
        }
    }
    memcpy(gameState.frameGrid, gameState.computeGrid, sizeof(Cell) * gameState.width * gameState.height);
}

//region Game Engine

/**
 * Ran once at the start of the program.
 */
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

    initGameState(50, 50);


    textureId = amgl_texture_create((amgl_texture_info){
            .data = (unsigned char*) gameState.display,
            .width = gameState.width,
            .height = gameState.height,
            .format = AMGL_TEXTURE_FORMAT_RGBA,
            .wrap_s = AMGL_TEXTURE_WRAP_REPEAT,
            .wrap_t = AMGL_TEXTURE_WRAP_REPEAT,
            .min_filter = AMGL_TEXTURE_FILTER_NEAREST,
            .mag_filter = AMGL_TEXTURE_FILTER_NEAREST,
            .name = "gameStateTexture"
    });

    uniformSamplerId = amgl_uniform_create((amgl_uniform_info){
            .name = "uSampler",
            .type = AMGL_UNIFORM_TYPE_SAMPLER2D
    });

    defaultRenderPassId = amgl_render_pass_create((amgl_render_pass_info){0});

}

/**
 * Ran every frame.
 */
void update() {
    amgl_set_viewport( 0,0, am_platform_window_get_size(1).x, am_platform_window_get_size(1).y);
    if (am_platform_key_pressed(AM_KEYCODE_ESCAPE)) am_engine_quit();
    if (am_platform_key_pressed(AM_KEYCODE_SPACE)) runGame = !runGame;
    if (runGame) {
        updateGameState();
        amgl_texture_update(textureId, AMGL_TEXTURE_UPDATE_SUBDATA, (amgl_texture_info) {
                                    .data = (unsigned char *) gameState.display,
                                    .width = gameState.width,
                                    .height = gameState.height,
                                    .format = AMGL_TEXTURE_FORMAT_RGBA
                            }
        );
    }
    if (am_platform_mouse_button_down(AM_MOUSE_BUTTON_LEFT)) {
        am_vec2 mousePos = am_platform_mouse_get_positionv();
        am_vec2u winSize = am_platform_window_get_size(1);
        /*
         * -winSize.x / 4 and -winSize.y / 4 are the offsets from the center of the window
         * (winSize.x / 2) / gameState.width is the width of a single cell
         * (winSize.y / 2) / gameState.height is the height of a single cell
         * We subtract the y coordinate from winSize.y because the y coordinate is inverted in OpenGL
         */
        int x =  (mousePos.x - winSize.x / 4) / ((winSize.x / 2) / gameState.width);
        int y =  (winSize.y - mousePos.y - winSize.y / 4) / ((winSize.y / 2) / gameState.height);
        printf("%d %d\n", x, y);
        if (x >= 0 && x < gameState.width * 1.5f && y >= 0 && y < gameState.height) {
            gameState.frameGrid[y * gameState.width + x].is_alive = 1;

            gameState.display[y * gameState.width + x] = RGBA(255,255,255,255);
            amgl_texture_update(textureId, AMGL_TEXTURE_UPDATE_SUBDATA, (amgl_texture_info) {
                                        .data = (unsigned char *) gameState.display,
                                        .width = gameState.width,
                                        .height = gameState.height,
                                        .format = AMGL_TEXTURE_FORMAT_RGBA
                                }
            );
        }

    }
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

/**
 * Ran once at the end of the program.
 */
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

//endregion