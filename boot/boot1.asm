BITS 16
ORG 0x7c00

jmp near boot1

; the kernel's path
kernel_name: db '/boot/kernel', 0
kernel_name_ptr: dd 0
; the kernel's file size
kernel_fsize: dd 0
; where the kernel is located before ELF relocation (meh) to 1MiB
kernel_preloc: dd 0
; kernel file's block size
kernel_blksz: dd 0

%define MAX_FNAME_SIZE 255

%define INODE_SIZE 0x100
%define NXADDR     2     ; # of external blocks in inode
%define NDADDR     12    ; # of direct blocks in inode
%define NIADDR     3     ; # of indirect blocks in inode

; ELF related
%define PT_LOAD    1

; watch out! little endian
%define UFS2_MAGIC 0x19540119
%define ELF_MAGIC  0x464c457f

%include "print.asm"
%include "ufs.asm"
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
; {{{
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
; }}}
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
  cmp dword [si + 1372], UFS2_MAGIC
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

  ; see what the location of ROOTINO (and two other inodes) is
%ifdef DEBUG
; {{{
  mov ecx, 0x2
  call inode_addr
  mov edx, eax
  call puthex
call putnl
mov ecx, 0x3
call inode_addr
mov edx, eax
call puthex
call putnl
  call putnl
; }}}
%endif

  ; load the / inode just into above boot1
  mov ax, 0x17a0
  mov es, ax
  xor bx, bx        ; es:bx = 0x17a0:0x0000 (= 0x17a00)
  ; the inode number
  mov ecx, 0x2
  call load_inode

  mov esi, kernel_name
  mov dword [kernel_name_ptr], esi

  ; while (*esi != '\0')
  cmp byte [esi], 0x0
  inc esi
  inc dword [kernel_name_ptr]
  je next_path_segment_end

  jmp next_path_segment
  ; the name buffer where each path's segment will be kept
  name_buffer: times MAX_FNAME_SIZE + 1 db 0
  ; whether the current segment is last in the path
  last_segment: db 0

next_path_segment:
  push ecx
  ; {{{

  ; zero-out the name_buffer
  mov eax, name_buffer
  mov ebx, 0x0
  mov ecx, MAX_FNAME_SIZE
  call memset

  ; reset last_segment
  mov byte [last_segment], 0x0
  ; zero out ecx (it is used as an index when writing to the name_buffer)
  xor ecx, ecx

  fetch_the_segment:
    push eax
    push edx
    ; {{{
    ; *esi != '\0' && *esi != '/'
    cmp byte [esi], 0x0
    je .end
    cmp byte [esi], '/'
    je .end

    ; *(name_buffer + ecx++) = *esi
    mov al, byte [esi]
    mov [name_buffer + ecx], al
    inc ecx
    ; }}}
    pop edx
    pop eax
    inc esi
    inc dword [kernel_name_ptr]
    jmp fetch_the_segment
  .end:

  ; see if the segment is the last in the path (ie. if it is not followed by a
  ; slash)
  see_if_last_segment:
    cmp byte [esi], 0x0
    jne .end
    mov byte [last_segment], 1
  .end:

%ifdef DEBUG
; {{{
  call putnl
  push esi
  mov esi, searching_for_msg
  call putstr
  mov esi, name_buffer
  call putstr
  call putnl
  pop esi
; }}}
%endif

  ; this! we're gonna be traversing the block which contains names of
  ; files/directories/etc. which are contained in the directory which's
  ; inode is currently loaded at 0x17a00
  ;
  ; ECX will hold the counter (which goes from 0 to NDADDR)
  xor ecx, ecx
  ; EBX will hold current block's address
  mov ebx, 0x17a70
  traverse_direct_blocks:
    push ecx
    push ebx
    push edx
    push esi
    ; {{{

    ; don't bother even trying to traverse a block which's address is zero
    cmp dword [ebx], 0
    je traverse_direct_blocks_next

    ; load the current block into the memory into just above the inode
    load_the_current_block:
      ; {{{
      push ecx
      push ebx

      ; XXX i'm not sure that this block size is right
      mov eax, [fs_bsize]
      mov ecx, [ebx]
      mov bx, 0x17c0
      mov es, bx
      xor bx, bx     ; es:bx = 0x17c0:0x0000 (= 0x17c00)

      call load_blk

      pop ebx
      pop ecx
      ; }}}

%ifdef DEBUG
; {{{
    push esi
    mov esi, traversing_block_msg
    call putstr
    pop esi
    push edx
    mov edx, [ebx]
    call puthex
    call putnl
    pop edx
; }}}
%endif

    mov edi, 0x17c00
    traverse_one_block:
      push edx
      push ecx
      push ebx
      push eax
      push esi
      ; {{{
      push edi
      add edi, 8
      mov esi, name_buffer

      ; compare ESI and EDI
      .found?:
        mov ah, byte [esi]
        mov al, byte [edi]
        inc edi
        inc esi
        cmp ah, al
        jne .nope
        cmp al, 0
        je .yup
        jmp .found?

      .nope:
        jmp .end
      .yup:

%ifdef DEBUG
; {{{
        mov esi, found_inode_msg
        call putstr
        mov esi, name_buffer
        call putstr
        call putnl
; }}}
%endif

        ; see what kind of a thing that was what we found
        pop edi
        cmp byte [edi + 6], 0x8
        je .file
        cmp byte [edi + 6], 0x4
        je .directory
        ; none of the above, strange
        mov esi, isunknown_msg
        call putstr
        jmp halt

        .file:
          ; code for a file
%ifdef DEBUG
; {{{
          mov esi, isafile_msg
          call putstr
; }}}
%endif
          ; see if it was the last segment
          cmp byte [last_segment], 0
          je .1
            ; load it's inode in place of the previous one
            mov ecx, dword [edi]
            mov ax, 0x17a0
            mov es, ax
            xor bx, bx     ; es:bx = 0x17a0:0x0000 (= 0x17a00)
%ifdef DEBUG
; {{{
            mov esi, loading_inode_msg1
            call putstr
            mov edx, ecx
            call puthex
            mov esi, loading_inode_msg2
            call putstr
; }}}
%endif
            call load_inode
            jc halt

            ; save the file's size
            ; 0x10 is the offset of the inode's `size' field
            mov ebx, 0x17a10 ; 0x17a00 + 0x10
            mov eax, [ebx]
            mov [kernel_fsize], eax

            ; calculate the LBN (logical blocks number)
            ;call lblkno

            ;mov ebx, eax
            ;mov ecx, [kernel_fsize]
            ;call blksize

            ; (for now) assume the kernel's block size is the default (could it
            ; be different? what's the deal with fragments?)
            mov eax, [fs_bsize]
            mov [kernel_blksz], eax

            jmp kernel_found
          .1:
            ; 'crash' if it wasn't
            mov esi, error_loading_kernel_msg
            jmp halt
        .directory:
          ; code for a directory
%ifdef DEBUG
; {{{
          mov esi, isadir_msg
          call putstr
; }}}
%endif
          ; see if it was the last segment
          cmp byte [last_segment], 1
          je .2
            ; load the inode in the place of the actual one
            mov ecx, dword [edi]
            mov ax, 0x17a0
            mov es, ax
            xor bx, bx     ; es:bx = 0x17a0:0x0000 (= 0x17a00)
%ifdef DEBUG
; {{{
            mov esi, loading_inode_msg1
            call putstr
            mov edx, ecx
            call puthex
            mov esi, loading_inode_msg2
            call putstr
; }}}
%endif
            call load_inode
            jc halt
            mov esi, dword [kernel_name_ptr]
            jmp next_path_segment_next
          .2:
            ; 'crash' if it was
            mov esi, error_loading_kernel_msg
            jmp halt

        .end:

      pop edi
      ; calculate the number of bytes EDI has to be increased by
      xor edx, edx
      xor ebx, ebx
      xor eax, eax    ; eax will keep the file name's length
      mov al, [byte edi + 7]
      push eax        ; save it for a bit later
      ; names are padded to a 4 byte boundary with null bytes
      ; so we have to figure out those 'missing' bits (nah, bytes)
      mov bl, 4
      div byte bl     ; al (name length) / 4
      ; al = name length / 4
      ; ah = name length % 4
      mov bl, 4
      sub bl, ah      ; bl = 4 - (name length % 4)

      ; actually increase EDI
      pop eax
      add edi, eax    ; file name length
      add edi, ebx    ; the 'missing bits'
      add edi, 8      ; that many bytes before the name
      ; }}}
    traverse_one_block_next:
      pop esi
      pop eax
      pop ebx
      pop ecx
      pop edx
      ; loop again if EDI - 0x17c00 < d_bsize
      ; this is probably not perfect, but it would have to do
      push eax
      mov eax, edi
      sub eax, dword 0x17c00
      cmp eax, dword [d_bsize]
      pop eax
      jb traverse_one_block

    ; }}}
  traverse_direct_blocks_next:
    pop esi
    pop edx
    pop ebx
    pop ecx

    cmp ecx, NDADDR
    jae traverse_direct_blocks_end
    inc ecx    ; ecx++
    add ebx, 8 ; move onto the next block
    jmp traverse_direct_blocks
  traverse_direct_blocks_end:

  ; TODO: traverse also indirect blocks

  ; if we got here (ie. the loop ended) it means that the kernel was not found
  call putnl
  mov esi, kernel_not_found_msg
  call putstr
  mov esi, kernel_name
  call putstr
  jmp halt

  ; }}}
  next_path_segment_next:
  pop ecx
  ; *esi++ != '\0'
  cmp byte [esi], 0x0
  je next_path_segment_end
  inc esi
  inc dword [kernel_name_ptr]
  jmp next_path_segment
  next_path_segment_end:

  ; if we got here (ie. the loop ended) it means that the kernel was not found
  call putnl
  mov esi, kernel_not_found_msg
  call putstr
  mov esi, kernel_name
  call putstr
  jmp halt

kernel_found:
  mov esi, kernel_found_msg
  call putstr
  mov esi, kernel_name
  call putstr
  call putnl
  call putnl

  ; now, to load the kernel
  ; temporary location to where to load a block (just above the inode)
  mov dx, 0x17c0
  mov es, dx
  xor bx, bx       ; es:bx = 0x17c0:0x0000 (= 0x17c00)
  ; where to move the block from the temporary location
  ; but actually, this is not the actual location
  ; for now we're loading the kernel to:
  ; 0x100000 + kernel_fsize + (kernel_blksz - kernel_fsize % kernel_blksz)
  ; ..or in another words to:
  ; kernel_fsize bytes, aligned to kernel_blksz boundary, past the 0x100000 mark
  ;
  ; this is to make sure when we're relocating all the ELF stuff to 0x100000
  ; that there is enough space
  mov edi, 0x100000
  ; align the load location to a `kernel_blksz' boundary
  push edx
  push ebx

  xor edx, edx
  mov eax, [kernel_fsize]
  mov ebx, [kernel_blksz]
  div ebx           ; edx:eax (kernel's file size) / kernel_blksz
  ; eax = file size / kernel_blksz
  ; edx = file size % kernel_blksz
  mov ebx, [kernel_blksz]
  sub ebx, edx      ; ebx = kernel_blksz - (file size % kernel_blksz)
  add ebx, [kernel_fsize]
  add edi, ebx

  pop ebx
  pop edx

  mov [kernel_preloc], edi

  ; first, direct blocks (0x70 is the offset of the direct blocks array)
  mov edx, 0x17a70
  ; loop NDADDR times
  mov ecx, NDADDR

%ifdef DEBUG
  ; {{{
  push esi
  push edi
  push edx
  mov esi, kernel_loading_to_msg
  call putstr
  mov edx, edi
  call puthex
  call putnl
  call putnl
  pop edx
  pop edi
  pop esi
  ; }}}
%endif

  .load_direct_blocks:
    ; don't even try loading blocks which's addresses are nul
    cmp dword [edx], 0x0
    je .load_direct_blocks_next

    push ecx
    push edx
    push esi
    push es
    push ebx

    ; load the block to a temporary location (0x17c00)
    mov ecx, [edx]
    mov eax, [kernel_blksz]
    call load_blk

    ; move it to above 1MiB
    mov esi, 0x17c00
    ;   edi is already set

    ; load 4 bytes at a time
    xor edx, edx
    mov eax, [kernel_blksz]
    mov ecx, 4
    div ecx  ; eax = kernel_blksz / 4
    mov ecx, eax

    .move:
      mov edx, [esi]
      mov [edi], edx

      add esi, 4
      add edi, 4
      loop .move

    pop ebx
    pop es
    pop esi
    pop edx
    pop ecx

    ; increase the current block address by 64 bits
    add edx, 8
  .load_direct_blocks_next:
  loop .load_direct_blocks

  ; TODO: load also indirect blocks

; load the ELF sections to where they belong
relocate:
  mov esi, [kernel_preloc]

  ; fetch the ELF header variables
%macro fetch_dd 2
  mov eax, dword [esi + %2]
  mov [e_%1], eax
%endmacro

%macro fetch_dw 2
  xor eax, eax
  mov ax, word [esi + %2]
  mov [e_%1], ax
%endmacro

  fetch_dd phoff,     0x1c
  fetch_dw phnum,     0x2c
  fetch_dw phentsize, 0x2a

%undef fetch_dd
%undef fetch_dw

  ; update esi to point to the offset of the program headers / sections
  add esi, [e_phoff]
  ; loop `e_phnum' times
  xor ecx, ecx
  mov cx, [e_phnum]

  .loop_phdrs:
    push esi
    push ecx

    ; edx contains `p_type'
    mov edx, dword [esi]
    ; test if the section's loadable
    and edx, PT_LOAD

    cmp edx, 0
    je .loop_phdrs_next

    .is_loadable:
      mov eax, [kernel_preloc]
      ; 0x4 is the offset of `p_offset'
      add eax, [esi + 0x04]
      ; 0x10 is the offset of `p_filesz' (ie. number of bytes to copy)
      mov ecx, [esi + 0x10]
      ; edi now points to the section's `p_vaddr'
      mov edi, [esi + 0x08]
      ; esi now points to the section's actual contents in memory
      mov esi, eax

      ; don't load sections that want to go nowhere
      cmp edi, 0x0
      je .loop_phdrs_next

%ifdef DEBUG
; {{{
      push esi
      mov edx, esi
      mov esi, relocating_section_from_msg
      call putstr
      call puthex
      mov esi, relocating_section_to_msg
      call putstr
      mov edx, edi
      call puthex
      call putnl
      pop esi
; }}}
%endif

      .move:
        mov ebx, [esi]
        mov [edi], ebx

        add esi, 1
        add edi, 1
      loop .move

    .loop_phdrs_next:
      pop ecx
      pop esi
      add si, [e_phentsize]
  loop .loop_phdrs

enter_pmode:
  cli
  lgdt [gdt]
  mov eax, cr0
  or al, 1
  mov cr0, eax
  jmp 0x08:pmode

BITS 32
pmode:
  mov eax, 0x10
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax
  mov ss, eax
  ; is that necessary?
  mov esp, 0x5c00

  ; that's upsetting..
  jmp $

halt:
  cli
  hlt
  jmp halt

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

; Superblock variables
;
; {{{
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
; }}}

; kernel's ELF header variables
;
; {{{
; offset of the program headers
e_phoff: dd 0
; number of the program headers
e_phnum: dw 0
; program header's size in the program section header table
e_phentsize: dw 0
; }}}

; number of heads
number_of_heads: db 0
; sectors per track
sectors_per_track: db 0
; when loading the superblock, this is the head value
head: db 0
; this is the cylinder number
cylinder: db 0
; and the sector
sector: db 0
; number of the drive we have booted from
bootdrv: db 0

; the messages
kernel_no_elf_msg: db ': the ELF magic was not found!', 0xd, 0xa, 0
load_blk_msg: db 'called load_blk with arg: ', 0
blks_addr_msg: db 'blocks address is: ', 0
loading_inode_msg1: db 'loading inode ', 0
loading_inode_msg2: db ' into memory', 0xd, 0xa, 0
traversing_block_msg: db 'traversing block ', 0
kernel_not_found_msg: db "couldn't find ", 0
kernel_found_msg: db 'kernel found: ', 0
kernel_loading_to_msg: db 'loading the kernel to location ', 0
error_loading_kernel_msg: db 'error loading kernel!', 0xd, 0xa, 0
relocating_section_from_msg: db 'relocating section from ', 0
relocating_section_to_msg: db ' to ', 0
loading_block_to_msg: db 'loading block to location ', 0
searching_for_msg: db 'searching for directory/file: ', 0
found_inode_msg: db 'found inode: ', 0
isafile_msg: db '.. which is a file', 0xd, 0xa, 0
isadir_msg: db '.. which is a directory', 0xd, 0xa, 0
isunknown_msg: db '.. which is unknown..', 0xd, 0xa, 0
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

