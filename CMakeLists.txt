cmake_minimum_required(VERSION 3.6)                     # CMake version check
project(proxypop3)                                      # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                                # Enable c standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
add_compile_options("-lsctp")
#add_compile_options("-Wall")
#add_compile_options("-pedantic")
#add_compile_options("-Werror")
#add_compile_options("-Wextra")

link_libraries("-lsctp")

# Add .c and .h files of project root directory as source file
set(SOURCE_FILES_CLIENT configurationClient/administrator.c configurationClient/include/administrator.h configurationClient/input_parser.c configurationClient/include/input_parser.h configurationClient/main.c configurationClient/include/request.h configurationClient/include/response.h configurationClient/send_request.c configurationClient/include/send_request.h configurationClient/serializer.c configurationClient/include/serializer.h configurationClient/receive_response.c configurationClient/deserializer.c configurationClient/include/deserializer.h configurationClient/include/receive_response.h configurationClient/utils.c configurationClient/include/utils.h)
set(SOURCE_FILES_MIME mimeFilter/mimeFilter.c)
set(SOURCE_FILES_PROXY proxy/buffer.c proxy/external_transformation.c  proxy/main.c proxy/admin.c proxy/pop3.c proxy/response.c proxy/request.c
        proxy/stm.c proxy/utils.c proxy/transformation_test.c proxy/selector.c proxy/include/buffer.h proxy/include/external_transformation.h proxy/include/admin.h proxy/include/pop3.h proxy/include/request.h proxy/include/response.h proxy/include/selector.h
        proxy/include/stm.h proxy/include/utils.h proxy/queue.c proxy/request_parser.c proxy/response_parser.c proxy/include/request_parser.h proxy/include/response_parser.h proxy/pop3_session.c proxy/include/pop3_session.h proxy/include/queue.h proxy/input_parser.c proxy/include/input_parser.h proxy/deserializer.c proxy/serializer.c proxy/include/serializer.h proxy/include/deserializer.h proxy/include/response_admin.h proxy/include/request_admin.h proxy/admin_actions.c proxy/include/admin_actions.h proxy/admin_parser_request.c proxy/include/admin_parser.h proxy/admin_parse_response.c proxy/include/metrics.h)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(client ${SOURCE_FILES_CLIENT} configurationClient/main.c configurationClient/include/request.h configurationClient/include/response.h configurationClient/send_request.c configurationClient/include/send_request.h configurationClient/serializer.c configurationClient/receive_response.c configurationClient/deserializer.c configurationClient/include/receive_response.h configurationClient/utils.c configurationClient/include/utils.h)
add_executable(mime ${SOURCE_FILES_MIME})
add_executable(proxypop3 ${SOURCE_FILES_PROXY} proxy/include/admin_actions.h proxy/admin_parser_request.c proxy/include/admin_parser.h proxy/admin_parse_response.c proxy/include/metrics.h)