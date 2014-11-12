/*
 *
 * paging.c
 *
 * Created at:  Sat  8 Nov 14:09:21 2014 14:09:21
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include "common.h"
#include "paging.h"

static int paging_enabled = 0;
/* those addresses are physical */
static uint32_t *page_directory;
static uint32_t *page_directory_end;

/*
 * Map a single page
 */
void map_page(void *paddr, void *vaddr, unsigned int flags)
{
  paddr = PALIGN(paddr);
  vaddr = PALIGN(vaddr);

  uint32_t pdir_idx = (uint32_t)vaddr >> 22;
  uint32_t ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

  uint32_t *pdir = page_directory;
  uint32_t *ptab = page_directory_end + pdir_idx * 0x400;

  pdir[pdir_idx] |= PDIR_ATTR_PRESENT;
  ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTAB_ATTR_PRESENT;
}

uint32_t *paging_init(struct kern_bootinfo *bootinfo)
{
  page_directory = PALIGN(bootinfo->kernel_addr + bootinfo->kernel_size);
  page_directory_end = page_directory + 0x400;

  /* setup the page directory */
  for (int i = 0; i < 1024; i++){
    /* attributes: supervisor level, read + write, not present */
    page_directory[i] = (uint32_t)(page_directory_end + i * 0x400) | PDIR_ATTR_RDWR;
  }

  /* 1MiB (BIOS &c) + kernel's size + 4KiB + 4MiB (page directory and tables) */
  unsigned npages = (0x100000 + bootinfo->kernel_size + 0x401000) / PAGE_SIZE;
  void *addr = 0x0;
  /* identity-map the first <npages> pages */
  for (int i = 0; i < npages - 1; i++, addr += PAGE_SIZE){
    map_page(addr, addr, PTAB_ATTR_RDWR);
  }

  /* actually enable paging */
  uint32_t cr0;

  __asm volatile("mov %0, %%cr3" : : "b"(page_directory));
  __asm volatile("mov %%cr0, %0" : "=b"(cr0));
  cr0 |= 0x80000000;
  __asm volatile("mov %0, %%cr0" : : "b"(cr0));

  paging_enabled = 1;

  return page_directory;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

