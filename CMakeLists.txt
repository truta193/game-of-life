cmake_minimum_required(VERSION 3.26)
project(Sandbox C)

set(CMAKE_C_STANDARD 17)

add_executable(Sandbox main.c external/glad/src/glad.c ametrine.h
        elements.h)

find_package(OpenGL REQUIRED)

include_directories(external/glad/include ${OPENGL_INCLUDE_DIRS} ${X11_INCLUDE_DIR} external/stb_image m)

target_link_libraries(Sandbox ${OPENGL_LIBRARIES} ${X11_LIBRARIES} m gdi32 winmm)