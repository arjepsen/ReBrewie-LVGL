cmake_minimum_required(VERSION 3.18)

project(brewie_ui_rebuild C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Select LVGL config before LVGL is added
if (CMAKE_CROSSCOMPILING)
    configure_file(
        ${CMAKE_SOURCE_DIR}/lv_conf_target.h
        ${CMAKE_SOURCE_DIR}/lv_conf.h
        COPYONLY
    )
endif()

# LVGL library (submodule) - builds for both host + target
add_subdirectory(external/lvgl)

# Tool for SOM<->MCU com testing - builds for both host + target
add_subdirectory(Tools/McuProbe)

if (CMAKE_CROSSCOMPILING)
    add_executable(brewie_ui
        src/main.c
        UI/UI.c
    )
else()
    find_package(SDL2 REQUIRED)

    add_executable(brewie_ui
        src/main_sim.c
        UI/UI.c
    )

    target_link_libraries(brewie_ui PRIVATE SDL2::SDL2)
endif()

target_include_directories(brewie_ui PRIVATE
    include
    core
    ui
    platform
)

target_link_libraries(brewie_ui PRIVATE
    lvgl
)