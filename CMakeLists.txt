cmake_minimum_required(VERSION 3.6)                     # CMake version check
project(proxypop3)                                      # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                                # Enable c standard

# Add .c and .h files of project root directory as source file
set(SOURCE_FILES_CLIENT administrator/administrator.c administrator/include/administrator.h administrator/input_parser.c administrator/include/input_parser.h proxy/include/administrator.h administrator/main.c)
set(SOURCE_FILES_MIME mimeFilter/mimeFilter.c)
set(SOURCE_FILES_PROXY proxy/main.c proxy/include/main.h proxy/include/input_parser.h proxy/input_parser.c proxy/include/proxypop3.h proxy/include/handler.h proxy/handler.c proxy/proxypop3.c proxy/clients.c proxy/include/clients.h proxy/administrator.c)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(client ${SOURCE_FILES_CLIENT} administrator/main.c)
add_executable(mime ${SOURCE_FILES_MIME})
add_executable(proxypop3 ${SOURCE_FILES_PROXY} proxy/clients.c proxy/include/clients.h proxy/administrator.c)