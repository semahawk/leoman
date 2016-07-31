cmake_minimum_required (VERSION 2.6)
project (drivers)

include_directories ("${CMAKE_SOURCE_DIR}/include")

set (CMAKE_C_FLAGS "-target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib ${CFLAGS}")
set (LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/process.ld")
set (LIBS $<TARGET_FILE:c> $<TARGET_FILE:ipc>)

# compile every .c file to an executable
file (GLOB SOURCES *.c)
foreach (source ${SOURCES})
  string (REPLACE ".c" "" path_to_executable ${source})
  get_filename_component (executable ${path_to_executable} NAME)
  add_executable (${executable} ${source})
  set_target_properties (${executable} PROPERTIES LINKER_LANGUAGE C PREFIX "" SUFFIX "" LINK_FLAGS "-T ${LINKER_SCRIPT}")
  add_dependencies (${executable} c ipc)
  target_link_libraries (${executable} ${LIBS})
endforeach (source ${SOURCES})

