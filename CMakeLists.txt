cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
project(AG_PKG_2 VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory("nana")

add_executable(AG_PKG_2 main.cpp)

find_package (Threads)
target_link_libraries (AG_PKG_2 ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(AG_PKG_2 nana)