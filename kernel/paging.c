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

uint32_t *paging_init(struct kern_bootinfo *bootinfo)
{
  uint32_t kernel_end = bootinfo->kernel_addr + bootinfo->kernel_size;
  /* page align the kernel's end address */
  kernel_end = (kernel_end & (0xffffffff - (PAGE_SIZE - 1))) + PAGE_SIZE;
  page_directory = (uint32_t *)kernel_end;

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

  /* put the page table in the page directory */
  page_directory[0]  = (uint32_t)page_table;
  /* set the page table #0 to be present */
  page_directory[0] |= PDIR_ATTR_PRESENT;

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

