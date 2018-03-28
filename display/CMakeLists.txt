cmake_minimum_required(VERSION 3.5.1)
project(demo)

set(src_path		${CMAKE_SOURCE_DIR}/src)
set(pub_include_path 	${CMAKE_SOURCE_DIR}/include)

file(GLOB includes		include/*.h)
file(GLOB sources		src/*.c)

add_executable(demo ${sources} ${includes})

target_compile_options(demo PUBLIC -I${CMAKE_SOURCE_DIR}/include -I/usr/include/SDL2 -std=c11 -Wall -g -L/usr/lib -L/usr/lib/x86_64-linux-gnu)
target_link_libraries(demo SDL2 pthread.so libm.so)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
