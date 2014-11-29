/*
 *
 * elf.h
 *
 * Created at:  Thu 27 Nov 20:37:42 2014 20:37:42
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef ELF_H
#define ELF_H

#include <stdint.h>

struct elf_header {
  uint32_t e_magic;       /* the magic number */
  uint8_t  e_ident[12];   /* file identification */
  uint16_t e_type;        /* file type */
  uint16_t e_machine;     /* machine architecture */
  uint32_t e_version;     /* ELF format version */
  uint32_t e_entry;       /* program's entry point */
  uint32_t e_phoff;       /* program header file offset */
  uint32_t e_shoff;       /* section header file offset */
  uint32_t e_flags;       /* architecture-specific flags */
  uint16_t e_ehsize;      /* size of ELF header in bytes */
  uint16_t e_phentsize;   /* size of each program header */
  uint16_t e_phnum;       /* number of program headers */
  uint16_t e_shentsize;   /* size of each section header */
  uint16_t e_shnum;       /* number of section headers */
  uint16_t e_shstrndx;    /* section name strings section */
};

struct elf_pheader {
  uint32_t p_type;           /* entry type */
  uint32_t p_offset;         /* file offset of contents */
  uint32_t p_vaddr;          /* virtual address in memory image */
  uint32_t p_paddr;          /* physical address */
  uint32_t p_filesz;         /* size of contents in file */
  uint32_t p_memsz;          /* size of contents in memory */
  uint32_t p_flags;          /* access permission flags */
  uint32_t p_align;          /* alignment in memory and file */
};

struct elf_sheader {
  uint32_t s_name;           /* index into the section header string table */
  uint32_t s_type;           /* section type */
  uint32_t s_flags;          /* section flags */
  uint32_t s_addr;           /* address in memory image */
  uint32_t s_offset;         /* offset in file */
  uint32_t s_size;           /* size in bytes */
  uint32_t s_link;           /* index of a related section */
  uint32_t s_info;           /* depends on section type */
  uint32_t s_addralign;      /* alignment in bytes */
  uint32_t s_entsize;        /* size of each entry in section */
};

/* program header types */
#define PT_NULL 0
#define PT_LOAD 1

void *elf_get_entry(const void *);
int   elf_execute(const void *);

#endif /* ELF_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

