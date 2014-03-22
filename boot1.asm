BITS 16
ORG 0x7c00

jmp boot1

%include "print.asm"

boot1:
  ; update the segment register
  xor ax, ax
  mov ds, ax

  ; set up the stack
  cli
  xor ax, ax
  mov ss, ax
  mov sp, 0x5c00
  sti

; 'enter' unreal mode
go_unreal:
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
  mov ax, 0x05c0
  mov es, ax
  xor bx, bx          ; es:bx = 0x05c0:0x0000 (= 0x5c00)

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

  ; set up the segments
  mov ax, 0x0
  mov ds, ax
  mov si, 0x5c00

  ; fetch the superblock essentials
  ; ...with a handy macro!
%macro fetch 2
  mov eax, dword [si + %2]
  mov [fs_%1], eax
%endmacro

  fetch sblkno,  8
  fetch cblkno,  12
  fetch iblkno,  16
  fetch dblkno,  20
  fetch ncg,     44
  fetch bsize,   48
  fetch fsize,   52
  fetch frag,    56
  fetch fsbtodb, 100
  fetch cgsize,  160
  fetch ipg,     184
  fetch fpg,     188
  fetch size,    1080

; fetch is no more
%undef fetch

; fsbtodb(reg, val):
;   reg = val << fs_fsbtodb
;
; !! reg must be different than ECX!
;
%macro fsbtodb 2
  ; save
  push ecx
  ; shift you!
  mov %1, %2
  mov ecx, dword [fs_fsbtodb]
  shl %1, cl
  ; restore
  pop ecx
%endmacro

  ; d_bsize is not fetched, but calculated
  xor edx, edx
  mov eax, [fs_fsize]
  fsbtodb ebx, 1
  div dword ebx
  ; d_bsize = eax = fs_fsize / fsbtodb(1)
  mov [d_bsize], eax

  ; print a smiley face
  mov bx, 0x0f01
  mov eax, 0x0b8000
  mov word [ds:eax], bx

; traverse the cylinder groups in search for the kernel
; initialize the counter (going from 0 to fs_ncg)
mov ecx, 0

loop_through_cgs:
  jmp varsend
  ; space for some variables
  cgbase: dd 0
  cgimin: dd 0
  fsb: dd 0
  phys: dd 0

  varsend:
  ; save the counter
  push ecx

  ; calculate the physical address of the given CG
  ;
  ; cgbase(N) = fs_fpg * N
  ; cgimin(N) = cgbase(N) + fs_cblkno
  ; phys = fsbtodb(cgimin(ECX)) * d_bsize
  mov eax, [fs_fpg]     ; eax = fs_fpg
  mul ecx               ; edx:eax = eax * ecx
  mov ecx, eax
;jmp $
  mov [cgbase], ecx     ; cgbase = ecx
  add ecx, [fs_cblkno]  ; ecx += fs_cblkno
;jmp $
  mov [cgimin], ecx     ; cgimin = ecx
  fsbtodb eax, ecx      ; eax = fsbtodb(ecx)
;jmp $
  mov [fsb], eax        ; fsb = eax
  mul dword [d_bsize]   ; edx:eax = eax * d_bsize
;jmp $
  mov [phys], eax       ; THE RESULT = eax

  mov si, cg_msg
  call putstr
  pop ecx
  mov al, cl
  push ecx
  call putdigit
  mov ah, 0xe
  mov al, ':'
  int 10h
  mov al, ' '
  int 10h
  mov edx, [phys]
  call puthex
  call putnl

  ; restore the counter
  pop ecx
  ; counter++
  inc ecx
  ; see if the counter is less than fs_ncg
  cmp ecx, [fs_ncg]
  ; if it is then go back to the beginning of the loop
  jb loop_through_cgs

halt:
  mov si, goodbye_msg
  call putstr

  cli
  hlt

;
; The Global Descriptor Table
;
gdt_data:
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

; Superblock variables
;
; offset of superblock in filesystem
fs_sblkno: dd 0
; offset of cylinder block
fs_cblkno: dd 0
; offset of inode blocks
fs_iblkno: dd 0
; offset of first data after CG
fs_dblkno: dd 0
; # of cylinder groups
fs_ncg: dd 0
; size of basic blocks in fs
fs_bsize: dd 0
; size of fragment blocks in fs
fs_fsize: dd 0
; number of framents in a block
fs_frag: dd 0
; fstbtodb and dbtofsb shift constant
fs_fsbtodb: dd 0
; cylinder group size
fs_cgsize: dd 0
; inodes per group
fs_ipg: dd 0
; blocks per group * fs_frag
fs_fpg: dd 0
; number of blocks in fs
fs_size: dq 0
; number of data blocks in fs
fs_dsize: dq 0
; device bsize
d_bsize: dd 0

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

; the messages
floppy_msg: db 'Floppy.', 0xd, 0xa, 0
hd_msg: db 'Hard drive.', 0xd, 0xa, 0
cg_msg: db 'CG #', 0
welcome_msg: db 'Quidquid Latine dictum, sit altum videtur.', 0xd, 0xa, 0
goodbye_msg: db 'Sit vis vobiscum', 0xd, 0xa, 0

; make it be 127 sectors wide
times 512*127-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

