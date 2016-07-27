/*
 *
 * elf.c
 *
 * Created at:  Thu 27 Nov 21:12:04 2014 21:12:04
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include <kernel/common.h>
#include <kernel/elf.h>
#include <kernel/vm.h>
#include <kernel/proc.h>
#include <kernel/vga.h>

uint32_t *elf_load(const void *file)
{
  struct elf_header *hdr = (struct elf_header *)file;
  struct elf_pheader *phdr = (struct elf_pheader *)(file + hdr->e_phoff);

  for (int i = 0; i < hdr->e_phnum; i++, phdr++){
    switch (phdr->p_type){
      case PT_NULL:
        /* nuthin' */
        break;

      case PT_LOAD:
        map_pages((void *)file + phdr->p_offset, (void *)phdr->p_vaddr, PTE_W | PTE_U, phdr->p_memsz);
        break;
    }
  }

  return (uint32_t *)hdr->e_entry;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

