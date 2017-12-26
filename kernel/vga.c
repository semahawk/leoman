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
#include <kernel/print.h>
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
 * printf-eque function to print to VGA screen
 */
void vga_printf(const char *fmt, ...)
{
  char buf[KERNEL_MAX_PRINT_SIZE];
  va_list vl;

  va_start(vl, fmt);
  kvformat(buf, sizeof(buf), fmt, vl);
  vga_puts(buf);
  va_end(vl);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

