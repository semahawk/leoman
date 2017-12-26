/*
 *
 * heap.c
 *
 * Created at:  07 Nov 2017 20:17:35 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/common.h>
#include <kernel/heap.h>
#include <kernel/pm.h>
#include <kernel/print.h>
#include <kernel/vm.h>
#include <kernel/x86.h>

#define BLOCK_UNUSED 0x00
#define BLOCK_USED   0xff

/* an allocation 'unit', let's call it 'block' */
struct block {
  uint16_t magic;
  uint8_t type;
  struct block *next;
  uint32_t data_size;
};

static bool heap_initialized = false;
static void *const heap_pool_addr = (void *)0xe4000000;

void *kalloc(size_t bytes)
{
  if (!heap_initialized){
    kprintf("[heap] the kernel heap was not initialized yet - cannot allocate %d bytes!\n", bytes);
    return NULL;
  }

  if (0 == bytes){
    kprintf("[heap] WARNING: requesting 0 bytes of data!\n");
    return NULL;
  }

  struct block *block;

  for (block = heap_pool_addr; block != NULL; block = block->next){
    if (block->type == BLOCK_UNUSED && (block->data_size + sizeof(struct block)) >= bytes){
      if (block->next == NULL){
        struct block *new_block = (void *)((uintptr_t)block + sizeof(struct block) + bytes);

        /* copy the meta-data from the 'current' block to the new one */
        memcpy(new_block, block, sizeof(struct block));
        /* decrease by the number of requested bytes */
        new_block->data_size -= bytes + sizeof(struct block);

        block->next = new_block;
      }

      block->data_size = bytes;
      block->type = BLOCK_USED;

      return (void *)((uintptr_t)block + sizeof(struct block));
    }
  }

  return NULL;
}

void kfree(void *addr)
{
  if (!heap_initialized)
    return;

  if (NULL == addr)
    return;

  struct block *block = (void *)((uintptr_t)addr - sizeof(struct block));

  /* FIXME: let's go the easy way and just mark it unused */
  /* in future we should do some merging with adjacent blocks &c. */
  block->type = BLOCK_UNUSED;
}

void heap_list_blocks(void)
{
  if (!heap_initialized){
    kprintf("[heap] %s: the heap is not initialized yet!\n", __func__);
    return;
  }

  kprintf("[heap] ## dumping the blocks:\n");
  for (struct block *block = heap_pool_addr;
       block != NULL;
       block = block->next){
    kprintf("[heap] -- type: %d, data_size: %d\n", block->type, block->data_size);
  }
}

void heap_init(void)
{
  void *heap_pool_phys_addr;

  kprintf("[heap] initializing heap (%d B)\n", CONFIG_KERNEL_HEAP_SIZE);

  if (NULL == (heap_pool_phys_addr = pm_alloc_cont(CONFIG_KERNEL_HEAP_SIZE / PAGE_SIZE))){
    kprintf("[heap] -- failed - can't proceed!\n");
    halt();
  }

  map_pages(heap_pool_phys_addr, heap_pool_addr, 0, CONFIG_KERNEL_HEAP_SIZE);

  /* let's create the first block which will span the entire heap, and will be
   * marked unused */
  struct block *initial = heap_pool_addr;

  initial->magic = 0xbabe;
  initial->type = BLOCK_UNUSED;
  /* blerh */
  initial->data_size = CONFIG_KERNEL_HEAP_SIZE - (sizeof(*initial) - sizeof(initial->data_size));
  initial->next = NULL;

  heap_initialized = true;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

