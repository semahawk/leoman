BITS 32

; declare constants used for creating a multiboot header
MBALIGN  equ 1 << 0
MEMINFO  equ 1 << 1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

; the stack is 16KiB
section .bootstrap_stack
align 4
stack_bottom:
  times 16384 db 0
stack_top:

section .text
global _start
_start:
  ; to set up a stack, we simply set the ESP register to point to the top of our
  ; stack (as it grows downwards)
jmp $
  mov esp, stack_top

  ; we are now ready to actually execute C code
  extern kmain
  call kmain

  ; in case the function returns
  cli
.halt:
  hlt
  jmp .halt

; vi: ft=nasm:ts=2:sw=2 expandtab

