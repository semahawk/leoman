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
#include <stdarg.h>

#include <kernel/common.h>
#include <kernel/vga.h>

const size_t VGA_WIDTH  = 80;
const size_t VGA_HEIGHT = 25;
static uint16_t *const VGA_MEM = (uint16_t *)0xb8000;

size_t vga_col, vga_row;
uint8_t vga_color;

void vga_scrollup(void)
{
  for (unsigned y = 1; y < VGA_HEIGHT; y++){
    for (unsigned x = 0; x < VGA_WIDTH; x++){
      *(VGA_MEM + ((y - 1) * VGA_WIDTH + x)) = *(VGA_MEM + (y * VGA_WIDTH + x));
    }
  }

  for (unsigned x = 0; x < VGA_WIDTH; x++){
    *(VGA_MEM + ((VGA_HEIGHT - 1) * VGA_WIDTH + x)) = ' ';
  }
}

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
  if (vga_col >= VGA_WIDTH || ch == '\n'){
    vga_col = 0;
    vga_row++;
  }

  if (vga_row >= VGA_HEIGHT){
    vga_scrollup();
    vga_row--;
  }

  if (ch == '\n')
    return;

  vga_putchat(ch, vga_color, vga_col, vga_row);
  vga_col++;
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
  int out[10 /* ceil(log10(2^32)) */] = { 0 };
  int i = 0;

  if (v == 0){
    vga_putch('0');

    return;
  }

  if (v < 0){
    v = -v;
    vga_putch('-');
  }

  for (; v > 0; v /= 10)
    out[i++] = v % 10;

  i--;

  while (i >= 0)
    vga_put_digit(out[i--]);
}

/*
 * Print the given value <v> as a hexadecimal double word
 */
void vga_puthd(uint32_t v)
{
  uint32_t mask = (uint32_t)0xf0000000;
  uint8_t i = 8; /* number of digits in a dword */

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

  for (; mask > 0; mask >>= 4, i--)
    vga_put_digit((mask & v) >> (i * 4 - 4));
}

/*
 * printf-eque function to print to VGA screen
 */
void vga_printf(const char *fmt, ...)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
  /* meh, this is quite ugly */
  char *p = fmt;
#pragma clang diagnostic pop
  va_list vl;

  va_start(vl, fmt);

  for (; *p != '\0'; p++){
    if (*p == '%'){
      p++;
      switch (*p){
        case '%':
          vga_putch('%');
          break;
        case 'c':
          vga_putch(va_arg(vl, char));
          break;
        case 'd':
          vga_putd(va_arg(vl, int));
          break;
        case 'x':
          vga_puthd(va_arg(vl, uint32_t));
          break;
        case 's':
          vga_puts(va_arg(vl, const char *));
          break;
      }
    } else {
      vga_putch(*p);
    }
  }

  va_end(vl);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

