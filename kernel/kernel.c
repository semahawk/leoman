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

#ifdef __linux__
#error "you are not using a cross-compiler!"
#endif

#ifndef __i386__
#error "the only supported architecture is i386"
#endif

static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 24;

size_t term_row;
size_t term_col;
uint8_t term_color;
uint16_t *term_buff;

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

uint16_t make_vga_entry(char ch, uint8_t color)
{
  uint16_t ch16 = ch;
  uint16_t color16 = color;

  return ch16 | color16 << 8;
}

void term_setcolor(uint8_t color)
{
  term_color = color;
}

void term_putchat(char ch, uint8_t color, size_t x, size_t y)
{
  const size_t idx = y * VGA_WIDTH + x;

  term_buff[idx] = make_vga_entry(ch, color);
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
  term_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
  term_buff = (uint16_t *)0xb8000;

  for (y = 0; y < VGA_HEIGHT; y++)
    for (x = 0; x < VGA_WIDTH; x++)
      term_putchat(' ', term_color, x, y);
}

void term_puts(const char *s)
{
  char *p = s;

  for (; *p++ != '\0';){
    term_putch(*p);
  }
}

#ifdef __cplusplus
extern "C"
#endif

int kmain(void)
{
  term_init();
  term_puts("kernel says hello\n");

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

