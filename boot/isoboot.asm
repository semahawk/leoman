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

clear_the_screen:
  mov ah, 0
  mov al, 3
  int 10h

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

enter_protected_mode:
  cli
  lgdt [gdt]
  mov eax, cr0
  or al, 1
  mov cr0, eax
  jmp 0x08:protected_mode

BITS 32
protected_mode:
  mov eax, 0x10
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax
  mov ss, eax
  ; is that necessary?
  mov esp, 0x5c00

  ; farewell!
  jmp 0x8000

hang:
  cli
  hlt

; the device's number from which we've booted
bootdrv: db 0
; name/location of the bootloader's next stage
next_stage: db "/BOOT/LOADER/MAIN.BIN", 0

;
; The Global Descriptor Table
;
gdt_data:
; {{{
; the null selector
  dq 0x0           ; nothing!

; the code selector: base = 0x0, limit = 0xfffff
  dw 0xffff        ; limit low (0-15)
  dw 0x0           ; base low (0-15)
  db 0x0           ; base middle (16-23)
  db 10011010b     ; access byte
  db 11001111b     ; flags + limit (16-19)
  db 0x0           ; base high (24-31)

; the data selector: base = 0x0, limit = 0xfffff
  dw 0xffff        ; limit low (0-15)
  dw 0x0           ; base low (0-15)
  db 0x0           ; base middle (16-23)
  db 10010010b     ; access byte
  db 11001111b     ; flags + limit (16-19)
  db 0x0           ; base high (24-31)
; THE actual descriptor
gdt_end:
gdt:
  dw gdt_end - gdt_data - 1 ; sizeof gdt
  dd gdt_data
; }}}

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

