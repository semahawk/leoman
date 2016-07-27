cmake_minimum_required (VERSION 2.6)
project (initrd)

set (INITRD_OBJS $<TARGET_FILE:angle> $<TARGET_FILE:shades>)

find_program (sar NAMES sar PATHS ${CMAKE_SOURCE_DIR} PATH_SUFFIXES tools)

add_custom_command (
  OUTPUT initrd.bin
  DEPENDS ${INITRD_OBJS}
  COMMAND $<TARGET_FILE:sar>
  ARGS c initrd.bin ${INITRD_OBJS}
)

add_custom_target (initrd ALL DEPENDS initrd.bin)

add_dependencies (initrd sar angle shades)

