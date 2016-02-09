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
  pusha
  mov ax, 0xe00+%1
  int 10h
  popa
%endmacro

%macro error 0
  putchar '!'
  mov ebx, __LINE__
  mov ecx, __LINE__
  mov edx, __LINE__
  jmp $
%endmacro

%include "isofs.asm"
%include "a20.asm"
%include "elf.asm"

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

  ; a dwarf
  putchar 0x01

; 'enter' unreal mode
go_unreal:
  ; disable interrupts
  cli
  ; save the data segment
  push ds
  ; load the GDT
  lgdt [gdt]
  ; set the PE bit
  mov eax, cr0
  or  al, 1
  mov cr0, eax
  ; tell 386/486 not to crash
  jmp $+2
  ; select the code descriptor
  mov bx, 0x08
  mov ds, bx
  ; unset the PE bit, back to real mode
  and al, 0xfe
  mov cr0, eax
  ; restore the data segment
  pop ds
  ; enable interrupts
  sti

running_in_quote_unquote_unreal_mode:
  ; sigma
  putchar 0xe4

try_enabling_a20:
  call enable_a20_or_die
  ; ae
  putchar 0x91

load_next_stage:
  call initialize_isofs_utilities
  ; a half
  putchar 0xab

  mov si, next_stage
  call find_and_load_file
  ; and beyond!
  putchar 0xec

load_and_parse_elf:
  call dispatch_elf_sections
  ; delta
  putchar 0xeb

bye_real_mode:
  ; a lovely heart
  putchar 0x03

enter_protected_mode:
  cli
  lgdt [gdt]
  mov eax, cr0
  or al, 1
  mov cr0, eax
  jmp 0x08:protected_mode

bits 32
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
  mov edx, [0x10000 + 0x18]
  jmp edx

hang:
  cli
  hlt

; the device's number from which we've booted
bootdrv: db 0
; name/location of the kernel
next_stage: db "/BOOT/KERNEL/KERNEL.BIN", 0

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

