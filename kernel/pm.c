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

#include <kernel/common.h>
#include <kernel/pm.h>
#include <kernel/print.h>
#include <kernel/vm.h>

/* the bitmap */
static uint32_t pm_bitmap[PM_BITMAP_NMEMB];
/* the physical memory manager will start handing out pages starting from this
 * address */
static uint32_t *pm_page_pool;

void *pm_alloc(void)
{
  void *page;

  /* traverse the bitmap */
  for (unsigned idx = 0; idx < sizeof(pm_bitmap) / sizeof(*pm_bitmap); idx++){
    /* if it's not all ones then at least one bit is zero */
    if (pm_bitmap[idx] != (uint32_t)-1){
      for (unsigned bit = 0; /* no guard */; bit++){
        if (!(pm_bitmap[idx] & (1 << bit))){
          /* calculate the page's address */
          page = (void *)((uint32_t)pm_page_pool + ((uint32_t)((idx * sizeof(*pm_bitmap) * 8) + bit) * PAGE_SIZE));
          /* set the bit - mark the page used */
          pm_bitmap[idx] |= 1 << bit;
          break;
        }
      }

      return (void *)v2p(page);
    }
  }

  return NULL;
}

void *pm_alloc_cont(size_t page_num)
{
  /* currently the limit is the number of bits in a word */
  /* TODO: add support for cross-index bitmask finding */
  /* ugh, what I mean is that if pm_bitmap[n] has 3 right-most bits zeroed, and
   * pm_bitmap[n+1] has 3 left-most bits zeroed, we should be able to allocate 6
   * continuous pages there */
  size_t limit = sizeof(pm_bitmap[0]) * 8;
  void *page;

  if (page_num > limit){
    kprintf("[pm] WARNING: allocating more than %d contiguous pages is not supported!\n", limit);
    return NULL;
  }

  /* traverse the bitmap */
  for (unsigned idx = 0; idx < sizeof(pm_bitmap) / sizeof(*pm_bitmap); idx++){
    /* if it's not all ones then at least one bit is zero */
    if (pm_bitmap[idx] != (uint32_t)-1){
      /* find 'page_num' free bits in pm_bitmap[idx] */
      uint32_t mask = (uint32_t)-1 >> (limit - page_num);
      uint32_t shift = 0;

      for (unsigned _i = 0; _i <= limit - page_num; _i++, mask <<= 1, shift++){
        /* we did find 'page_num' free bits */
        if (0 == (pm_bitmap[idx] & mask)){
          /* calculate the page's address */
          page = (void *)((uint32_t)pm_page_pool + ((uint32_t)((idx * sizeof(*pm_bitmap) * 8) + shift) * PAGE_SIZE));
          /* set the bit - mark the page used */
          pm_bitmap[idx] |= mask;

          return (void *)v2p(page);
        }
      }
    }
  }

  return NULL;
}

void pm_free(void *addr)
{
  addr = PALIGNDOWN(addr);

  size_t idx = ((uint32_t)addr - (uint32_t)pm_page_pool) / PAGE_SIZE / (sizeof(*pm_bitmap) * 8);
  size_t bit = ((uint32_t)addr - (uint32_t)pm_page_pool) / PAGE_SIZE % (sizeof(*pm_bitmap) * 8);

  /* unset the bit - mark the page free */
  pm_bitmap[idx] &= ~(1 << bit);
}

/*
 * the physical memory manager will use a bitmap
 * the pmm will start handing out pages at the beginning of the biggest free memory hole
 *
 * assumming 4GiB of available memory, the bitmap would be only 128KiB, which is
 * fairly nice
 */
void *pm_init(struct kern_bootinfo *bootinfo)
{
  uint32_t max_size = 0;

  /* find the biggest available memory area */
  for (int i = 0; i < 16; i++){
    if (bootinfo->memory_map[i].len_low >= max_size){
      max_size = bootinfo->memory_map[i].len_low;
      pm_page_pool = (uint32_t *)PALIGNUP(p2v((uint32_t)bootinfo->memory_map[i].base_low));
    }
  }

  /* zero-out the bitmap */
  memset(pm_bitmap, 0x0, sizeof(pm_bitmap));

  kprintf("[pm] physical memory manager initialized\n");

  return (void *)v2p(pm_page_pool);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

