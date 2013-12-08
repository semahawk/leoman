BITS 16
ORG 0

jmp 0x07C0:bootloader

bootloader:
  ; update the segment register
  mov ax, 0x07C0
  mov ds, ax

  ; set up the stack
  cli
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xFFFF
  sti

  ; save the device's number from which we've booted
  mov [bootdrv], dl

  mov si, welcome_msg
  mov ah, 0xE

  .print_char:
    lodsb
    cmp al, 0
    je .done
    int 10h
    jmp .print_char

  .done:
    %macro increment_si 1
      push cx
      mov cx, %1
      %%repeat:
        cmp cx, 0
        jle %%inc_done
        inc si
        dec cx
        jmp %%repeat
        %%inc_done:
          pop cx
    %endmacro

    %macro printhex 1
      mov si, hexadecimal
      increment_si %1
      mov ah, 0xE
      mov al, byte [si]
      int 10h
    %endmacro

    ; print the version
    mov ch, 0
    mov cl, [$$ + 509]
    mov ax, cx
    mov bx, 0x10
    ; divide the version by 16
    div bx
    ; now, ax = ax / 16 (quotient); dx = ax % 16 (remainder)
    printhex ax
    printhex dx

reset:
  ; reset the floppy drive
  mov ax, 0
  mov dl, [bootdrv]
  int 13h

  jc reset          ; error -> try again

read:
  mov ax, 0x0500
  mov es, ax
  mov bx, 0x0000    ; es:bx = 0500h:0000h

  ; load the second sector from the floppy
  mov ah, 0x02      ; the instruction
  mov al, 2         ; load two sectors
  mov ch, 0         ; cylinder no. 0
  mov cl, 2         ; sector no. 2
  mov dh, 0         ; head no. 0
  mov dl, [bootdrv] ; the drive from which we've booted
  int 13h           ; read!

  jc read           ; error -> try again

  ; save the number of sectors read
  mov [sectors_read], al

  ; jump to stage two
  jmp 0500h:0000h

; the string
welcome_msg db 'Nihilum bootloader 0x', 0
hexadecimal db '0123456789ABCDEF'
; some variables
sectors_read db 0
bootdrv db 0

; pad the remainder of the boot sector with n's
times 509-($-$$) db 0
; the bootloaders version
db 0x1
; the standard PC boot signature
dw 0xAA55

; vi: ft=nasm:ts=2:sw=2 expandtab

