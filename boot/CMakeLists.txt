cmake_minimum_required (VERSION 2.6)
project (boot C ASM_NASM)

set (CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> -f bin <SOURCE> -o <OBJECT>")

add_custom_command (
  OUTPUT isoboot.bin
  DEPENDS isoboot.asm
  COMMAND ${CMAKE_ASM_NASM_COMPILER}
  ARGS -f bin isoboot.asm -o isoboot.bin
)

add_custom_target (boot ALL DEPENDS isoboot.bin)

