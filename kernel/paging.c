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

/* TODO: write a page frame allocator */

static uint32_t *page_directory;

void *paddr(void *vaddr)
{
  unsigned long pdir_idx = (unsigned long)vaddr >> 22;
  unsigned long ptab_idx = (unsigned long)vaddr >> 12 & 0x03ff;

  unsigned long *pdir = (unsigned long *)0xfffff000;
  /* TODO: check whether the page directory entry is present */
  unsigned long *ptab = ((unsigned long *)0xffc00000) + (0x400 * pdir_idx);
  /* TODO: check whether the page table entry is present */

  return (void *)((ptab[ptab_idx] & ~0xFFF) + ((unsigned long)vaddr & 0xfff));
}

void map_page(void *paddr, void *vaddr, unsigned int flags)
{
  /* TODO: make sure both paddr and vaddr are page-aligned */
  unsigned long pdir_idx = (unsigned long)vaddr >> 22;
  unsigned long ptab_idx = (unsigned long)vaddr >> 12 & 0x03ff;

  unsigned long *pdir = (unsigned long *)0xfffff000;
  /* TODO: check whether the page directory entry is present */
  unsigned long *ptab = ((unsigned long *)0xffc00000) + (0x400 * pdir_idx);
  /* TODO: check whether the page table entry is present */

  ptab[ptab_idx] = ((unsigned long)paddr) | (flags & 0xfff) | PTAB_ATTR_PRESENT;
}

void unmap_page(void *vaddr)
{
  /* TODO: make sure both paddr and vaddr are page-aligned */
  unsigned long pdir_idx = (unsigned long)vaddr >> 22;
  unsigned long ptab_idx = (unsigned long)vaddr >> 12 & 0x03ff;

  unsigned long *pdir = (unsigned long *)0xfffff000;
  /* TODO: check whether the page directory entry is present */
  unsigned long *ptab = ((unsigned long *)0xffc00000) + (0x400 * pdir_idx);
  /* TODO: check whether the page table entry is present */

 ptab[ptab_idx] = 0x0;
}

uint32_t *paging_init(struct kern_bootinfo *bootinfo)
{
  page_directory = (uint32_t *)PALIGN(bootinfo->kernel_addr + bootinfo->kernel_size);

  /* zero-out the page directory */
  for (int i = 0; i < 1024; i++){
    /* attributes: supervisor level, read + write, not present */
    page_directory[i] = PDIR_ATTR_RDWR;
  }

  /* create the first page table */
  uint32_t *page_table = page_directory + 1024;
  /* the first page table will identity page the first 4MiB */
  uint32_t addr = 0x0;

  for (int i = 0; i < 1024; i++, addr += PAGE_SIZE){
    /* attributes: supervisor level, read + write, present */
    page_table[i] = addr | PTAB_ATTR_RDWR | PTAB_ATTR_PRESENT;
  }

  /* also, create a second page table, which will 'point' back at the page
   * directory */
  uint32_t *pdir_table = page_table + PAGE_SIZE;
  addr = (uint32_t)page_directory;

  for (int i = 0; i < 1024; i++, addr += PAGE_SIZE){
    /* attributes: supervisor level, read + write, present */
    pdir_table[i] = addr | PTAB_ATTR_RDWR | PTAB_ATTR_PRESENT;
  }

  /* put the page table in the page directory */
  page_directory[0]  = (uint32_t)page_table;
  /* set the page table #0 to be present */
  page_directory[0] |= PDIR_ATTR_PRESENT;
  /* put the page table in the page directory (in the last slot) */
  page_directory[1023]  = (uint32_t)pdir_table;
  /* set the page table #1023 to be present */
  page_directory[1023] |= PDIR_ATTR_PRESENT;

  /* actually enable paging */
  uint32_t cr0;

  __asm volatile("mov %0, %%cr3" : : "b"(page_directory));
  __asm volatile("mov %%cr0, %0" : "=b"(cr0));
  cr0 |= 0x80000000;
  __asm volatile("mov %0, %%cr0" : : "b"(cr0));

  return page_directory;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

