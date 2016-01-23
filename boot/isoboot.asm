org 0x7c00
bits 16

start:
  jmp 0x0000:boot
  ; pad with zeroes so the BIT starts at byte 8
  times 8-($-$$) db 0

  ; BIT = Boot Information Table
  BIT_PrimaryVolumeDescriptor  dd 0 ; LBA of the Primary Volume Descriptor
  BIT_BootFileLocation         dd 0 ; LBA of the Boot File
  BIT_BootFileLength           dd 0 ; length of the boot file in bytes
  BIT_Checksum                 dd 0 ; 32 bit checksum
  BIT_Reserved        times 40 db 0 ; reserved 'for future standardization'

boot:
  mov ah, 0xe
  mov al, 1
  int 10h

hang:
  jmp hang

; vi: ft=nasm:ts=2:sw=2 expandtab

