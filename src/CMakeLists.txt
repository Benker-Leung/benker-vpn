cmake_minimum_required(VERSION 2.7)

# set project name
project(benker-vpn C)

# set the variable
set(LIB lib/packet_handler.c
        lib/protocol_headers.c
        lib/session_struct.c
        lib/udp.c
        lib/vpn_raw_sock.c)

# similar to gcc *.c
add_executable(client   client/main.c
                        ${LIB})
add_executable(server   server/main.c
                        ${LIB})

# include the directory "lib", gcc -I, such that do not need the do something like #include "../lib/code.h"
target_include_directories(client PRIVATE lib)
target_include_directories(server PRIVATE lib)


# usage:  (run the command in directory of this file)
# mkdir -p build 
# cd build
# cmake ..
# cmake --build .
