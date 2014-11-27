; param:  ECX - inode's size
;         EBX - lbn (logical block number (ie. how many blocks is it needed to
;               contain the file's contents))
; return: EAX - the result (the inode's block's size)
blksize:
  cmp ebx, NDADDR
  jge .return_fs_bsize

  inc ebx   ; lbn++
  mov eax, ebx
  call smalllblktosize
  cmp ecx, eax
  jge .return_fs_bsize

  jmp .return_fragroundup

  .return_fs_bsize:
    mov eax, [fs_bsize]
    jmp .end

  .return_fragroundup:
    mov eax, ecx
    call blkoff
    call fragroundup
    jmp .end

  .end:
call putnl
  ret

; param:  EAX - loc
; return: EAX - the result of loc % fs_bsize
blkoff:
  push ebx
  push edx

  xor edx, edx
  mov ebx, [fs_bsize]
  div ebx
  mov eax, edx

  pop edx
  pop ebx
  ret

; param:  EAX - the file's length
; return: EAX - the result of the file's length / fs_bsize
lblkno:
  push ebx
  push edx

  xor edx, edx
  mov ebx, [fs_bsize]
  div ebx

  pop edx
  pop ebx
  ret

; param:  EAX - blk
; return: EAX - the result of blk * fs_bsize
; note:   the high part of the result is ignored
smalllblktosize:
  push ebx
  push edx
  mov ebx, [fs_bsize]
  mul ebx
  ; eax = the low part of the result
  pop edx
  pop ebx
  ret

; param:  EAX - size
; return: EAX - the result of roundup(size, fs_fsize)
fragroundup:
  push ebx
  mov ebx, [fs_fsize]
  call roundup
  pop ebx

  ret

; param:  ESI - the file's name
;         EAX - the address at which to load the file
; return: EAX - size of the loaded file (if was possible)
;         carry is cleared if the file was found
;         carry is set if there were problems loading
load_file:
  jmp load_file_vars
    name_ptr:  dd 0
    load_addr: dd 0
    file_size: dd 0
  load_file_vars:

  mov [name_ptr], esi
  mov [load_addr], eax

  ; load the / inode just into above boot1
  mov ax, 0x17a0
  mov es, ax
  xor bx, bx        ; es:bx = 0x17a0:0x0000 (= 0x17a00)
  ; the inode number
  mov ecx, 0x2
  call load_inode

  ; while (*esi != '\0')
  cmp byte [esi], 0x0
  inc esi
  inc dword [name_ptr]
  je next_path_segment_end

  jmp next_path_segment
  ; the name buffer where each path's segment will be kept
  name_buffer: times MAX_FNAME_SIZE + 1 db 0
  ; whether the current segment is last in the path
  last_segment: db 0

  next_path_segment:
  push ecx ; +
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
    push eax ; +
    push edx ; +
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
    pop edx ; +
    pop eax ; +

    inc esi
    inc dword [name_ptr]
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
  push esi ; +
  mov esi, searching_for_msg
  call putstr
  mov esi, name_buffer
  call putstr
  call putnl
  pop esi ; +
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
    push ecx ; +
    push ebx ; +
    push edx ; +
    push esi ; +
    ; {{{

    ; don't bother even trying to traverse a block which's address is zero
    cmp dword [ebx], 0
    je traverse_direct_blocks_next

    ; load the current block into the memory into just above the inode
    load_the_current_block:
      push ecx ; +
      push ebx ; +
      ; {{{

      ; XXX i'm not sure that this block size is right
      mov eax, [fs_bsize]
      mov ecx, [ebx]
      mov bx, 0x17c0
      mov es, bx
      xor bx, bx     ; es:bx = 0x17c0:0x0000 (= 0x17c00)

      call load_blk

      ; }}}
      pop ebx ; +
      pop ecx ; +

%ifdef DEBUG
  ; {{{
    push esi ; +
    mov esi, traversing_block_msg
    call putstr
    pop esi ; +
    push edx ; +
    mov edx, [ebx]
    call puthex
    call putnl
    pop edx ; +
  ; }}}
%endif

    mov edi, 0x17c00
    traverse_one_block:
      push edx ; +
      push ecx ; +
      push ebx ; +
      push eax ; +
      push esi ; +
      ; {{{
      push edi ; +

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
        jmp .next
      .yup:

%ifdef DEBUG
  ; {{{
        push esi ; +
        mov esi, found_inode_msg
        call putstr
        mov esi, name_buffer
        call putstr
        call putnl
        pop esi ; +
  ; }}}
%endif

        ; see what kind of a thing that was what we found
        pop edi ; +
        cmp byte [edi + 6], 0x8
        je .file
        cmp byte [edi + 6], 0x4
        je .directory
        ; none of the above, strange
        mov esi, isunknown_msg
        call putstr
        stc
        ret

        .file:
          ; code for a file
%ifdef DEBUG
  ; {{{
          push esi ; +
          mov esi, isafile_msg
          call putstr
          pop esi ; +
  ; }}}
%endif
          ; see if it was the last segment
          cmp byte [last_segment], 0
          je .file_not_found

            ; load it's inode in place of the previous one
            mov ecx, dword [edi]
            mov ax, 0x17a0
            mov es, ax
            xor bx, bx     ; es:bx = 0x17a0:0x0000 (= 0x17a00)

%ifdef DEBUG
  ; {{{
            push esi ; +
            mov esi, loading_inode_msg1
            call putstr
            mov edx, ecx
            call puthex
            mov esi, loading_inode_msg2
            call putstr
            pop esi ; +
  ; }}}
%endif

            call load_inode
            ; if there was no problem loading the inode, we've found the file
            ; but, if the carry was set, we've hit a problem
            jc .file_not_found

            ; save the file's size
            mov ebx, 0x17a10 ; 0x17a00 + 0x10 (offset of the `size' field)
            mov eax, [ebx]
            mov dword [file_size], eax

            ; clean up the stack
            add esp, 23 * 4 ; 23 4-byte values were pushed
            jmp file_found

            .file_not_found:
              stc ; set the carry, the file was not found (or there was a
                  ; problem loading it's inode
              ret

        .directory:
          ; code for a directory

%ifdef DEBUG
  ; {{{
          push esi ; +
          mov esi, isadir_msg
          call putstr
          pop esi ; +
  ; }}}
%endif

          ; see if it was the last segment
          cmp byte [last_segment], 1
          je .file_is_a_directory
            ; load the inode in the place of the actual one
            mov ecx, dword [edi]
            mov ax, 0x17a0
            mov es, ax
            xor bx, bx     ; es:bx = 0x17a0:0x0000 (= 0x17a00)

%ifdef DEBUG
  ; {{{
            push esi ; +
            push edx ; +
            mov esi, loading_inode_msg1
            call putstr
            mov edx, ecx
            call puthex
            mov esi, loading_inode_msg2
            call putstr
            pop edx ; +
            pop esi ; +
  ; }}}
%endif

            call load_inode
            jc .file_is_a_directory
            mov esi, dword [name_ptr]
            jmp next_path_segment_next

          .file_is_a_directory:
            stc ; set the carry flag
            ret

        .next:

      pop edi ; +
      ; calculate the number of bytes EDI has to be increased by
      xor edx, edx
      xor ebx, ebx
      xor eax, eax    ; eax will keep the file name's length
      mov al, [byte edi + 7]
      push eax ; +    ; save it for a bit later
      ; names are padded to a 4 byte boundary with null bytes
      ; so we have to figure out those 'missing' bits (nah, bytes)
      mov bl, 4
      div byte bl     ; al (name length) / 4
      ; al = name length / 4
      ; ah = name length % 4
      mov bl, 4
      sub bl, ah      ; bl = 4 - (name length % 4)

      ; actually increase EDI
      pop eax ; +
      add edi, eax    ; file name length
      add edi, ebx    ; the 'missing bits'
      add edi, 8      ; that many bytes before the name
      ; }}}
    traverse_one_block_next:
      pop esi ; +
      pop eax ; +
      pop ebx ; +
      pop ecx ; +
      pop edx ; +
      ; loop again if EDI - 0x17c00 < d_bsize
      ; this is probably not perfect, but it would have to do
      push eax ; +
      mov eax, edi
      sub eax, dword 0x17c00
      cmp eax, dword [d_bsize]
      pop eax ; +
      jb traverse_one_block

    ; }}}
  traverse_direct_blocks_next:
    pop esi ; +
    pop edx ; +
    pop ebx ; +
    pop ecx ; +

    cmp ecx, NDADDR
    jae traverse_direct_blocks_end
    inc ecx    ; ecx++
    add ebx, 8 ; move onto the next block
    jmp traverse_direct_blocks
  traverse_direct_blocks_end:

  ; TODO: traverse also indirect blocks

  ; if we got here (ie. the loop ended) it means that the file was not found
  stc
  ret

  ; }}}
  next_path_segment_next:
  pop ecx ; +
  ; *esi++ != '\0'
  cmp byte [esi], 0x0
  je next_path_segment_end
  inc esi
  inc dword [name_ptr]
  jmp next_path_segment
  next_path_segment_end:

  ; if we got here (ie. the loop ended) it means that the file was not found
  stc
  ret

  file_found:

  ; now, to load the file to the specified location
  ; temporary location to where to load a block (just above the inode)
  mov dx, 0x17c0
  mov es, dx
  xor bx, bx       ; es:bx = 0x17c0:0x0000 (= 0x17c00)

  ; first, direct blocks (0x70 is the offset of the direct blocks array)
  mov edx, 0x17a70
  ; the file's final location
  mov edi, [load_addr]
  ; loop NDADDR times
  mov ecx, NDADDR

  .load_direct_blocks:
    ; don't even try loading blocks which's addresses are nul
    cmp dword [edx], 0x0
    je .load_direct_blocks_next

    push ecx ; +
    push edx ; +
    push esi ; +
    push es ; +
    push ebx ; +
; {{{
    ; load the block to a temporary location (0x17c00)
    mov ecx, [edx]
    mov eax, [fs_bsize]
    call load_blk

    ; move it to the specified location
    mov esi, 0x17c00
    ;   edi is already set

    ; load 4 bytes at a time
    xor edx, edx
    mov eax, [fs_bsize]
    mov ecx, 4
    div ecx  ; eax = fs_bsize / 4
    mov ecx, eax

    .move:
      mov edx, [esi]
      mov [edi], edx

      add esi, 4
      add edi, 4
      loop .move
; }}}
    pop ebx ; +
    pop es ; +
    pop esi ; +
    pop edx ; +
    pop ecx ; +

    ; increase the current block address by 64 bits
    add edx, 8

  .load_direct_blocks_next:
  loop .load_direct_blocks

  ; TODO: load also indirect blocks

  mov eax, [file_size]

  clc
  ret

; vi: ft=nasm:ts=2:sw=2 expandtab

