/*
 *
 * vga.c
 *
 * Created at:  Thu 10 Apr 18:55:40 2014 18:55:40
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "vga.h"

const size_t VGA_WIDTH  = 80;
const size_t VGA_HEIGHT = 24;
static uint16_t *const VGA_MEM = (uint16_t *)0xb8000;

size_t vga_col, vga_row;
uint8_t vga_color;

uint8_t vga_make_color(enum vga_color fg, enum vga_color bg)
{
  return fg | bg << 4;
}

uint16_t vga_make_char(char ch, uint8_t color)
{
  uint8_t newc = ch;
  uint8_t newcol = color;

  return newc | newcol << 8;
}

void vga_putchat(char ch, uint8_t color, size_t x, size_t y)
{
  const size_t idx = y * VGA_WIDTH + x;

  *(VGA_MEM + idx) = vga_make_char(ch, color);
}

void vga_putch(char ch)
{
  switch (ch){
    case 0xa: /* newline */
      vga_row++;
      vga_col = 0;
      break;
    default:
      vga_putchat(ch, vga_color, vga_col, vga_row);
      break;
  }

  if (++vga_col == VGA_WIDTH){
    vga_col = 0;

    if (++vga_row == VGA_HEIGHT){
      vga_row = 0;
    }
  }
}

void vga_init(void)
{
  size_t x, y;

  vga_row = vga_col = 0;
  vga_color = vga_make_color(COLOR_WHITE, COLOR_BLACK);

  for (y = 0; y < VGA_HEIGHT; y++){
    for (x = 0; x < VGA_WIDTH; x++){
      vga_putchat(' ', vga_color, x, y);
    }
  }
}

void vga_puts(const char *s)
{
  while (*s != '\0')
    vga_putch(*s++);
}

/*
 * "Print" a newline
 */
void vga_putnl(void)
{
  vga_row += 1;
  vga_col = 0;
}

/*
 * Prints a given character <ch> as either an alphanumeric character (0-9) or
 * a lowercase hexadecimal (10-15)
 */
static inline void vga_put_digit(uint8_t ch)
{
  static char digits[] = "0123456789abcdef";

  vga_putch(digits[ch % 16]);
}

/*
 * Print a given value <v> as a decimal integer.
 * NOTE: It's quite buggy with big(ger) numbers.
 */
void vga_putd(int v)
{
  int mask /* meh, naming things... */ = 1000000000;

  if (v < 0){
    v = -v;
    vga_putch('-');
  }

  for (; mask > 0; mask /= 10){
    if (v > mask - 1){
      vga_put_digit(v / mask);
      v %= mask;
    }
  }
}

/*
 * Print the given value <v> as a hexadecimal double word
 */
void vga_puthd(uint32_t v)
{
  uint32_t mask = (uint32_t)0xf0000000;
  uint8_t i = 8; /* number of digits in a dword */

  vga_puts("0x");
  for (; mask > 0; mask >>= 4, i--)
    vga_put_digit((mask & v) >> (i * 4 - 4));
}

/*
 * Print the given value <v> as a hexadecimal word
 */
void vga_puthw(uint16_t v)
{
  uint16_t mask = (uint16_t)0xf000;
  uint8_t i = 4; /* number of digits in a word */

  vga_puts("0x");
  for (; mask > 0; mask >>= 4, i--)
    vga_put_digit((mask & v) >> (i * 4 - 4));
}

/*
 * Print the given value <v> as a hexadecimal byte
 */
void vga_puthb(uint8_t v)
{
  uint8_t mask = (uint8_t)0xf0;
  uint8_t i = 2; /* number of digits in a byte */

  vga_puts("0x");
  for (; mask > 0; mask >>= 4, i--)
    vga_put_digit((mask & v) >> (i * 4 - 4));
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

