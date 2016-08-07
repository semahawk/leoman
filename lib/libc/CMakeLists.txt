cmake_minimum_required (VERSION 2.6)
project (libc)

# TODO are those correct here?
set (CMAKE_C_FLAGS "-target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib ${CFLAGS}")

# compile every .c file to a library
file (GLOB SOURCES *.c)

add_library (c STATIC ${SOURCES})

target_include_directories (c PUBLIC ${CMAKE_SOURCE_DIR}/include)

