BITS 16
ORG 0

jmp 0x07c0:boot1

boot1:
  ; update the segment register
  mov ax, 0x07c0
  mov ds, ax

  ; set up the stack
  cli
  xor ax, ax
  mov ss, ax
  mov sp, 0x7c00   ; put the stack right below the bootsector
  sti

  ; save the drive number from which we've booted
  mov [bootdrv], dl
call puthex
mov ah, 0eh
mov al, ' '
int 10h

get_drive_params:
  ; fetch the drive geometry
  ;
  ; let's see if we're on a hardrive or a floppy
  ; if it's a hard drive, then we'll calculate/retrieve the values
  ; if it's a floppy, then we'll use the defaults
  cmp dl, 0x80
  jb .floppy

  ; here, it's a harddrive
  mov si, hd_msg    ;
  call putstr       ; deleteme

  xor dx, dx
  xor cx, cx  ; zero out dx and cx
  ; call uncle BIOS
  mov ah, 8
  mov dl, [bootdrv]
  int 13h
  ; now dh contains the number of heads - 1, so let's retrieve it
  inc dh
  mov byte [number_of_heads], dh
  ; cl holds the sector number, but, only the first 6 bits contain the actual number
  and cx, 0x003f
  mov byte [sectors_per_track], cl
  jmp .end

.floppy:
  ; here, it's a floppy
  mov si, floppy_msg
  call putstr

  mov byte [number_of_heads], 16
  mov byte [sectors_per_track], 63

.end:
;
; Load the superblock
;
reset_sblk:
  ; reset the boot drive
  xor ah, ah
  mov dl, [bootdrv]
  int 13h
  ; error, let's try again
  jc reset_sblk

calculate_chs:
  ; the primary superblock is 8KiB (16 sectors) wide, and is at
  ; offset 0x10000 (128 sectors), which makes it an LBA 128
  ; so yup, let's translate it into CHS

  ; temp     = LBA / sectors per track
  ; sector   = LBA % sectors per track + 1
  ; head     = temp % number of heads
  ; cylinder = temp / number of heads

  mov ax, 128         ; LBA / sectors per track
  div byte [sectors_per_track]
  xor dx, dx          ; dx will be the temp 'variable'
  mov dl, al          ; al = dl = LBA / sectors per track
                      ; ah = LBA % sectors per track
  inc ah              ; ah++
  mov [sector], ah    ; sector = ah

  mov ax, dx          ; temp / number of heads
  div byte [number_of_heads]
  mov [head], ah      ; head = ah = temp % number of heads
  mov [cylinder], al  ; cylinder = al = temp / number of heads

  call putnl
  xor dx, dx
  mov dl, [number_of_heads]
  call puthex
  call putnl

  mov dl, [sectors_per_track]
  call puthex
  call putnl
  call putnl

  ; print the CHS values
  xor dx, dx
  mov dl, [cylinder]
  call puthex
  call putnl

  mov dl, [head]
  call puthex
  call putnl

  mov dl, [sector]
  call puthex
  call putnl
  call putnl

read_sblk:
  ; all right, calculations are done, now let's roll!
  ; load the super block into just above the bootloader
  mov ax, 0x07e0
  mov es, ax
  xor bx, bx          ; es:bx = 0x07e0:0x0000 (= 0x7e00)

  mov ah, 02h         ; the instruction
  mov al, 10h         ; load 16 sectors
  mov ch, [cylinder]  ; the calculated cylinder
  mov dh, [head]      ; the calculated head
  mov cl, [sector]    ; the calculated sector
  mov dl, [bootdrv]   ; the drive we've booted from
  int 13h             ; load!

  ; error, let's try again
  jc read_sblk

welcome:
  mov si, welcome_msg
  call putstr
  call putnl

  ; DEBUG: see if the sblk was really loaded
  mov ax, 0x0
  mov ds, ax
  mov si, 0x7e00

  xor dx, dx
  mov dl, [si + 4]
  call puthex
  call putnl
  mov dl, [si + 8]
  call puthex
  call putnl
  mov dl, [si + 12]
  call puthex
  call putnl
  mov dl, [si + 16]
  call puthex
  call putnl

halt:
  cli
  hlt

; prints a newline
putnl:
; {{{
  pusha
  mov ah, 0xe
  mov al, 0xd
  int 10h
  mov al, 0xa
  int 10h
  popa
  ret
; }}}

; prints characters from SI until a nul is found
putstr:
; {{{
  pusha
  mov ah, 0xe
  .putchar:
    lodsb
    cmp al, 0
    je .done
    int 10h
    jmp .putchar
  .done:
    popa
    ret
; }}}

; print the value in AL as a digit (0-9 as decimal, 10-15 as
; lowercase hexadecimal)
putdigit:
; {{{
  pusha
  cmp al, 10
  jl .ten_less
  .ten_more:
    add al, 87
    jmp .print
  .ten_less:
    add al, 48
  .print:
    mov ah, 0xE
    int 10h
    popa
    ret
; }}}

; print the value in DX as a hexadecimal word
puthex:
; {{{
  pusha
  mov bx, 0
  mov cx, 0
  mov ah, 0xE
  mov al, '0'
  int 10h
  mov al, 'x'
  int 10h          ; print the leading '0x'

  mov cx, dx
  and cx, 0xF000   ; fetch the first nibble
  shr cx, 12       ; shift twelve bits right
  mov al, cl
  call putdigit    ; print it
  mov cx, dx
  and cx, 0x0F00   ; fetch the second nibble
  shr cx, 8        ; shift eight bits right
  mov al, cl
  call putdigit    ; print it
  mov cx, dx
  and cx, 0x00F0   ; fetch the third nibble
  shr cx, 4        ; shift four bits right
  mov al, cl
  call putdigit    ; print it
  mov cx, dx
  and cx, 0x000F   ; fetch the fourth nibble
                   ; no need to shift
  mov al, cl
  call putdigit    ; print it
  popa
  ret
; }}}

; the messages
welcome_msg: db 'sblk tinydump:', 0xd, 0xa, 0
floppy_msg: db 'Floppy.', 0xd, 0xa, 0
hd_msg: db 'Hard drive.', 0xd, 0xa, 0

; number of heads
number_of_heads: db 0
; sectors per track
sectors_per_track: db 0
; when loading the kernel, this is the head value
head: db 0
; this is the cylinder number
cylinder: db 0
; and the sector
sector: db 0
; number of the drive we have booted from
bootdrv: db 0

; make it be 127 sectors wide
times 512*127-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

