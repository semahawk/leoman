/*
 *
 * main.c
 *
 * Created at:  27 Jan 2016 11:55:10 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

/* some VGA-related constants */
const size_t VGA_COLS = 80, VGA_ROWS = 25;
static uint16_t *const VGA_BUFFER = (uint16_t *)0xb8000;

/* current position at which to write characters */
static unsigned x = 0, y = 0;

static void putc(const char c)
{
  size_t idx;

  switch (c){
    case '\n':
      y++;
      x = -1;
      break;
    case '\r':
      x = -1;
      break;
    default:
      idx = y * VGA_COLS + x;
      *(VGA_BUFFER + idx) = 0x0700 | (uint8_t)c;
      break;
  }

  if (++x >= VGA_COLS){
    x = 0;

    if (++y >= VGA_ROWS){
      y = 0;
    }
  }
}

static void puts(const char *s)
{
  while (*s != '\0')
    putc(*(s++));
}

int main(void)
{
  puts("hello, second stage");

  while (1);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

