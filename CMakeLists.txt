cmake_minimum_required(VERSION 3.6)                     # CMake version check
project(proxypop3)                                      # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                                # Enable c standard

# Add .c and .h files of project root directory as source file
set(SOURCE_FILES_CLIENT configurationClient/configurationClient.c)
set(SOURCE_FILES_MIME mimeFilter/mimeFilter.c)
set(SOURCE_FILES_PROXY proxy/proxypop3.c proxy/include/proxypop3.h proxy/include/optionsParser.h proxy/optionsParser.c proxy/selector.c proxy/include/selector.h)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(proxypop3 ${SOURCE_FILES_PROXY})
add_executable(mime ${SOURCE_FILES_CLIENT})
add_executable(client ${SOURCE_FILES_MIME})