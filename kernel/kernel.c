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

void kmain(uint32_t kernels_end)
{
  /* set up the printing utilities */
  vga_init();
  /* install the IDT (ISRs and IRQs) */
  idt_install();
  /* install the keyboard */
  kbd_install();
  /* initialize the memory management */
  mm_init(kernels_end);

  asm volatile("sti");

  vga_putchat('N', COLOR_WHITE, 3, 1);
  vga_putchat('m', COLOR_DARK_GREY, 4, 1);

  vga_row += 3;
  vga_puts(" Quidquid Latine dictum, sit altum videtur\n\n");

  vga_printf("printf says: hello, world %% 42, decimal %d, %s %x\n\n", 1234, "hex", 0xfeedbeef);

  void *one = kmalloc(7);
  void *two = kmalloc(2);
  void *three = kmalloc(3);

  vga_printf("kmalloc(1) says: %x\n", one);
  vga_printf("kmalloc(2) says: %x\n", two);
  vga_printf("kmalloc(3) says: %x\n", three);

  kfree(two);

  void *four = kmalloc(2);

  vga_printf("kmalloc(4) says: %x\n", four);

  if (four == two){
    /* see if the `four' was placed in the `two's spot, since `two' was freed
     * and it occupied the exact amount of memory `four' needs */
    vga_printf("nice :)\n");
    /* I'm not sure if this is exactly nice, but, still.. */
  }

  for (;;);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

