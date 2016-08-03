cmake_minimum_required (VERSION 2.6)
project (libipc)

# TODO are those correct here?
set (CMAKE_C_FLAGS "-target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib ${CFLAGS}")

# compile every .c file to a library
file (GLOB SOURCES *.c)

add_library (ipc STATIC ${SOURCES})

target_include_directories (ipc PUBLIC ${CMAKE_SOURCE_DIR}/include)

