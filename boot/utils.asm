; sets a memory to a given 'state'
;
; param:  EAX - place in the memory
;          BL - what to overwrite with
;         ECX - how many bytes to overwrite
memset:
; {{{
  push ecx
  push eax

  .1:
    mov byte [eax],  bl
    inc dword eax
  loop .1

  pop eax
  pop ecx
  ret
; }}}

; compares a string in ESI with the one in EDI
;
; param:  ESI - first string
;         EDI - second string
; return: 0 in CF if they are equal
;         1 in CF if they are not
streq:
; {{{
  push ax
  push ds
  push esi
  push edi

  ; make sure DS is set properly, so zero it out, ORG has got it covered
  xor ax, ax
  mov ds, ax

  .while:
    mov ah, byte [esi]
    mov al, byte [edi]
    inc edi
    inc esi
    cmp ah, al
    jne .fail
    cmp al, 0
    je .ok
    jmp .while

  .fail:
    stc
    jmp .end
  .ok:
    clc

  .end:
  pop edi
  pop esi
  pop ds
  pop ax
  ret
; }}}

; converts a given LBA address into the CHS equivalent
;
; param: ECX - the LBA
; return: CH - cylinder
;         DH - head
;         CL - sector
;         DL - the drive we've booted from
lba_to_chs:
; {{{
  push ds

  ; make sure DS is set properly, so zero it out, ORG has got it covered
  xor ax, ax
  mov ds, ax

  jmp varsend
    c: db 0
    h: db 0
    s: db 0
  varsend:

  mov eax, ecx
  xor edx, edx
  xor ecx, ecx
  mov cl, [sectors_per_track]
  div dword ecx
  ; eax = LBA / sectors_per_track
  ; edx = LBA % sectors_per_track
  inc dl
  and dl, 0x3f
  mov [s], dl
  xor edx, edx
  xor ecx, ecx
  mov cl, [number_of_heads]
  div dword ecx
  ; eax = eax / number of heads
  ; edx = eax % number of heads
  mov [h], dl
  mov [c], al
  and ax, 0x300
  shr ax, 2
  or al, byte [s]
  mov [s], al

  mov ch, [c]
  mov dh, [h]
  mov cl, [s]
  mov dl, [bootdrv]

  pop ds
  ret
; }}}

; credit: http://wiki.osdev.org/A20_Line
;
; return: 0 in AX if the a20 line is disabled (memory wraps around)
;         1 in AX if the a20 line is enabled (memory does not wrap around)
check_a20:
; {{{
  pushf
  push ds
  push es
  push di
  push si

  cli

  xor ax, ax ; ax = 0
  mov es, ax

  not ax ; ax = 0xFFFF
  mov ds, ax

  mov di, 0x0500
  mov si, 0x0510

  mov al, byte [es:di]
  push ax

  mov al, byte [ds:si]
  push ax

  mov byte [es:di], 0x00
  mov byte [ds:si], 0xFF

  cmp byte [es:di], 0xFF

  pop ax
  mov byte [ds:si], al

  pop ax
  mov byte [es:di], al

  mov ax, 0
  je .exit

  mov ax, 1

  .exit:
    pop si
    pop di
    pop es
    pop ds
    popf

    ret
; }}}

; credit: http://wiki.osdev.org/A20_Line
;
; _attempts_ to enable the a20 line using the keyboard controller
enable_a20_via_kbd:
; {{{
  cli

  call .wait
  mov al,0xAD
  out 0x64,al

  call .wait
  mov al,0xD0
  out 0x64,al

  call .wait2
  in al,0x60
  push eax

  call .wait
  mov al,0xD1
  out 0x64,al

  call .wait
  pop eax
  or al,2
  out 0x60,al

  call .wait
  mov al,0xAE
  out 0x64,al

  call .wait
  sti
  ret

  .wait:
    in al,0x64
    test al,2
    jnz .wait
    ret

  .wait2:
    in al,0x64
    test al,1
    jz .wait2
    ret
; }}}

; calculate the physical address of a cylinder group
;
; param:  ECX - # of the cylinder group
; return: EDX:EAX - the address
cg_addr:
; {{{
  ; cgbase(N) = fs_fpg * N
  ; cgtod(N) = cgbase(N) + fs_cblkno
  ; tell = fsbtodb(cgtod(ECX)) * d_bsize
  push ecx
  push ebx
  push ds

  ; make sure DS is set properly, so zero it out, ORG has got it covered
  xor ax, ax
  mov ds, ax

  xor edx, edx
  mov eax, [fs_fpg]
  mul ecx

  add eax, [fs_cblkno]

  mov ecx, [fs_fsbtodb]
  shl eax, cl

  mul dword [d_bsize]

  pop ds
  pop ebx
  pop ecx
  ret
; }}}

; calculate the physical address of a CG's inode table
;
; param:  ECX - # of the cylinder group
; return: EDX:EAX - the address
cg_inodes_addr:
; {{{
  ; cgbase(N) = fs_fpg * N
  ; cgimin(N) = cgbase(N) + fs_iblkno
  ; phcgimin = fsbtodb(cgimin(ECX)) * d_bsize
  push ecx
  push ebx
  push ds

  ; make sure DS is set properly, so zero it out, ORG has got it covered
  xor ax, ax
  mov ds, ax

  xor edx, edx
  mov eax, [fs_fpg]
  mul ecx

  add eax, [fs_iblkno]

  mov ecx, [fs_fsbtodb]
  shl eax, cl

  mul dword [d_bsize]

  pop ds
  pop ebx
  pop ecx
  ret
; }}}

; calculate the physical address of a CG's data blocks start
;
; param:  ECX - # of the cylinder group
; return: EDX:EAX - the address
cg_data_addr:
; {{{
  ; cgbase(N) = fs_fpg * N
  ; cgdmin(N) = cgbase(N) + fs_dblkno
  ; phcgdmin = fsbtodb(cgdmin(ECX)) * d_bsize
  push ecx
  push ebx
  push ds

  ; make sure DS is set properly, so zero it out, ORG has got it covered
  xor ax, ax
  mov ds, ax

  xor edx, edx
  mov eax, [fs_fpg]
  mul ecx

  add eax, [fs_dblkno]

  mov ecx, [fs_fsbtodb]
  shl eax, cl

  mul dword [d_bsize]

  pop ds
  pop ebx
  pop ecx
  ret
; }}}

; calculates a block's number into it's physical address on the disk
;
; param:  ECX - block's #
; return: ECX - the physical address
blk_addr:
; {{{
  push eax
  push edx
  push ds

  ; make sure we can access the 'variables'
  ; ORG is already set to the correct value
  xor eax, eax
  mov ds, eax

  ; calculate!
  mov eax, ecx
  mov ecx, [fs_fsbtodb]
  shl eax, cl

  xor edx, edx
  mul dword [d_bsize]
  mov ecx, eax

  pop ds
  pop edx
  pop eax
  ret
; }}}

; loads a block into the memory
;
; param:  ECX - block's #
;         ES:BX - where to load the block
; return: 1 in CF if the loading failed
;         0 in CF if the loading succeeded
load_blk:
; {{{
  push ecx
  push esi

%ifdef DEBUG
; {{{
  mov esi, load_blk_msg
  call putstr
  mov edx, ecx
  call puthex
  call putnl
; }}}
%endif

  call blk_addr

%ifdef DEBUG
; {{{
  mov esi, blks_addr_msg
  call putstr
  mov edx, ecx
  call puthex
  call putnl
; }}}
%endif
  xor edx, edx
  mov eax, ecx
  ; edx:eax - the block's address
  mov ecx, 512
  div dword ecx
  ; eax = block addr / 512 (LBA)
  ; edx = block addr % 512
  mov ecx, eax
  call lba_to_chs
  ; now ch, dh, cl and dl contain the right values
  ; es and bx also should be upright, but that's up to the caller

  ; calculate the number of sectors to load (fs_fsize * fs_frag / 512)
  push edx
  push ecx
  xor edx, edx
  mov eax, [fs_fsize]
  mov ecx, [fs_frag]
  mul ecx
  mov ecx, 512
  div dword ecx
  pop ecx
  pop edx

  .load:
    mov ah, 0x02
    int 13h
    jc .load

  ; it wouldn't have worked if we didn't get here
  clc

  pop ecx
  pop esi

  ret
; }}}

; calculate the physical address from a given inode's number
;
; param:  ECX - the inode's number
; return: EDX:EAX - the address
inode_addr:
; {{{
  ; address = cginoloc(inode / fs_ipg) + (inode % fs_ipg) * inode size
  push ecx
  push ds

  ; make sure DS is set properly, so zero it out, ORG has got it covered
  xor ax, ax
  mov ds, ax

  xor edx, edx
  mov eax, ecx
  div dword [fs_ipg]
  ; eax = inode / fs_ipg
  ; edx = inode % fs_ipg
  push edx
  mov ecx, eax
  call cg_inodes_addr
  pop edx
  ; eax = cginoloc
  ; edx = inode % fs_ipg
  push eax
  mov eax, edx
  xor edx, edx
  mov ecx, dword INODE_SIZE
  mul dword ecx
  ; edx:eax = (inode % fs_ipg) * inode size
  pop ecx
  ; ecx = cginoloc
  add eax, ecx

  pop ds
  pop ecx
  ret
; }}}

; param:  ECX - the inode's number
;         ES:BX - where to load the data
; return: 1 in CF if the loading failed
;         0 in CF if the loading succeeded
load_inode:
; {{{
  pushad
  call inode_addr
  ; edx:eax - the inode's address
  mov edi, es
  shl edi, 4
  add di, bx
  mov esi, edi
  mov ecx, 512
  div dword ecx
  ; eax = inode addr / 512
  ; edx = inode addr % 512
  mov ecx, eax
  add esi, edx

  call lba_to_chs
  ; now ch, dh, cl and dl contain the right values
  ; es and bx also should be upright, but that's up to the caller
  .load:
    mov ah, 0x02
    mov al, 0x01    ; we actually need only half a sector
    int 13h
    jc .load

  ; because we only need half a sector, sometimes the actual inode is going to
  ; be in the upper half of the sector, so we have to move it the requested
  ; location
  mov ecx, 0x40 ; 256 bytes, 0x40 dwords
  .move:
    mov edx, [esi]
    mov [edi], edx
    add esi, 4
    add edi, 4
    loop .move

  ; it wouldn't have worked if we didn't get here
  clc

  popad
  ret
; }}}

; vi: ft=nasm:ts=2:sw=2 expandtab

