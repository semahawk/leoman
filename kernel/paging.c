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
static uint32_t *page_tables_end;
/* the byte map */
static  uint8_t *page_bmap;
static uint32_t  page_bmap_num;
static uint32_t  page_bmap_size;
static  uint8_t *page_bmap_end;

/*
 * Map a single page
 */
void map_page(void *paddr, void *vaddr, unsigned flags)
{
  paddr = PALIGNDOWN(paddr);
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx = (uint32_t)vaddr >> 22;
  uint32_t ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

  uint32_t *pdir = page_directory;
  uint32_t *ptab = page_directory_end + pdir_idx * KiB(1);

  pdir[pdir_idx] |= PDE_P;
  ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTE_P;
}

/*
 * Unmap a single page
 */
void unmap_page(void *vaddr)
{
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx = (uint32_t)vaddr >> 22;
  uint32_t ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

  uint32_t *ptab = page_directory_end + pdir_idx * KiB(1);

  ptab[ptab_idx] = 0x0;
}

/*
 * Map <sz> memory of continuous pages
 */
void map_pages(void *paddr, void *vaddr, unsigned flags, unsigned sz)
{
  if (sz == 0)
    return;

  paddr = PALIGNDOWN(paddr);
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx;
  uint32_t ptab_idx;

  uint32_t *pdir = page_directory;
  uint32_t *ptab;

  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    pdir_idx = (uint32_t)vaddr >> 22;
    ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

    ptab = page_directory_end + pdir_idx * KiB(1);

    pdir[pdir_idx] |= PDE_P;
    ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTE_P;

    paddr += PAGE_SIZE;
    vaddr += PAGE_SIZE;
  }
}

/*
 * Unmap <sz> memory of continuous pages
 */
void unmap_pages(void *vaddr, unsigned sz)
{
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx;
  uint32_t ptab_idx;

  uint32_t *pdir = page_directory;
  uint32_t *ptab;

  unsigned npages = sz / PAGE_SIZE;

  for (int i = 0; i < npages; i++){
    pdir_idx = (uint32_t)vaddr >> 22;
    ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

    ptab = page_directory_end + pdir_idx * KiB(1);
    ptab[ptab_idx] = 0x0;
    vaddr += PAGE_SIZE;
  }
}

void *kalloc(void)
{
  void *page;
  int i;

  /* traverse the byte map */
  for (i = 0; i < page_bmap_num; i++){
    /* see which byte is 'free', indicating a free, unused page */
    if (page_bmap[i] == 0){
      /* found a free page */
      /* calculate it's address */
      page = page_bmap_end + i * PAGE_SIZE;
      /* map it */
      map_page((void *)v2p(page), page, PTE_W);
      /* mark it 'used' */
      page_bmap[i] = 1;

      return page;
    }
  }

  return NULL;
}

void kfree(void *addr)
{
  /* page's index in `page_bmap' */
  size_t idx = ((uint32_t)addr - (uint32_t)page_bmap_end) / PAGE_SIZE;

  unmap_page(addr);
  /* mark it 'free' / 'unused' */
  page_bmap[idx] = 0;
}

/*
 * the virtual memory manager will use a bytemap
 * the bytemap will be stored at the beginning of the biggest free memory hole
 * it's size is dependent on the available memory
 *
 * assumming 4GiB of available memory, the bytemap would be only 1MiB, which is
 * nice
 */
void *kvm_init(struct kern_bootinfo *bootinfo)
{
  uint32_t max_size = 0;

  /* find the biggest available memory area */
  for (int i = 0; i < 64; i++){
    if (bootinfo->memory_map[i].len_low >= max_size){
      max_size = bootinfo->memory_map[i].len_low;
      page_bmap = (uint8_t *)PALIGNUP(p2v((uint32_t)bootinfo->memory_map[i].base_low));
    }
  }

  page_bmap_num = max_size / PAGE_SIZE;
  page_bmap_size = (uint32_t)PALIGNUP(page_bmap_num);
  page_bmap_end = page_bmap + page_bmap_size;

  map_pages((void *)v2p(page_bmap), page_bmap, PTE_W, page_bmap_size);
  /* zero-out the bytemap */
  memset(page_bmap, 0x0, page_bmap_size);

  return (void *)page_bmap;
}

void *paging_init(struct kern_bootinfo *bootinfo)
{
  /* the page directory is right after the kernel */
  page_directory = PALIGNUP((uint32_t)&kernel_start + (uint32_t)&kernel_size);
  /* the page tables are right after the page directory */
  /* so yeah, whole 4MiB+4KiB are reserved for kernel's paging stuff */
  page_directory_end = page_directory + KiB(4);
  page_tables_end = page_directory_end + MiB(4);

  /* zero-out the page directory and the page tables */
  memset(page_directory, 0x0, KiB(4) + MiB(4));

  for (int i = 0; i < 1024; i++)
    page_directory[i] = v2p(page_directory_end + i * KiB(1)) | PDE_W;

  /* identity map the first 1 MiB of memory */
  map_pages(0x0, 0x0, PTE_W, MiB(1));
  /* map the kernel intestines to the higher half */
  map_pages(0x0, &kernel_off, PTE_W, ((uint32_t)&kernel_start - (uint32_t)&kernel_off) + ((uint32_t)&kernel_size) + MiB(4) + KiB(4));

  set_cr3(v2p(page_directory));

  return (void *)v2p(page_directory);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

