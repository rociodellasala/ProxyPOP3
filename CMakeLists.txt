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
set(SOURCE_FILES_CLIENT pop3ctl/admin.c pop3ctl/include/admin.h pop3ctl/input_parser.c pop3ctl/include/input_parser.h pop3ctl/main.c pop3ctl/include/client_request.h pop3ctl/include/response.h pop3ctl/send_request.c pop3ctl/include/send_request.h pop3ctl/serializer.c pop3ctl/include/serializer.h pop3ctl/receive_response.c pop3ctl/deserializer.c pop3ctl/include/deserializer.h pop3ctl/include/receive_response.h pop3ctl/utils.c pop3ctl/include/utils.h)
set(SOURCE_FILES_POP3FILTER pop3filter/filter_list.c pop3filter/include/filter_list.h pop3filter/buffer.c pop3filter/main.c pop3filter/admin.c pop3filter/pop3nio.c
        pop3filter/stm.c pop3filter/utils.c pop3filter/selector.c pop3filter/include/buffer.h pop3filter/include/admin.h pop3filter/include/pop3nio.h pop3filter/include/selector.h
        pop3filter/include/utils.h pop3filter/pop3_multi.c pop3filter/include/pop3_multi.h pop3filter/parser.c pop3filter/include/parser.h pop3filter/msg_queue.c pop3filter/include/msg_queue.h pop3filter/input_parser.c pop3filter/include/input_parser.h pop3filter/admin_deserializer.c pop3filter/admin_serializer.c pop3filter/include/admin_serializer.h pop3filter/include/admin_deserializer.h pop3filter/include/admin_response.h pop3filter/include/admin_request.h pop3filter/admin_actions.c pop3filter/include/admin_actions.h pop3filter/admin_parser_request.c pop3filter/include/admin_parser.h pop3filter/admin_parse_response.c pop3filter/include/metrics.h pop3filter/metrics.c pop3filter/include/stm.h pop3filter/client_parser_request.c pop3filter/include/client_parser_request.h pop3filter/include/client_parser_response.h pop3filter/include/client_request.h pop3filter/include/client_response.h pop3filter/client_request.c pop3filter/pop3_session.c pop3filter/include/pop3_session.h pop3filter/client_response.c pop3filter/include/client_response.h pop3filter/client_parser_response.c pop3filter/pipelining.c pop3filter/include/pipelining.h pop3filter/client_request_utils.c pop3filter/include/client_request_utils.h pop3filter/pop3_handler.c pop3filter/include/pop3_handler.h pop3filter/e_transformation.c pop3filter/include/e_transformation.h)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(client ${SOURCE_FILES_CLIENT} pop3ctl/main.c pop3ctl/include/client_request.h pop3ctl/include/response.h pop3ctl/send_request.c pop3ctl/include/send_request.h pop3ctl/serializer.c pop3ctl/receive_response.c pop3ctl/deserializer.c pop3ctl/include/receive_response.h pop3ctl/utils.c pop3ctl/include/utils.h)

add_executable(proxypop3 ${SOURCE_FILES_POP3FILTER} pop3filter/include/admin_actions.h pop3filter/admin_parser_request.c pop3filter/include/admin_parser.h pop3filter/admin_parse_response.c pop3filter/include/metrics.h pop3filter/metrics.c pop3filter/include/stm.h pop3filter/client_parser_request.c pop3filter/include/client_parser_request.h pop3filter/include/client_parser_response.h pop3filter/include/client_request.h pop3filter/include/client_response.h pop3filter/client_request.c pop3filter/pop3_session.c pop3filter/include/pop3_session.h pop3filter/client_response.c pop3filter/include/client_response.h pop3filter/client_parser_response.c pop3filter/pipelining.c pop3filter/include/pipelining.h pop3filter/client_request_utils.c pop3filter/include/client_request_utils.h pop3filter/pop3_handler.c pop3filter/include/pop3_handler.h pop3filter/e_transformation.c pop3filter/include/e_transformation.h)
