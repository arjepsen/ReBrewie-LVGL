# Tools/McuProbe/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

add_executable(mcu_probe McuProbe.c)
target_compile_options(mcu_probe PRIVATE -Wall -Wextra -Wpedantic)