/*
 *
 * kernel.c
 *
 * Created at:  Fri 28 Mar 13:20:40 2014 13:20:40
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "paging.h"
#include "idt.h"
#include "kbd.h"
#include "vga.h"
#include "mm.h"
#include "timer.h"

#ifndef __i386__
#error "the only supported architecture is i386"
#endif

size_t strlen(const char *s)
{
  size_t ret = 0;

  while (s[ret++] != '\0')
    ;

  return ret;
}

void *memset(void *dst, int ch, size_t len)
{
  while (len-- != 0)
    *(uint8_t *)dst++ = (unsigned char)ch;

  return dst;
}

void *memcpy(void *dst, void *src, size_t len)
{
  void *ret = dst;

  while (len-- != 0)
    *(uint8_t *)dst++ = *(uint8_t *)src++;

  return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

static void adjust_the_memory_map(struct kern_bootinfo *bootinfo)
{
  /* {{{ */
  /* add a (reserved) memory entry for kernel's guts */
  struct memory_map_entry kernentry = {
    .base_low  = &kernel_phys,
    .base_high = 0x0,
    /* make room for the paging stuff */
    .len_low   = (uint32_t)&kernel_size + KiB(4) + MiB(4),
    .len_high  = 0x0,
    .type      = 2,
    .acpi_ext  = 0
  };

  /* the 61 is intentional (64 - 3 since at most 3 entries may be added) */
  for (int i = 0; i < 61; i++){
    struct memory_map_entry e = bootinfo->memory_map[i];

    /* see if the kernel's is going to fit in that (available) entry */
    if (e.type == 1){
      if (kernentry.base_low >= e.base_low){
        if (e.len_low >= kernentry.len_low + (kernentry.base_low - e.base_low)){
          struct memory_map_entry pre = e;
          struct memory_map_entry pre_align = e;

          pre.len_low = kernentry.base_low - e.base_low;
          pre.len_high = 0x0;

          pre_align.base_low = pre.base_low + pre.len_low;
          pre_align.base_high = 0x0;
          pre_align.len_low = e.len_low - pre.len_low;
          pre_align.len_high = 0x0;

          /* we might need to create an available entry preceding the kernel's */
          if (kernentry.base_low - e.base_low > 0){
            int k;

            for (k = 62; k > i; k--)
              bootinfo->memory_map[k + 1] = bootinfo->memory_map[k];

            bootinfo->memory_map[k] = pre;
            bootinfo->memory_map[k + 1] = pre_align;
            i++;
          }

          bootinfo->memory_map[i] = kernentry;

          /* we might need to create an (available) entry following the kernel's */
          if (e.len_low - kernentry.len_low - (kernentry.base_low - e.base_low) > 0){
            struct memory_map_entry post = {
              .base_low  = kernentry.base_low + kernentry.len_low,
              .base_high = 0x0,
              .len_low   = (e.len_low + e.base_low) - (kernentry.base_low + kernentry.len_low),
              .len_high  = 0x0,
              .type      = 1,
              .acpi_ext  = 0
            };

            int k;

            for (k = 62; k > i; k--)
              bootinfo->memory_map[k + 1] = bootinfo->memory_map[k];

            bootinfo->memory_map[k + 1] = post;
          }
        }
      }
    }
  }

  /* merge any adjacent entries of the same type */
  for (int i = 0; i < 64; i++){
    struct memory_map_entry *e = &bootinfo->memory_map[i];

    if ((e->len_low | e->len_high) == 0) continue;

    if ((e + 1)->type == e->type){
      int j = 1;

      while ((e + j)->type == e->type){
        e->len_low += (e + j)->len_low;
        j++;
      }

      j--;

      for (int k = i + 1; k < 64 - i - 1; k++){
        struct memory_map_entry *p = &bootinfo->memory_map[k];

        *p = *(p + j);
      }
    }
  }

  /* calculate the (total) available memory */
  for (int i = 0; i < 64; i++){
    struct memory_map_entry *e = &bootinfo->memory_map[i];

    if (e->type == 1 || e->type == 3)
      bootinfo->mem_avail += e->len_low;
  }
  /* }}} */
}

void kmain(struct kern_bootinfo *bootinfo)
{
  adjust_the_memory_map(bootinfo);
  /* set up the printing utilities */
  vga_init();
  /* set up paging */
  uint32_t *pdir_addr = paging_init(bootinfo);
  /* install the IDT (ISRs and IRQs) */
  idt_install();
  /* install the keyboard */
  kbd_install();
  /* install the timer */
  timer_install();
  /* initialize the memory management */
  /*uint32_t heap_addr = mm_init(bootinfo);*/
  uint32_t *heap_addr = NULL;

  __asm volatile("sti");

  vga_puts("\n Figh\n\n");
  vga_puts(" Tha mo bhata-foluaimein loma-lan easgannan\n");
  vga_puts(" ------------------------------------------\n\n");
  vga_printf(" kernel's physical address: 0x%x\n", &kernel_phys);
  vga_printf(" kernel's  virtual address: 0x%x\n", &kernel_start);
  vga_printf(" kernel's size:             0x%x\n", &kernel_size);
  vga_printf(" heap created:              0x%x\n", heap_addr);
  vga_printf(" page directory created:    0x%x\n", pdir_addr);
  vga_printf(" available memory detected: 0x%x (%d MiB)\n\n", bootinfo->mem_avail, bootinfo->mem_avail / 1024 / 1024);
  vga_printf(" memory map:\n");
  vga_printf(" base address         length              type\n");
  vga_printf(" ---------------------------------------------\n");

  for (int i = 0; i < 64; i++){
    struct memory_map_entry *e = &bootinfo->memory_map[i];
    if ((e->len_low | e->len_high) == 0) continue;
    vga_printf(" 0x%x%x - 0x%x%x     %d\n", e->base_high, e->base_low, e->len_high, e->len_low, e->type);
  }

  for (;;);
}

#ifdef __cplusplus
}
#endif

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

