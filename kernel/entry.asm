BITS 32

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
  ; we have to remember the pointer's value because the kernel sets up it's
  ; own stack
  pop eax

  ; yup, right here
  mov esp, stack_top

  ; we are now ready to actually execute C code
  ; (calling kmain(eax))
  push eax
  extern kmain
  call kmain

  ; in case the function returns
  cli
.halt:
  hlt
  jmp .halt

; vi: ft=nasm:ts=2:sw=2 expandtab

