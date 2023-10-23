#include <iostream>
#include "ametrine.h"

void init() {}

void update() {}

void am_shutdown() {}

int main() {
    am_engine_create((am_engine_info) {
            .init = init,
            .update = update,
            .shutdown = am_shutdown,
            .is_running = true,
            .vsync_enabled = false,
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
