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

#ifdef __linux__
#error "you are not using a cross-compiler!"
#endif

#ifndef __i386__
#error "the only supported architecture is i386"
#endif

const size_t VGA_WIDTH  = 80;
const size_t VGA_HEIGHT = 24;
static uint16_t *const VGA_MEM = (uint16_t *)0xb8000;

size_t term_col, term_row;
uint8_t term_color;

size_t strlen(const char *s)
{
  size_t ret = 0;

  while (s[ret++] != '\0')
    ;

  return ret;
}

uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
  return fg | bg << 4;
}

uint16_t make_char(char ch, uint8_t color)
{
  uint8_t newc = ch;
  uint8_t newcol = color;

  return newc | newcol << 8;
}

void term_putchat(char ch, uint8_t color, size_t x, size_t y)
{
  const size_t idx = y * VGA_WIDTH + x;

  *(VGA_MEM + idx) = make_char(ch, color);
}

void term_putch(char ch)
{
  switch (ch){
    case 0xa: /* newline */
      term_row++;
      term_col = 0;
      break;
    default:
      term_putchat(ch, term_color, term_col, term_row);
      break;
  }

  if (++term_col == VGA_WIDTH){
    term_col = 0;

    if (++term_row == VGA_HEIGHT){
      term_row = 0;
    }
  }
}

void term_init(void)
{
  size_t x, y;

  term_row = term_col = 0;
  term_color = make_color(COLOR_WHITE, COLOR_BLACK);

  for (y = 0; y < VGA_HEIGHT; y++){
    for (x = 0; x < VGA_WIDTH; x++){
      term_putchat(' ', term_color, x, y);
    }
  }
}

void term_puts(const char *s)
{
  char *p = s;

  while (*p != '\0')
    term_putch(*p++);
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
  term_init();
  term_putchat('N', COLOR_WHITE, 3, 1);
  term_putchat('m', COLOR_DARK_GREY, 4, 1);

  term_row += 3;
  term_puts(" Quidquid Latine dictum, sit altum videtur\n");
  term_row += 2;

  /* install the IDT */
  idt_install();

  /* let's test it ;) */
  asm("int $0");

  for (;;);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

