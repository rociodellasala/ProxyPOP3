cmake_minimum_required(VERSION 3.6)             # CMake version check
project(proxypop3)                              # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                        # Enable c standard

set(SOURCE_FILES main.c)                        # Add main.c file of project root directory as source file
add_executable(proxypop3 ${SOURCE_FILES})  # Add executable target with source files listed in SOURCE_FILES variable