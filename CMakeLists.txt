cmake_minimum_required (VERSION 2.6)
project (leoman)

set (DISK_IMAGE "leoman.iso")

add_subdirectory (lib)
add_subdirectory (boot)
add_subdirectory (drivers)
add_subdirectory (initrd)
add_subdirectory (kernel)
add_subdirectory (servers)
add_subdirectory (tools)

find_program (qemu qemu-system-i386)
find_program (mkisofs mkisofs)

add_custom_command (
  OUTPUT ${DISK_IMAGE}
  DEPENDS boot kernel initrd
  COMMAND mkdir
  ARGS -p iso_root/boot/{kernel,loader}/
  COMMAND cp
  ARGS kernel/kernel iso_root/boot/kernel/kernel.bin
  COMMAND cp
  ARGS initrd/initrd.bin iso_root/boot/kernel/
  COMMAND cp
  ARGS boot/isoboot.bin iso_root/boot/loader/
  COMMAND ${mkisofs}
  ARGS -quiet -R -J -l -c boot/boot.cat
       -b boot/loader/isoboot.bin -no-emul-boot
       -boot-load-size 4 -o ${DISK_IMAGE} iso_root
)

add_custom_target (run
  DEPENDS ${DISK_IMAGE}
  COMMAND ${qemu} -cdrom ${DISK_IMAGE} -monitor stdio
)

add_custom_target (leoman ALL DEPENDS ${DISK_IMAGE})

