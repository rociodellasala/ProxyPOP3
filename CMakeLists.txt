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
set(SOURCE_FILES_CLIENT pop3ctl/administrator.c pop3ctl/include/administrator.h pop3ctl/input_parser.c pop3ctl/include/input_parser.h pop3ctl/main.c pop3ctl/include/client_request.h pop3ctl/include/response.h pop3ctl/send_request.c pop3ctl/include/send_request.h pop3ctl/serializer.c pop3ctl/include/serializer.h pop3ctl/receive_response.c pop3ctl/deserializer.c pop3ctl/include/admin_deserializer.h pop3ctl/include/receive_response.h pop3ctl/utils.c pop3ctl/include/utils.h)
set(SOURCE_FILES_MIME mimeFilter/mimeFilter.c)
set(SOURCE_FILES_POP3FILTER pop3filterbuffer.c pop3filtermain.c pop3filteradmin.c pop3filterpop3nio.c
        pop3filterstm.c pop3filterutils.c pop3filterselector.c pop3filterinclude/buffer.h pop3filterinclude/admin.h pop3filterinclude/pop3nio.h pop3filterinclude/selector.h
        pop3filterinclude/utils.h pop3filterpop3_multi.c pop3filterinclude/pop3_multi.h pop3filterparser.c pop3filterinclude/parser.h pop3filtermsg_queue.c pop3filterinclude/msg_queue.h pop3filterinput_parser.c pop3filterinclude/input_parser.h pop3filteradmin_deserializer.c pop3filteradmin_serializer.c pop3filterinclude/admin_serializer.h pop3filterinclude/deserializer.h pop3filterinclude/admin_response.h pop3filterinclude/admin_request.h pop3filteradmin_actions.c pop3filterinclude/admin_actions.h pop3filteradmin_parser_request.c pop3filterinclude/admin_parser.h pop3filteradmin_parse_response.c pop3filterinclude/metrics.h pop3filtermetrics.c pop3filterinclude/stm.h pop3filterclient_parser_request.c pop3filterinclude/client_parser_request.h pop3filterinclude/client_parser_response.h pop3filterinclude/client_request.h pop3filterinclude/client_response.h pop3filterclient_request.c pop3filterpop3_session.c pop3filterinclude/pop3_session.h pop3filterclient_response.c pop3filterinclude/client_response.h pop3filterclient_parser_response.c pop3filterpipelining.c pop3filterinclude/pipelining.h pop3filterclient_request_utils.c pop3filterinclude/client_request_utils.h)

# ATENCION: CUANDO TENGAMOS EJECUTABLES EN CONFIGURATIONCLIENT Y MIMEFILTER HAY QUE SETEAR ESTE ARCHIVO (no se si lo que esta, está bien)

# Add executable target with source files listed in SOURCE_FILES_PROXY variable
add_executable(client ${SOURCE_FILES_CLIENT} pop3ctl/main.c pop3ctl/include/client_request.h pop3ctl/include/response.h pop3ctl/send_request.c pop3ctl/include/send_request.h pop3ctl/serializer.c pop3ctl/receive_response.c pop3ctl/deserializer.c pop3ctl/include/receive_response.h pop3ctl/utils.c pop3ctl/include/utils.h)
add_executable(mime ${SOURCE_FILES_MIME})
add_executable(proxypop3 ${SOURCE_FILES_PROXY} pop3filterinclude/admin_actions.h pop3filteradmin_parser_request.c pop3filterinclude/admin_parser.h pop3filteradmin_parse_response.c pop3filterinclude/metrics.h pop3filtermetrics.c pop3filterinclude/stm.h pop3filterclient_parser_request.c pop3filterinclude/client_parser_request.h pop3filterinclude/client_parser_response.h pop3filterinclude/client_request.h pop3filterinclude/client_response.h pop3filterclient_request.c pop3filterpop3_session.c pop3filterinclude/pop3_session.h pop3filterclient_response.c pop3filterinclude/client_response.h pop3filterclient_parser_response.c pop3filterpipelining.c pop3filterinclude/pipelining.h pop3filterclient_request_utils.c pop3filterinclude/client_request_utils.h)