cmake_minimum_required(VERSION 3.6)                     # CMake version check
project(proxypop3)                                      # Create project "proxypop3"
set(CMAKE_C_STANDARD 11)                                # Enable c standard
set(CMAKE_C_FLAGS    " -pthread -lsctp --std=c99 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L")
link_libraries(-lsctp)

set(CLANG_TIDY_CHECKS *
        -boost-*
        -mpi-*
        -llvm-*
        -cppcoreguidelines-*
        -clang-analyzer-osx.*
        -cert-*-cpp
        -misc-unused-parameters,
        -clang-diagnostic-missing-declarations)
string(REPLACE ";" "," CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS}")

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    find_program(CLANGTIDY NAMES clang-tidy)
    if(CLANGTIDY)
        message(STATUS "Found clang-tidy: ${CLANGTIDY}")
        set(CMAKE_C_CLANG_TIDY clang-tidy
                -checks=${CLANG_TIDY_CHECKS}
                -warnings-as-errors=*
                -fix)
    endif()
endif()
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} -pedantic -pedantic-errors -Wall -Wextra
-Werror -Wno-unused-parameter -D_POSIX_C_SOURCE=200112L")


# Add .c and .h files of project root directory as source file
set(SOURCE_FILES_CLIENT configurationClient/administrator.c configurationClient/include/administrator.h configurationClient/input_parser.c configurationClient/include/input_parser.h configurationClient/main.c configurationClient/include/client_request.h configurationClient/include/response.h configurationClient/send_request.c configurationClient/include/send_request.h configurationClient/serializer.c configurationClient/include/serializer.h configurationClient/receive_response.c configurationClient/deserializer.c configurationClient/include/admin_deserializer.h configurationClient/include/receive_response.h configurationClient/utils.c configurationClient/include/utils.h)
set(SOURCE_FILES_MIME mimeFilter/mimeFilter.c)
set(SOURCE_FILES_PROXY proxy/buffer.c proxy/main.c proxy/admin.c proxy/pop3nio.c
        proxy/stm.c proxy/utils.c proxy/selector.c proxy/include/buffer.h proxy/include/admin.h proxy/include/pop3nio.h proxy/include/selector.h
        proxy/include/utils.h proxy/pop3_multi.c proxy/include/pop3_multi.h proxy/parser.c proxy/include/parser.h proxy/msg_queue.c proxy/include/msg_queue.h proxy/input_parser.c proxy/include/input_parser.h proxy/admin_deserializer.c proxy/admin_serializer.c proxy/include/admin_serializer.h proxy/include/deserializer.h proxy/include/admin_response.h proxy/include/admin_request.h proxy/admin_actions.c proxy/include/admin_actions.h proxy/admin_parser_request.c proxy/include/admin_parser.h proxy/admin_parse_response.c proxy/include/metrics.h proxy/metrics.c proxy/include/stm.h proxy/client_parser_request.c proxy/include/client_parser_request.h proxy/include/client_parser_response.h proxy/include/client_request.h proxy/include/client_response.h proxy/client_request.c proxy/pop3_session.c proxy/include/pop3_session.h proxy/client_response.c proxy/include/client_response.h proxy/client_parser_response.c proxy/pipelining.c proxy/include/pipelining.h proxy/client_request_utils.c proxy/include/client_request_utils.h)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(client ${SOURCE_FILES_CLIENT} configurationClient/main.c configurationClient/include/client_request.h configurationClient/include/response.h configurationClient/send_request.c configurationClient/include/send_request.h configurationClient/serializer.c configurationClient/receive_response.c configurationClient/deserializer.c configurationClient/include/receive_response.h configurationClient/utils.c configurationClient/include/utils.h)
add_executable(mime ${SOURCE_FILES_MIME})
add_executable(proxypop3 ${SOURCE_FILES_PROXY} proxy/include/admin_actions.h proxy/admin_parser_request.c proxy/include/admin_parser.h proxy/admin_parse_response.c proxy/include/metrics.h proxy/metrics.c proxy/include/stm.h proxy/client_parser_request.c proxy/include/client_parser_request.h proxy/include/client_parser_response.h proxy/include/client_request.h proxy/include/client_response.h proxy/client_request.c proxy/pop3_session.c proxy/include/pop3_session.h proxy/client_response.c proxy/include/client_response.h proxy/client_parser_response.c proxy/pipelining.c proxy/include/pipelining.h proxy/client_request_utils.c proxy/include/client_request_utils.h)