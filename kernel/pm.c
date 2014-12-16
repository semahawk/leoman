/*
 *
 * pm.c
 *
 * Created at:  Tue 16 Dec 16:05:19 2014 16:05:19
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include "common.h"
#include "vm.h"

/* the byte map */
static  uint8_t *page_bmap;
static uint32_t  page_bmap_num;
static uint32_t  page_bmap_size;
static  uint8_t *page_bmap_end;

void *palloc(void)
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

void pfree(void *addr)
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
void *pm_init(struct kern_bootinfo *bootinfo)
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

  return (void *)v2p(page_bmap);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

