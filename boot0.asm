BITS 16
ORG 0

jmp 0x07c0:boot0

boot0:
  ; update the segment register
  mov ax, 0x07c0
  mov ds, ax

  ; set up the stack
  cli
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xffff
  sti

  ; save the device's number from which we've booted
  mov [bootdrv], dl

  ; print the message
  mov si, msg
  call putstr

  ; relocate to 0x7e00
  cld                   ; go downwards
  mov si, boot0         ; source
  mov di, 0x7e00        ; destination
  mov cx, 0x0100        ; one whole sector (0x100 words - 0x200 bytes)
  rep movsw             ; do it!
  jmp 0x07e0:relocated  ; jump to the relocated bit

relocated:
  ; update the segment register
  mov ax, 0x07e0
  mov ds, ax

  ; print the message (again)
  mov si, msg
  call putstr

hang:
  cli
  hlt

; prints a string located in SI
; the registers are preserved
putstr:
  pusha
  mov ah, 0xe
  .putchar:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .putchar
  .done:
    popa
    ret

msg db 'hello, 16-bit world', 0xd, 0xa, 0
; number of the drive we have booted from
bootdrv db 0

; pad the remainder of the code section with zeros
times 446-($-$$) db 0

; the phony partition table
; three blank partition entries (3 * 16)
%rep 48
  db 0x00
%endrep
; the Nihilum's entry (shamelessly copied from /usr/src/sys/boot/i386/boot0.S)
db 0x80, 0x00, 0x01, 0x00
db 0x7f, 0xfe, 0xff, 0xff
db 0x00, 0x00, 0x00, 0x00
db 0x50, 0xc3, 0x00, 0x00

; the standard PC boot signature
dw 0xaa55

; vi: ft=nasm:ts=2:sw=2 expandtab

