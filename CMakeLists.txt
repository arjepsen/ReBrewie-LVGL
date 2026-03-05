cmake_minimum_required(VERSION 3.18)

project(brewie_ui_rebuild C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Make sure LVGL (the submodule) sees your config while it compiles
#add_compile_definitions(LV_CONF_PATH="${CMAKE_SOURCE_DIR}/include/lv_conf.h")

# SDL2 for the simulator build
find_package(SDL2 REQUIRED)

# LVGL library (submodule)
add_subdirectory(external/lvgl)

add_executable(brewie_ui
    src/main.c
)

target_include_directories(brewie_ui PRIVATE
    include
    core
    ui
    platform
)

# Debian Bullseye usually provides SDL2::SDL2; if it doesn’t, we’ll adjust
target_link_libraries(brewie_ui PRIVATE
    lvgl
    SDL2::SDL2
)