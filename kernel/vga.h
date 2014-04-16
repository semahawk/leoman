/*
 *
 * term.h
 *
 * Created at:  Thu 10 Apr 18:56:58 2014 18:56:58
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef TERM_H
#define TERM_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

extern const size_t VGA_HEIGHT, VGA_WIDTH;
extern size_t vga_row, vga_col;
extern uint8_t vga_color;

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

void vga_init(void);
void vga_putch(char ch);
void vga_putchat(char ch, uint8_t color, size_t x, size_t y);
void vga_printf(const char *fmt, ...);
void vga_puts(const char *s);
void vga_putd(int v);
void vga_puthd(uint32_t v);
void vga_puthw(uint16_t v);
void vga_puthb(uint8_t v);
void vga_putnl(void);
uint8_t vga_make_color(enum vga_color fg, enum vga_color bg);

#endif /* TERM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

