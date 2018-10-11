cmake_minimum_required(VERSION 3.6)                     # CMake version check
project(proxypop3)                                      # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                                # Enable c standard

# Add .c and .h files of project root directory as source file
set(SOURCE_FILES_CLIENT )
set(SOURCE_FILES_MIME )
set(SOURCE_FILES_PROXY proxy/main.c proxy/include/main.h proxy/include/optionsParser.h proxy/optionsParser.c)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(proxypop3 ${SOURCE_FILES_PROXY})