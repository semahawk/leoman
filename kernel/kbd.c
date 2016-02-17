/*
 *
 * kbd.c
 *
 * Created at:  Sun 13 Apr 15:24:28 2014 15:24:28
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include "common.h"
#include "idt.h"
#include "vga.h"
#include "x86.h"

/* last key pressed */
static uint8_t key_buffer;

#define KEY_LSHIFT 0x2a
#define KEY_RSHIFT 0x36
/* take advantage of the fact that released keys have the 7th bit set */
#define RELEASED(key) ((key) | 0x80)

/* these determine which element from a key's array should be pulled */
#define MASK_NORMAL 0
#define MASK_SHIFT  1
/* TODO: support for Alts and Controls */

/* keep track of which key is currently being pressed */
/* element number MASK_SHIFT in this array tells whether Shift is pressed (and
 * so on, with the rest of special keys) */
static uint8_t is_pressed[2];

/* a very simple keyboard layout (beware, it's Dvorak!) */
/* TODO: make it customizable (preferably at compile time) */
static uint8_t layout[][2] =
{
  { 0x0, 0x0   }, { 0x1b, 0x1b }, { '1', '!'   }, { '2', '@'   },
  { '3', '#'   }, { '4', '$'   }, { '5', '%'   }, { '6', '^'   },
  { '7', '&'   }, { '8', '*'   }, { '9', '('   }, { '0', ')'   },
  { '[', '{'   }, { ']', '}'   }, { '\b', '\b' }, { '\t', '\t' },
  { '\'', '"'  }, { ',', '<'   }, { '.', '>'   }, { 'p', 'P'   },
  { 'y', 'Y'   }, { 'f', 'F'   }, { 'g', 'G'   }, { 'c', 'C'   },
  { 'r', 'R'   }, { 'l', 'L'   }, { '/', '?'   }, { '=', '+'   },
  { '\n', '\n' }, { 0x0, 0x0   }, { 'a', 'A'   }, { 'o', 'O'   },
  { 'e', 'E'   }, { 'u', 'U'   }, { 'i', 'I'   }, { 'd', 'D'   },
  { 'h', 'H'   }, { 't', 'T'   }, { 'n', 'N'   }, { 's', 'S'   },
  { '-', '_'   }, { '`', '~'   }, { 0x0, 0x0   }, { '\\', '|'  },
  { ';', ':'   }, { 'q', 'Q'   }, { 'j', 'J'   }, { 'k', 'K'   },
  { 'x', 'X'   }, { 'b', 'B'   }, { 'm', 'M'   }, { 'w', 'W'   },
  { 'v', 'V'   }, { 'z', 'Z'   }, { 0x0, 0x0   }, { 0x0, 0x0   },
  { 0x0, 0x0   }, { ' ', ' '   }, { 0x0, 0x0   }, { 0x0, 0x0   }
};

static inline uint8_t kbd_get_scancode(void)
{
  uint8_t c;

  do {
    c = inb(0x64);
  } while ((c & 0x1) == 0);

  return inb(0x60);
}

static void kbd_handler(struct intregs *regs)
{
  uint8_t scancode = kbd_get_scancode();
  uint8_t mask = MASK_NORMAL;

  /* see if Shift was pressed */
  if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT)
    is_pressed[MASK_SHIFT] = 1;
  /* see if Shift was released */
  if (scancode == RELEASED(KEY_LSHIFT) || scancode == RELEASED(KEY_RSHIFT))
    is_pressed[MASK_SHIFT] = 0;

  if (is_pressed[MASK_SHIFT])
    mask = MASK_SHIFT;

  /* don't bother with break codes */
  if (!(scancode & 0x80)){
    key_buffer = layout[scancode][mask];

    vga_putch(key_buffer);
  }
}

void kbd_install(void)
{
  irq_install_handler(1, kbd_handler);
}

void kbd_uninstall(void)
{
  irq_uninstall_handler(1);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

