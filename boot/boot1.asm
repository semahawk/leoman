BITS 16
ORG 0x7c00

jmp near boot1

; the kernel's path
kernel_name: db '/boot/kernel', 0
; where the kernel is located before ELF relocation (meh) to 1MiB
kernel_preloc equ 0x1000000 ; that's 16MiB
; kernel's entry point
kernel_entry: dd 0

; the initrd's file name
initrd_name: db '/boot/initrd', 0
; where to load it
; yes, it would overwrite the kernel, but the kernel's been already
; relocated (ELF paddr thingy) so it's no problem
initrd_load_addr equ 0x1000000 ; 16MiB

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

  mov esi, kernel_name
  mov eax, kernel_preloc
  call load_file
  ; no problems, do the relocation
  jnc relocate
  ; oh no, there were problems loading the kernel
  mov esi, kernel_not_found_msg
  call putstr
  mov esi, kernel_name
  call putstr
  call putnl
  jmp halt

; load the ELF sections to where they belong
relocate:
  mov esi, kernel_preloc

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

  ; a nonstandard name for 'fetch'
  mov eax, [esi + 0x18]
  mov [kernel_entry], eax

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
      mov eax, kernel_preloc
      ; edi now points to the section's `p_paddr'
      mov edi, [esi + 0x0c]

      ; don't load sections that want to go nowhere
      cmp edi, 0x0
      je .loop_phdrs_next

      ; 0x4 is the offset of `p_offset'
      add eax, [esi + 0x04]
      ; 0x10 is the offset of `p_filesz' (ie. number of bytes to copy)
      mov ecx, [esi + 0x10]
      push ecx
      ; 0x14 is the offset of `p_memsz'
      mov ebx, [esi + 0x14]
      push ebx
      ; esi now points to the section's actual contents in memory
      mov esi, eax

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

      ; relocate the section's contents
      .move:
        mov ebx, [esi]
        mov [edi], ebx

        add esi, 1
        add edi, 1
      loop .move

      ; zero out the remainder of memsz (`p_memsz' - `p_filesz')
      pop eax  ; eax = `p_memsz'
      pop ebx  ; ebx = `p_filesz'

      sub eax, ebx

      xor edx, edx
      mov ebx, 4
      div ebx

      mov ecx, eax

      ; don't zero out if there's nothing to be zeroed
      ; for example when `p_memsz' is equal to `p_filesz'
      cmp ecx, 0
      je .dont_zero_out

      mov esi, zeroing_msg
      call putstr
      call putnl

      .zero_out:
        mov [edi], dword 0x0

        add edi, 4
      loop .zero_out
      .dont_zero_out:

    .loop_phdrs_next:
      pop ecx
      pop esi
      add si, [e_phentsize]

  ;loop .loop_phdrs
  dec ecx
  jnz .loop_phdrs

detect_memory:
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
    jmp detect_memory_end

  .fail:
    mov esi, memory_map_failed_msg
    call putstr
    jmp halt

detect_memory_end:

load_initrd:
  mov esi, initrd_name
  mov eax, initrd_load_addr
  call load_file
  jnc load_initrd_end

  mov esi, problems_loading_msg
  call putstr
  mov esi, initrd_name
  call putstr
  call putnl

  jmp halt

load_initrd_end:
  ; set the `bootinfo' field
  mov dword [initrd_addr], initrd_load_addr
  mov dword [initrd_size], eax

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

  mov eax, bootinfo
  push eax

  ; farewell!
  jmp [kernel_entry]

halt:
  cli
  hlt
  jmp halt

;
; the `struct bootinfo' definition
; the field order and sizes must match with those in the declaration
; of `struct bootinfo' (found in kernel/common.h)
;
bootinfo:
; {{{
initrd_addr: dd 0xffffffff
initrd_size: dd 0xffffffff
mem_avail: dd 0x0
memory_map: times 24 * 64 db 0 ; max 64 entries (is it enough?)
; }}}

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
problems_loading_msg: db 'oops, there were problems loading ', 0
kernel_found_msg: db 'kernel found: ', 0
kernel_loading_to_msg: db 'loading the kernel to location ', 0
error_loading_kernel_msg: db 'error loading kernel!', 0xd, 0xa, 0
relocating_section_from_msg: db 'relocating section from ', 0
relocating_section_to_msg: db ' to ', 0
loading_block_to_msg: db 'loading block to location ', 0
searching_for_msg: db 'searching for directory/file: ', 0
found_inode_msg: db 'found inode: ', 0
zeroing_msg: db 'p_memsz is bigger than p_filesz, zeroing the remainder', 0
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
enable_a20_fail_msg: db 'fatal: failed to enable the a20 line!', 0xd, 0xa, 0
magic_not_found_msg: db 'fatal: UFS2 magic not found!', 0xd, 0xa, 0
memory_map_failed_msg: db 'fatal: failed to detect the memory map!', 0xd, 0xa, 0
welcome_msg: db 'Figh is booting...', 0xd, 0xa, 0xd, 0xa, 0

; make it be 127 sectors wide
times 512*127-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

