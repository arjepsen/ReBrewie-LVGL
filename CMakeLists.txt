cmake_minimum_required(VERSION 3.18)

project(brewie_ui_rebuild C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(brewie_ui
    src/main.c
)

target_include_directories(brewie_ui PRIVATE
    include
    core
    ui
    platform
)
