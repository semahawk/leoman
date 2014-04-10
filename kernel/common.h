/*
 *
 * common.h
 *
 * Created at:  Wed  9 Apr 11:48:18 2014 11:48:18
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/* few beautifiers */
#define __PACKED __attribute__((packed))
#define __NAKED  __attribute__((naked))

/* these should later go to string.h, as soon as we have libc */
void *memset(void *dst, int ch, size_t len);
void *memcpy(void *dst, void *src, size_t len);

/* terminal/VGA thingies */
extern const size_t VGA_HEIGHT, VGA_WIDTH;
extern size_t term_row, term_col;
extern uint8_t term_color;

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

void term_putch(char ch);
void term_putchat(char ch, uint8_t color, size_t x, size_t y);
void term_puts(const char *s);
uint8_t make_color(enum vga_color fg, enum vga_color bg);

#endif /* COMMON_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

