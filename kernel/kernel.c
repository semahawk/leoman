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
#include "idt.h"
#include "kbd.h"
#include "vga.h"
#include "mm.h"

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
extern "C"
#endif

void kmain(struct kern_bootinfo *bootinfo)
{
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

  for (int i = 0; i < 64; i++){
    struct memory_map_entry *e = &bootinfo->memory_map[i];

    if (e->type == 1 || e->type == 3)
      /* FIXME handle len_high */
      bootinfo->mem_avail += e->len_low;
  }

  /* set up the printing utilities */
  vga_init();
  /* install the IDT (ISRs and IRQs) */
  idt_install();
  /* install the keyboard */
  kbd_install();
  /* initialize the memory management */
  uint32_t heap_addr = mm_init(bootinfo);

  asm volatile("sti");

  vga_puts("\n Gorm\n\n");
  vga_puts(" Tha mo bhata-foluaimein loma-lan easgannan\n");
  vga_puts(" ------------------------------------------\n\n");
  vga_printf(" kernel's address:          0x%x\n", bootinfo->kernel_addr);
  vga_printf(" heap created:              0x%x\n", heap_addr);
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

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

