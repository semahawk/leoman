global switch_to_userspace

extern tss_set_esp

; void switch_to_userspace(void *kstack, void *ustack);
switch_to_userspace:
  ; we don't want to be interrupted here
  cli
  ; fetch the parameters and store them in callee-saved registers
  mov ebx, [esp + 4] ; kernel stack
  mov edi, [esp + 8] ; user stack

  ; that's pretty wanky
  ; set the esp0 field in the TSS (kernel's stack)
  push ebx ; the param for the `tss_set_esp`
  call tss_set_esp

  ; 0x23 is the user data segment (0x20) with two lowest bits set (indicating
  ; that we want ring 3)

  mov ax, 0x23
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; again, user data segment (0x20) with two lowest bits set
  push dword 0x23
  ; the stack (passed as a parameter)
  push dword edi  ; the user's stack
  ; the flags we want to use in user mode
  pushfd
  ; set the IF flag (ie. enable interrupts)
  or dword [esp], 0x200
  ; the code segment we want to use in user mode (0x18 with two lowest bits set)
  push dword 0x1b
  ; the instruction we want to execute in user mode
  push dword usermode
  ; let's go :)
hlt
  iretd

usermode:
  mov eax, 0xcafebabe
  mov ebx, 0x0badbeef
  mov ecx, 0xdeadc0de
  mov edx, 0xfacefeed

  ; let's see if the stack works here
  push edx
  push ecx
  push ebx
  push eax

  jmp $

; vi: ft=nasm:ts=2:sw=2 expandtab

