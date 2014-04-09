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

#ifndef __cplusplus
# include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>

#include "idt.h"

#ifdef __linux__
#error "you are not using a cross-compiler!"
#endif

#ifndef __i386__
#error "the only supported architecture is i386"
#endif

#define VGA_BUFF 0xb8000

static const size_t VGA_WIDTH    = 80;
static const size_t VGA_HEIGHT   = 24;

size_t term_col, term_row;
uint8_t term_color;

enum vga_color {
  COLOR_BLACK = 0,
  COLOR_BLUE,
  COLOR_GREEN,
  COLOR_CYAN,
  COLOR_RED,
  COLOR_MAGENTA,
  COLOR_BROWN,
  COLOR_LIGHT_GREY,
  COLOR_DARK_GREY,
  COLOR_LIGHT_BLUE,
  COLOR_LIGHT_GREEN,
  COLOR_LIGHT_CYAN,
  COLOR_LIGHT_RED,
  COLOR_LIGHT_MAGENTA,
  COLOR_LIGHT_BROWN,
  COLOR_WHITE
};

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

  *((uint16_t *)VGA_BUFF + idx) = make_char(ch, color);
}

void term_putch(char ch)
{
  term_putchat(ch, term_color, term_col, term_row);

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

  for (; *p != '\0';){
    term_putch(*p++);
  }
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
  term_puts(" Quidquid Latine dictum, sit altum videtur");

  for (;;);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

