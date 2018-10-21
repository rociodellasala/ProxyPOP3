cmake_minimum_required(VERSION 3.6)                     # CMake version check
project(proxypop3)                                      # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                                # Enable c standard

# Add .c and .h files of project root directory as source file
set(SOURCE_FILES_CLIENT configurationClient/administrator.c configurationClient/include/administrator.h configurationClient/input_parser.c configurationClient/include/input_parser.h proxy/include/administrator.h configurationClient/main.c configurationClient/include/request.h configurationClient/include/response.h configurationClient/send_request.c configurationClient/include/send_request.h configurationClient/serializer.c configurationClient/include/serializer.h configurationClient/receive_response.c configurationClient/deserializer.c configurationClient/include/deserializer.h configurationClient/include/receive_response.h configurationClient/utils.c configurationClient/include/utils.h)
set(SOURCE_FILES_MIME mimeFilter/mimeFilter.c)
set(SOURCE_FILES_PROXY proxy/main.c proxy/include/main.h proxy/include/input_parser.h proxy/input_parser.c proxy/include/proxypop3.h proxy/include/handler.h proxy/handler.c proxy/proxypop3.c proxy/clients.c proxy/include/clients.h proxy/administrator.c proxy/include/request.h proxy/include/response.h proxy/serializer.c proxy/deserializer.c proxy/include/serializer.h proxy/include/deserializer.h proxy/admin_responses.c proxy/admin_actions.c proxy/media_types_utils.c proxy/include/media_types_utils.h)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(client ${SOURCE_FILES_CLIENT} configurationClient/main.c configurationClient/include/request.h configurationClient/include/response.h configurationClient/send_request.c configurationClient/include/send_request.h configurationClient/serializer.c configurationClient/receive_response.c configurationClient/deserializer.c configurationClient/include/receive_response.h configurationClient/utils.c configurationClient/include/utils.h)
add_executable(mime ${SOURCE_FILES_MIME})
add_executable(proxypop3 ${SOURCE_FILES_PROXY} proxy/clients.c proxy/include/clients.h proxy/administrator.c proxy/include/request.h proxy/include/response.h proxy/serializer.c proxy/deserializer.c proxy/include/serializer.h proxy/include/deserializer.h proxy/admin_responses.c proxy/admin_actions.c proxy/media_types_utils.c proxy/include/media_types_utils.h)