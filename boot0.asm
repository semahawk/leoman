BITS 16
ORG 0

jmp 0x07c0:boot0

boot0:
  ; update the segment register
  mov ax, 0x07c0
  mov ds, ax

  ; set up the stack
  cli
  xor ax, ax
  mov ss, ax
  mov sp, 0x7c00   ; put the stack right below the bootsector
  sti

  ; save the device's number from which we've booted
  mov [bootdrv], dl

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

  ; read the partition table
  mov es, ax
  mov si, 0x01be  ; es:si = 0x07e0:0x01be (= 0x7fbe)
  mov cx, 4       ; four loops
  ; 'parse' the partition table, see which partition is active / bootable, and
  ; then 'boot' it
printpart:
  ; is it marked active / bootable?
  cmp byte [si], 0x80
  ; yup!
  je blastoff

  ; go to the next partition entry
  add si, 0x10
  ; continue
  loop printpart
  ; if we got here, it means that no partition is active
  jmp halt

blastoff:
  mov si, msg
  call putstr

; "This is the end of the road; this is the end of the line
;  This is the end of your life; this is the..." -- Endgame
halt:
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

msg: db 'blastoff', 0xd, 0xa, 0
; number of the drive we have booted from
bootdrv: db 0

; pad the remainder of the code section with zeros
times 446-($-$$) db 0

; the phony partition table
; Nihilum's entry (shamelessly copied from /usr/src/sys/boot/i386/boot2/boot1.S)
db 0x80, 0x00, 0x01, 0x00
db 0x7f, 0xfe, 0xff, 0xff
db 0x00, 0x00, 0x00, 0x00
db 0x50, 0xc3, 0x00, 0x00
; three blank partition entries (3 * 16)
%rep 48
  db 0x00
%endrep

; the standard PC boot signature
dw 0xaa55

; vi: ft=nasm:ts=2:sw=2 expandtab

