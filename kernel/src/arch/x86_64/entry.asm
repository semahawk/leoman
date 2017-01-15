;
; entry.S
; Copyright (C) 2017 Szymon Urbaś <szymon.urbas@aol.com>
; Distributed under terms of the BSD (2-clause) license.
;
; Created on: 06 Jan 2017 21:23:34 +0100 (CET)
;

bits 32

section .multiboot_header
multiboot_header_start:
  ; magic number (multiboot 2)
  dd 0xe85250d6
  ; architecture 0 (protected mode i386)
  dd 0
  ; header length
  dd multiboot_header_end - multiboot_header_start
  ; checksum
  dd 0x100000000 - (0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))

  ; required end tag
  dw 0    ; type
  dw 0    ; flags
  dd 8    ; size
multiboot_header_end:

global start
section .text
start:
  ; print `OK` to screen
  mov dword [0xb8000], 0x2f4b2f4f
  hlt

; vi: ft=nasm ts=2 sw=2 expandtab
;

