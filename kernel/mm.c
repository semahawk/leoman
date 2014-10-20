/*
 *
 * mm.c
 *
 * Created at:  Mon 14 Apr 19:36:28 2014 19:36:28
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include "common.h"
#include "vga.h"
#include "mm.h"

static struct memblock *blocks; /* do we need a 'tail'? */

void mm_init(uint32_t start)
{
  struct memblock *initial = (struct memblock *)start;

  initial->prev = NULL;
  initial->next = NULL;
  /* TODO FIXME XXX boy do I need a memory map..
   *
   * see how much memory we have in total
   * and use it here (delta kernel code &c.) */
  initial->size = 3096;
  initial->used = MM_UNUSED;

  blocks = initial;
}

void *kmalloc(size_t size)
{
  /* traverse the memory blocks searching for a sufficiently large free block */
  struct memblock *block;

  for (block = blocks; block != NULL; block = block->next){
    if (block->used == MM_UNUSED){
      /* we found a free block */
      /* let's see if it's big enough */
      if (block->size >= size){
        /* yay */

        /* let's see if there is a space for a new block header */
        if (block->size - size > sizeof(struct memblock)){
          /* create a new block, if there is a space for it */
          struct memblock *new = (struct memblock *)((size_t)block + (sizeof(struct memblock) + size));

          new->prev = block;
          new->next = block->next;
          new->size = block->size - sizeof(struct memblock) - size;
          new->used = MM_UNUSED;
          block->next = new;
          block->size = size;
        }

        block->used = MM_USED;

        /* return the data right next to the 'found' block header */
        return (void *)((size_t)block + sizeof(struct memblock));
      }
    }
  }

  /* that's sad */
  return NULL;
}

void kfree(void *ptr)
{
  struct memblock *p;
  struct memblock *block = (struct memblock *)((size_t)ptr - sizeof(struct memblock));

  block->used = MM_UNUSED;

  /* merge free blocks to the right of the one being freed */
  for (p = block->next; p != NULL && !p->used; p = p->next){
    p->prev->next = p->next;
    p->prev->size += p->size + sizeof(struct memblock);
  }

  /* merge free blocks to the left of the one being freed */
  for (p = block->prev; p != NULL && !p->used; p = p->prev){
    p->next = p->next->next;
    p->size += p->size + sizeof(struct memblock);
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

