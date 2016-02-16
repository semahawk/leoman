dispatch_elf_sections:
  ; 0x10000 is where files are loaded
  mov ebx, dword 0x10000
  ; load phnum (number of program headers)
  mov cx, [ebx + 0x2c]
  ; update the si to point at the first program header
  add ebx, [ebx + 0x1c]

.load_program_headers:
  push cx
  ; offset zero containts the program header's type
  cmp dword [ebx], 1 ; 1 = PT_LOAD
  jne .load_next_program_header

  mov esi, 0x10000
  add esi, dword [ebx + 0x04] ; source: offset 0x04 is p_offset
  mov edi, dword [ebx + 0x0c] ; destination: offset 0x0c is p_paddr
  mov ecx, dword [ebx + 0x10] ; offset 0x10 is p_filesz (so number of bytes)
  ; divide ecx by four
  xor edx, edx
  mov eax, ecx
  mov ecx, 4
  div ecx
  mov ecx, eax

  ; we don't want to perform the first loop if ecx = 0 already
  cmp ecx, 0
  je .check_if_memsz_bigger_than_filesz

  ; TODO check for any remainders
  ; btw, rep movsd is crazy (or I am)
.relocate_program_header:
  mov edx, dword [esi]
  mov dword [edi], edx
  add esi, 4
  add edi, 4
  loop .relocate_program_header

.check_if_memsz_bigger_than_filesz:
  ; see if the header's memsz is bigger than filesz
  ; if so, then zero out remainder of memsz
  mov eax, [ebx + 0x14] ; offset 0x14 is p_memsz
  mov edx, [ebx + 0x10] ; offset 0x10 is p_filesz
  cmp eax, edx
  jbe .load_next_program_header

  ; store the differece of p_memsz - p_filesz
  sub eax, edx
  xor edx, edx
  mov ecx, 4
  div ecx
  mov ecx, eax
.zero_out_remainder:
  mov dword [edi], 0
  add edi, 4
  loop .zero_out_remainder

.load_next_program_header:
  pop cx
  ; point ebx at the next program header
  ; (offset 0x2a contains program header's size)
  xor edx, edx
  mov esi, 0x10000
  mov dx, word [esi + 0x2a]
  add ebx, edx
  ; no 'loop' because the jump address is too far away
  dec cx
  jnz .load_program_headers

.done:
  ret

; vi: ft=nasm:ts=2:sw=2 expandtab

