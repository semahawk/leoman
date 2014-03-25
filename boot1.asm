BITS 16
ORG 0x7c00

jmp near boot1

%define INODE_SIZE 0x100
%define NXADDR     2     ; # of external blocks in inode
%define NDADDR     12    ; # of direct blocks in inode
%define NIADDR     3     ; # of indirect blocks in inode

%include "print.asm"
%include "utils.asm"

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

%ifdef DEBUG
  call puthex
  mov ah, 0eh
  mov al, ' '
  int 10h
%endif

  ; see if the a20 line is enabled, and enable it if it isn't
  call check_a20
  test ax, ax
  je enable_a20
  jmp get_drive_params

enable_a20:
  mov si, enable_a20_msg
  call putstr

  .1:
    ; try the BIOS
    mov ax, 0x2401
    int 0x15
    ; see if it worked
    call check_a20
    test ax, ax
    jne .end
    ; it didn't, carry on
  .2:
    ; try using the keyboard controller
    call enable_a20_via_kbd
    ; see if it worked
    call check_a20
    test ax, ax
    jne .end
    ; it didn't, carry on
  .3:
    ; try the Fast A20 Gate
    in al, 0x92
    test al, 2
    jne .end
    or al, 2
    and al, 0xFE
    out 0x92, al
    ; check if it worked
    call check_a20
    test ax, ax
    jne .end

    ; here, it seems it didn't work, which is a shame
    mov si, enable_a20_fail_msg
    call putstr
    jmp halt

  .end:

get_drive_params:
  ; fetch the drive geometry
  ;
  ; let's see if we're on a hardrive or a floppy
  ; if it's a hard drive, then we'll calculate/retrieve the values
  ; if it's a floppy, then we'll use the defaults
  cmp dl, 0x80
  jb .floppy

  ; here, it's a harddrive
%ifdef DEBUG
  mov si, hd_msg    ;
  call putstr       ; deleteme
%endif

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
%ifdef DEBUG
  mov si, floppy_msg
  call putstr
%endif

  mov byte [number_of_heads], 16
  mov byte [sectors_per_track], 63

.end:
  call putnl
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

read_sblk:
  ; the primary superblock is 8KiB (16 sectors) wide, and is at
  ; offset 0x10000 (128 sectors), which makes it an LBA 128

  ; fetch the CHS values
  mov ecx, 128
  call lba_to_chs

  ; load the super block into just below the bootloader
  mov ax, 0x05c0
  mov es, ax
  xor bx, bx          ; es:bx = 0x05c0:0x0000 (= 0x5c00)

  mov ah, 02h         ; the instruction
  mov al, 10h         ; load 16 sectors
  int 13h             ; load!

  ; error, let's try again
  jc read_sblk

  ; set up the segments
  mov ax, 0x0
  mov ds, ax
  mov si, 0x5c00
  push si

  ; make sure we actually have the superblock loaded, and that it's UFS2
  cmp dword [si + 1372], 0x19540119
  je welcome

  ; it didn't work out :c
  mov si, magic_not_found_msg
  call putstr
  jmp halt

welcome:
  mov si, welcome_msg
  call putstr

fetch_fs_variables:
  ; restore SI
  pop si

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

  ; d_bsize is not fetched, but calculated
  xor edx, edx
  mov eax, [fs_fsize]
  mov ebx, 1
  mov ecx, [fs_fsbtodb]
  shl ebx, cl
  div dword ebx
  ; d_bsize = eax = fs_fsize / fsbtodb(1)
  mov [d_bsize], eax

  ; print a smiley face
  mov bx, 0x0f01
  mov eax, 0x0b8000
  mov word [ds:eax], bx

  ; see what the location of ROOTINO (and two other inodes) is
  mov ecx, 0x2
  call inode_addr
  mov edx, eax
  call puthex
  call putnl
  mov ecx, 0xfb80
  call inode_addr
  mov edx, eax
  call puthex
  call putnl
  mov ecx, 0xfb81
  call inode_addr
  mov edx, eax
  call puthex
  call putnl
  call putnl

  ; load the / inode just into above boot1
  mov ax, 0x1fa0
  mov es, ax
  xor bx, bx        ; es:bx = 0x1fa0:0x0000 (= 0x1fa00)
  ; the inode number
  mov ecx, 0x2
  call load_inode

  ; save these two
  push ds
  push esi
  ; loop through it's direct blocks
  xor ecx, ecx      ; going 0->NDADDR
  mov esi, 0x70     ; direct blocks start at offset 0x70 in the inode
  mov ax, es        ;
  mov ds, ax        ; ds = es

  loop_dirblks:
    ; save the registers
    push ecx

    ; print block's address
    mov ecx, [esi]
    call blk_addr
    mov edx, ecx
    call puthex
    call putnl

    ; restore the registers
    pop ecx
    add ecx, 1
    ; increase the SI by the size of one direct block's number, ie. 64 bits
    add esi, 8
    ; check if ecx is less than the number of direct blocks
    cmp ecx, dword NDADDR
    ; if it is, here you go again
    jb loop_dirblks

  call putnl

  ; restore DS and ESI
  pop esi
  pop ds

nice_halt:
  mov si, goodbye_msg
  call putstr

halt:
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
lba_msg: db 'lba:', 0
ecx_msg: db 'ecx:', 0
c_msg: db 'c:', 0
h_msg: db 'h:', 0
s_msg: db 's:', 0
enable_a20_msg: db 'Enabling the a20 line', 0xd, 0xa, 0
enable_a20_fail_msg: db 'Failed to enable the a20 line!', 0xd, 0xa, 0
magic_not_found_msg: db 'fatal: UFS2 magic not found!', 0xd, 0xa, 0
welcome_msg: db 'Quidquid Latine dictum, sit altum videtur.', 0xd, 0xa, 0xd, 0xa, 0
goodbye_msg: db 'Sit vis vobiscum', 0xd, 0xa, 0

; make it be 127 sectors wide
times 512*127-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

