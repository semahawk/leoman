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

  ; hello :)
  mov ah, 0xe
  mov al, 1
  int 10h

; 'enter' unreal mode
go_unreal:
; {{{
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
; }}}

running_in_unreal_mode:
  mov ah, 0xe
  mov al, 171
  int 10h

  ; see if the a20 line is enabled, and enable it if it isn't
  call check_a20
  test ax, ax
  je enable_a20
  jmp a20_enabled

; credit: http://wiki.osdev.org/A20_Line
;
; return: 0 in AX if the a20 line is disabled (memory wraps around)
;         1 in AX if the a20 line is enabled (memory does not wrap around)
check_a20:
; {{{
  pushf
  push ds
  push es
  push di
  push si

  cli

  xor ax, ax ; ax = 0
  mov es, ax

  not ax ; ax = 0xFFFF
  mov ds, ax

  mov di, 0x0500
  mov si, 0x0510

  mov al, byte [es:di]
  push ax

  mov al, byte [ds:si]
  push ax

  mov byte [es:di], 0x00
  mov byte [ds:si], 0xFF

  cmp byte [es:di], 0xFF

  pop ax
  mov byte [ds:si], al

  pop ax
  mov byte [es:di], al

  mov ax, 0
  je .exit

  mov ax, 1

  .exit:
    pop si
    pop di
    pop es
    pop ds
    popf

    ret
; }}}

; credit: http://wiki.osdev.org/A20_Line
;
; _attempts_ to enable the a20 line using the keyboard controller
enable_a20_via_kbd:
; {{{
  cli

  call .wait
  mov al,0xAD
  out 0x64,al

  call .wait
  mov al,0xD0
  out 0x64,al

  call .wait2
  in al,0x60
  push eax

  call .wait
  mov al,0xD1
  out 0x64,al

  call .wait
  pop eax
  or al,2
  out 0x60,al

  call .wait
  mov al,0xAE
  out 0x64,al

  call .wait
  sti
  ret

  .wait:
    in al,0x64
    test al,2
    jnz .wait
    ret

  .wait2:
    in al,0x64
    test al,1
    jz .wait2
    ret
; }}}

enable_a20:
; {{{
  .1:
    ; try the BIOS
    mov ax, 0x2401
    int 0x15
    ; see if it worked
    call check_a20
    test ax, ax
    jne .done
    ; it didn't, carry on
  .2:
    ; try using the keyboard controller
    call enable_a20_via_kbd
    ; see if it worked
    call check_a20
    test ax, ax
    jne .done
    ; it didn't, carry on
  .3:
    ; try the Fast A20 Gate
    in al, 0x92
    test al, 2
    jne .done
    or al, 2
    and al, 0xFE
    out 0x92, al
    ; check if it worked
    call check_a20
    test ax, ax
    jne .done

    ; here, it seems it didn't work, which is a shame
    jmp hang
  .done:
; }}}

a20_enabled:
  mov ah, 0xe
  mov al, 145
  int 10h

detect_memory:
; {{{
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
    jmp memory_detected

  .fail:
    ; sadness
    jmp hang
; }}}

memory_detected:
  mov ah, 0xe
  mov al, 251
  int 10h

so_far_so_good:
  mov ah, 0xe
  mov al, 3
  int 10h

hang:
  cli
  hlt
  jmp hang

; the device's number from which we've booted
bootdrv: db 0
;
; the `struct bootinfo' definition
; the field order and sizes must match with those in the declaration
; of `struct bootinfo' (found in kernel/common.h)
;
bootinfo:
initrd_addr: dd 0xffffffff
initrd_size: dd 0xffffffff
mem_avail:   dd 0x0
memory_map: times 24 * 64 db 0 ; max 64 entries (is it enough?)

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

; pad with zeroes so the bootsector is exactly 2048 bytes long
times 2048-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

