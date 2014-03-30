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
  mov sp, 0x7a00   ; put the stack 512 bytes below the bootsector
  sti

  ; save the device's number from which we've booted
  mov [bootdrv], dl

  ; relocate itself to 0x7a00
  cld                   ; go downwards
  mov si, boot0         ; source
  mov di, 0x7a00        ; destination
  mov cx, 0x0100        ; one whole sector (0x100 words - 0x200 bytes)
  rep movsw             ; do it!
  jmp 0x07a0:relocated  ; jump to the relocated bit

relocated:
  ; update the segment register
  mov ax, 0x07a0
  mov ds, ax

  ; read the partition table
  mov es, ax
  mov si, 0x01be  ; es:si = 0x07a0:0x01be (= 0x7bbe)
  mov cx, 4       ; four loops
  ; 'parse' the partition table, see which partition is active / bootable, and
  ; then 'boot' it
try:
  ; is it marked active / bootable?
  cmp byte [si], 0x80
  ; yup!
  je blastoff

  ; go to the next partition entry
  add si, 0x10
  ; continue
  loop try
  ; if we got here, it means that no partition is active
  jmp halt

blastoff:
  ; print 'N'
  mov ah, 0xe
  mov al, 0x4e
  int 10h

.reset:
  ; reset the drive from which we've booted from
  mov ah, 00h
  mov dl, [bootdrv]
  int 13h
  ; error, let's try again
  jc .reset

.read:
  ; set up the registers
  mov ax, 0x07c0
  mov es, ax
  mov bx, 0x0000       ; es:bx = 0x07c0:0x0000 (= 0x7c00)

  ; save the source register
  push si

  ; set the rest of the registers
  mov ah, 02h          ; the instruction
  mov al, 7fh          ; load 127 sectors (65024 bytes)
  mov ch, [si + 0x3]   ; bits 7-0 of cylinder
  mov dh, [si + 0x1]   ; head
  mov cl, [si + 0x2]   ; sector in bits 5-0; bits 7-6 are high bits of
                       ; cylinder
  mov dl, [bootdrv]    ; the drive we've booted from
  push dx              ; save dl for later
  int 13h              ; read!
  ; error, let's try again
  jc .read

  ; print 'M'
  mov ah, 0xe
  mov al, 0x4d
  int 10h
  ; print two newlines
  mov cx, 2
twonls:
  mov ah, 0xe
  mov al, 0xd
  int 10h
  mov ah, 0xe
  mov al, 0xa
  int 10h
  loop twonls

  ; restore dl
  pop dx
  ; set ds:si to 0x07c0:0x01.e (whatever the active partition was)
  mov ax, 0x07c0
  mov ds, ax
  pop si

  ; buenos aires!
  jmp 0x0000:0x7c00

; "This is the end of the road; this is the end of the line
;  This is the end of your life; this is the..." -- Endgame
halt:
  cli
  hlt

; number of the drive we have booted from
bootdrv: db 0

; pad the remainder of the code section with zeros
times 446-($-$$) db 0

; the phony partition table
; three blank partition entries (3 * 16)
%rep 48
  db 0x00
%endrep
; Nihilum's entry (shamelessly copied from /usr/src/sys/boot/i386/boot2/boot1.S)
db 0x80, 0x00, 0x02, 0x00
db 0x7f, 0xfe, 0xff, 0xff
db 0x00, 0x00, 0x00, 0x00
db 0x50, 0xc3, 0x00, 0x00

; the standard PC boot signature
dw 0xaa55

; vi: ft=nasm:ts=2:sw=2 expandtab

