BITS 32

section .data
  ; the linker fills this in
  kern_size: dd 0 ; 32 bits (ought to do it)

; the stack is 16KiB
section .stack
align 4
stack_bottom:
  resb 16384
stack_top:

section .text
global _start
_start:
  ; boot1 creates the `bootinfo' structure, populates it, and pushes a pointer
  ; to it to the stack (the pointer that's to be passed to `kmain')
  ; we have to remember the pointer's value because we set up our own stack
  pop eax

  ; kernel's own new stack
  mov esp, stack_top

  ; we are now ready to actually execute C code
  ; but first, we have to adjust the `kern_size' field in the `bootinfo'
  ; structure
  push eax
  ; **ptr = kern_size
  ; that's why the `kern_size' field has to be the first one in the struct
  mov [eax], dword kern_size

  extern kmain
  call kmain

  ; in case the function returns
  cli
.halt:
  hlt
  jmp .halt

; vi: ft=nasm:ts=2:sw=2 expandtab

