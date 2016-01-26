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

%macro putchar 1
  mov ax, 0xe00+%1
  int 10h
%endmacro

%macro error 0
  putchar '!'
  jmp $
%endmacro

%include "isofs.asm"

boot:
  ; update the segment register
  xor ax, ax
  mov ds, ax

  cli
  mov ss, ax
  ; put the stack just below the bootsector
  mov sp, 0x7bff
  sti

  ; remember the device's number
  mov [bootdrv], dl

  putchar 0x01

load_next_stage:
  call initialize_isofs_utilities

  putchar 0xab

  mov si, next_stage
  call find_and_load_file

next_stage_is_loaded:
  putchar 0x03

  ; print a newline to signify that the first stage has finished
  putchar 0xd
  putchar 0xa

  ; bye :)
  jmp 0x0000:0x8000

error:
  putchar '!'
hang:
  cli
  hlt

; the device's number from which we've booted
bootdrv: db 0
; name/location of the bootloader's next stage
next_stage: db "/BOOT/LOADER/MAIN.BIN", 0

; the DAP used when issuing int 13h, ah=42h
disk_address_packet:
  .size: db 0x10
  .zero: db 0x00
  .sector_num: dw 0x0001
  .membuf: dd 0x00000000
  .sector: dq 0x10

; padding so the bootsector is exactly 2048 bytes long
times 2048-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

