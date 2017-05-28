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
  push ax
  mov ax, 0xe00+%1
  int 10h
  pop ax
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
%include "memory.asm"

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

clear_the_screen:
  mov ah, 0
  mov al, 3
  int 10h

  ; a dwarf
  putchar 0x01

; 'enter' unreal mode
go_unreal:
  call enter_unreal_mode
  ; sigma
  putchar 0xe4

try_enabling_a20:
  call enable_a20_or_die
  ; ae
  putchar 0x91

load_the_kernel:
  call initialize_isofs_utilities
  ; a half
  putchar 0xab

  mov si, kernel_path
  call find_and_load_file

  mov esi, 0x10000
  mov edx, dword [esi+0x18]
  mov dword [kernel_entry], edx

  ; and beyond!
  putchar 0xec

load_and_parse_elf:
  call dispatch_elf_sections
  ; delta
  putchar 0xeb

  call detect_memory
  ; alpha
  putchar 0xe0

load_initrd:
  mov si, initrd_path
  call find_and_load_file
  ; phi!
  putchar 0xe8

initrd_loaded:
  ; set the `bootinfo' field
  mov dword [initrd_addr], 0x10000
  mov dword [initrd_size], eax

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

  ; the parameter for the kernel's C function
  mov eax, bootinfo
  push eax

  ; farewell!
  mov edx, dword [kernel_entry]
  jmp edx

hang:
  cli
  hlt

; the device's number from which we've booted
bootdrv: db 0
; name/location of the kernel
kernel_path: db "/BOOT/KERNEL/KERNEL.BIN", 0
; kernel's entry point
kernel_entry: dd 0xdeadbeef
; and of the initrd
initrd_path: db "/BOOT/KERNEL/INITRD.BIN", 0

;
; the `struct bootinfo' definition
; the field order and sizes must match with those in the declaration
; of `struct bootinfo' (found in kernel/common.h)
;
bootinfo:
; {{{
initrd_addr: dd 0xffffffff
initrd_size: dd 0xffffffff
mem_avail: dd 0x0
; meh..
memory_map: times 24 * 16 db 0 ; max 16 entries (is it enough?)
; }}}

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

; padding so the bootsector is exactly 2048 bytes long
times 2048-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

