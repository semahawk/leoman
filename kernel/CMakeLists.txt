cmake_minimum_required (VERSION 2.6)
project (kernel C ASM_NASM)

set (INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
include_directories ("${INCLUDE_DIR}")

# fix this crap
set (CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_C_COMPILER> -Wno-invalid-pp-token -D__ASSEMBLY__ -I${INCLUDE_DIR} -E - < <SOURCE> > <SOURCE>.cpped && <CMAKE_ASM_NASM_COMPILER> -f elf32 <SOURCE>.cpped -o <OBJECT>")
set (CMAKE_C_FLAGS "-target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib ${CFLAGS}")
set (LINKER_SCRIPT "kernel.ld")

file (GLOB SOURCES . *.c *.asm)
add_executable (kernel ${SOURCES})
set_target_properties (kernel PROPERTIES LINKER_LANGUAGE C PREFIX "" SUFFIX "" LINK_FLAGS "-T ${LINKER_SCRIPT}")

