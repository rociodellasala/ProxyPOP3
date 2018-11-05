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
set(SOURCE_FILES_MIME mime_filter/delimiter.c mime_filter/mime_chars.c mime_filter/mime_filter.c mime_filter/mime_list.c mime_filter/mime_msg.c mime_filter/mime_type.c mime_filter/multi_pop3_parser.c mime_filter/parser_creator.c mime_filter/parser_utils.c mime_filter/stack.c mime_filter/include/delimiter.h mime_filter/include/mime_chars.h mime_filter/include/mime_filter.h mime_filter/include/mime_list.h mime_filter/include/mime_msg.h mime_filter/include/mime_type.h mime_filter/include/multi_pop3_parser.h mime_filter/include/parser_creator.h mime_filter/include/parser_utils.h mime_filter/include/stack.h)
set(SOURCE_FILES_CLIENT admin_client/admin.c admin_client/include/admin.h admin_client/input_parser.c admin_client/include/input_parser.h admin_client/main.c admin_client/include/client_request.h admin_client/include/response.h admin_client/send_request.c admin_client/include/send_request.h admin_client/serializer.c admin_client/include/serializer.h admin_client/receive_response.c admin_client/deserializer.c admin_client/include/deserializer.h admin_client/include/receive_response.h admin_client/utils.c admin_client/include/utils.h)
set(SOURCE_FILES_POP3FILTER proxy_pop3/filter_list.c proxy_pop3/include/filter_list.h proxy_pop3/buffer.c proxy_pop3/main.c proxy_pop3/admin.c proxy_pop3/pop3nio.c
        proxy_pop3/stm.c proxy_pop3/utils.c proxy_pop3/selector.c proxy_pop3/include/buffer.h proxy_pop3/include/admin.h proxy_pop3/include/pop3nio.h proxy_pop3/include/selector.h
        proxy_pop3/include/utils.h proxy_pop3/pop3_multi.c proxy_pop3/include/pop3_multi.h proxy_pop3/parser.c proxy_pop3/include/parser.h proxy_pop3/msg_queue.c proxy_pop3/include/msg_queue.h proxy_pop3/input_parser.c proxy_pop3/include/input_parser.h proxy_pop3/admin_deserializer.c proxy_pop3/admin_serializer.c proxy_pop3/include/admin_serializer.h proxy_pop3/include/admin_deserializer.h proxy_pop3/include/admin_response.h proxy_pop3/include/admin_request.h proxy_pop3/admin_actions.c proxy_pop3/include/admin_actions.h proxy_pop3/admin_parser_request.c proxy_pop3/include/admin_parser.h proxy_pop3/admin_parse_response.c proxy_pop3/include/metrics.h proxy_pop3/metrics.c proxy_pop3/include/stm.h proxy_pop3/client_parser_request.c proxy_pop3/include/client_parser_request.h proxy_pop3/include/client_parser_response.h proxy_pop3/include/client_request.h proxy_pop3/include/client_response.h proxy_pop3/client_request.c proxy_pop3/pop3_session.c proxy_pop3/include/pop3_session.h proxy_pop3/client_response.c proxy_pop3/include/client_response.h proxy_pop3/client_parser_response.c proxy_pop3/pipelining.c proxy_pop3/include/pipelining.h proxy_pop3/client_request_utils.c proxy_pop3/include/client_request_utils.h proxy_pop3/pop3_handler.c proxy_pop3/include/pop3_handler.h proxy_pop3/e_transformation.c proxy_pop3/include/e_transformation.h)


# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(stripmime ${SOURCE_FILES_MIME})

add_executable(pop3ctl ${SOURCE_FILES_CLIENT} admin_client/main.c admin_client/include/client_request.h admin_client/include/response.h admin_client/send_request.c admin_client/include/send_request.h admin_client/serializer.c admin_client/receive_response.c admin_client/deserializer.c admin_client/include/receive_response.h admin_client/utils.c admin_client/include/utils.h)

add_executable(pop3filter ${SOURCE_FILES_POP3FILTER} proxy_pop3/include/admin_actions.h proxy_pop3/admin_parser_request.c proxy_pop3/include/admin_parser.h proxy_pop3/admin_parse_response.c proxy_pop3/include/metrics.h proxy_pop3/metrics.c proxy_pop3/include/stm.h proxy_pop3/client_parser_request.c proxy_pop3/include/client_parser_request.h proxy_pop3/include/client_parser_response.h proxy_pop3/include/client_request.h proxy_pop3/include/client_response.h proxy_pop3/client_request.c proxy_pop3/pop3_session.c proxy_pop3/include/pop3_session.h proxy_pop3/client_response.c proxy_pop3/include/client_response.h proxy_pop3/client_parser_response.c proxy_pop3/pipelining.c proxy_pop3/include/pipelining.h proxy_pop3/client_request_utils.c proxy_pop3/include/client_request_utils.h proxy_pop3/pop3_handler.c proxy_pop3/include/pop3_handler.h proxy_pop3/e_transformation.c proxy_pop3/include/e_transformation.h)
