cmake_minimum_required(VERSION 2.8)
project(file2tcp)

find_package (Threads)
set(src file2tcp.c f2t_network.c)
add_executable(file2tcp ${src})
target_link_libraries(file2tcp ${CMAKE_THREAD_LIBS_INIT})
