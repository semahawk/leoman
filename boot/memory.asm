enter_unreal_mode:
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
  ret

detect_memory:
  xor eax, eax
  mov es, eax
  mov edi, memory_map       ; point es:edi to the memory map buffer

  mov eax, 0xe820
  xor ebx, ebx              ; ebx must be 0
  mov ecx, 24               ; ask for 24 bytes
  mov edx, 0x534d4150
  int 15h

  jc .fail

  mov edx, 0x534d4150
  cmp eax, edx
  jne .fail

  ; ebx == 0 implies list is only 1 entry long (so, worthless)
  test ebx, ebx
  je .fail

  jmp .skip_interrupt

.next_entry:
  mov eax, 0xe820
  mov ecx, 24               ; ask for 24 bytes
  mov [es:di + 20], dword 1 ; force a valid ACPI 3.X entry
  int 15h
  jc .ok                    ; carry set means "end of list already reached"
  mov edx, 0x534d4150       ; repair potentially trashed register

.skip_interrupt:
  jcxz .skip_entry          ; skip any 0 length entries
  cmp cl, 20                ; got a 24 byte ACPI 3.X response?
  jbe .not_extended
  test byte [es:di + 20], 1 ; if so: is the "ignore this data" bit clear?
  je .skip_entry

.not_extended:
  mov ecx, [es:di + 8]      ; get lower dword of memory region length
  or  ecx, [es:di + 12]     ; "or" it with upper dword to test for zero
  jz  .skip_entry           ; if length qword is 0; skip entry
  add edi, 24               ; point to the next entry in the buffer

.skip_entry:
  test ebx, ebx             ; if ebx is 0, list is complete
  jne .next_entry

.ok:
  clc ; I'm not sure if that's necessary
  ret

.fail:
  error

; vi: ft=nasm:ts=2:sw=2 expandtab

