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

static uint32_t *page_directory;
static uint32_t *page_directory_end;

/*
 * Map a single page
 */
void map_page(void *paddr, void *vaddr, unsigned flags)
{
  paddr = PALIGNUP(paddr);
  vaddr = PALIGNUP(vaddr);

  uint32_t pdir_idx = (uint32_t)vaddr >> 22;
  uint32_t ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

  uint32_t *pdir = page_directory;
  uint32_t *ptab = page_directory_end + pdir_idx * KiB(1);

  pdir[pdir_idx] |= PDIR_ATTR_PRESENT;
  ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTAB_ATTR_PRESENT;
}

/*
 * Map <sz> memory of continuous pages
 */
void map_pages(void *paddr, void *vaddr, unsigned flags, unsigned sz)
{
  paddr = PALIGNUP(paddr);
  vaddr = PALIGNUP(vaddr);

  uint32_t pdir_idx;
  uint32_t ptab_idx;

  uint32_t *pdir = page_directory;
  uint32_t *ptab;

  unsigned npages = sz / PAGE_SIZE;

  for (int i = 0; i < npages; i++){
    pdir_idx = (uint32_t)vaddr >> 22;
    ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

    ptab = page_directory_end + pdir_idx * KiB(1);

    pdir[pdir_idx] |= PDIR_ATTR_PRESENT;
    ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTAB_ATTR_PRESENT;

    paddr += PAGE_SIZE;
    vaddr += PAGE_SIZE;
  }
}

uint32_t *paging_init(struct kern_bootinfo *bootinfo)
{
  /* the page directory is right after the kernel */
  page_directory = PALIGNUP((uint32_t)&kernel_start + (uint32_t)&kernel_size);
  /* the page tables are right after the page directory */
  /* so yeah, whole 4MiB+4KiB are reserved for kernel's paging stuff */
  page_directory_end = page_directory + KiB(4);

  /* zero-out the page directory and the page tables */
  memset(page_directory, 0x0, KiB(4) + MiB(4));

  for (int i = 0; i < 1024; i++)
    page_directory[i] = v2p(page_directory_end + i * KiB(1)) | PDIR_ATTR_RDWR;

  /* identity map the first 1 MiB of memory */
  map_pages(0x0, 0x0, PTAB_ATTR_RDWR, MiB(1));
  /* map the kernel intestines to the higher half */
  map_pages(0x0, &kernel_off, PTAB_ATTR_RDWR, ((uint32_t)&kernel_start - (uint32_t)&kernel_off) + ((uint32_t)&kernel_size) + MiB(4) + KiB(4));

  set_cr3(v2p(page_directory));

  return (uint32_t *)v2p(page_directory);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

