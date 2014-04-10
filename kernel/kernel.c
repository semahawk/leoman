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
#include "vga.h"

#ifdef __linux__
#error "you are not using a cross-compiler!"
#endif

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

static inline uint8_t inb(uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(uint16_t port, uint8_t data)
{
  asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

#ifdef __cplusplus
extern "C"
#endif

int kmain(void)
{
  vga_init();
  vga_putchat('N', COLOR_WHITE, 3, 1);
  vga_putchat('m', COLOR_DARK_GREY, 4, 1);

  vga_row += 3;
  vga_puts(" Quidquid Latine dictum, sit altum videtur\n");
  vga_row += 2;

  /* install the IDT */
  idt_install();

  /* let's test it ;) */
  asm("int $0");

  for (;;);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

