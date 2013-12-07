BITS 16
ORG 0

jmp 0x07C0:bootloader

; {{{ the 'print' macro
%macro print 1
  mov si, word %1
  mov ah, 0xE

  %%print_char:
    lodsb
    cmp al, 0
    je %%done
    int 10h
    jmp %%print_char

  %%done:
%endmacro
; }}}
; {{{ the 'print_newline' macro
%macro print_newline 0
  mov ah, 0xE
  mov al, 0xD
  int 10h
  mov al, 0xA
  int 10h
%endmacro
; }}}

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

  print welcome_msg
  print_newline
  print_newline
  print reset_msg

reset:
  ; reset the floppy drive
  mov ax, 0
  mov dl, [bootdrv]
  int 13h

  jc reset          ; error -> try again

  print ok
  print read_msg

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

  print ok
  print sectors_read_msg

  mov ah, 0xE
  mov al, [sectors_read]
  add al, 48        ; convert it to ASCII number (should work unless we load
                    ; more than 10 (or actually 16 ;) sectors)
  int 10h           ; print the number of sectors read

  print_newline
  print stage_two_msg

  ; jump to stage two
  jmp 0500h:0000h

; strings
welcome_msg      db 'Nihilum bootloader 0x01', 0
reset_msg        db '* resetting the floppy disk...', 0
read_msg         db '* loading stage two... ', 0
sectors_read_msg db '*    sectors read: ', 0
stage_two_msg    db '* hopping on to stage two!... ', 0
ok               db ' ok!', 0dh, 0ah, 0
; some variables
sectors_read db 0
bootdrv db 0

db 'BTSEC_END'
; pad the remainder of the boot sector with n's
times 510-($-$$) db 0
; the standard PC boot signature
dw 0xAA55

  print ok
  print_newline

; vi: ft=nasm:ts=2:sw=2 expandtab

