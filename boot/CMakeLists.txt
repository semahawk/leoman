cmake_minimum_required (VERSION 2.6)
project (boot C ASM_NASM)

set (CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> -f bin <SOURCE> -o <OBJECT>")

add_custom_command (
  OUTPUT isoboot.bin
  DEPENDS isoboot.asm
  COMMAND ${CMAKE_ASM_NASM_COMPILER}
  ARGS -I${CMAKE_CURRENT_SOURCE_DIR}/ -f bin ${CMAKE_CURRENT_SOURCE_DIR}/isoboot.asm -o ${CMAKE_CURRENT_BINARY_DIR}/isoboot.bin
)

add_custom_target (boot ALL DEPENDS isoboot.bin)

