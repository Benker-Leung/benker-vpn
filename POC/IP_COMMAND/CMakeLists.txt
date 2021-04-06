cmake_minimum_required(VERSION 2.7)

# set project name
project(benker-vpn-poc C)


# similar to gcc *.c
add_executable(trial main.c)

target_link_libraries(trial netlink)

# usage:  (run the command in directory of this file)
# mkdir -p build 
# cd build
# cmake ..
# cmake --build .
