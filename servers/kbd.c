/*
 *
 * kbd.c
 *
 * Created at:  07 Sep 2017 18:37:32 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>
#include <stdio.h>

#include <ipc.h>
#include <msg/kernel.h>
#include <msg/io.h>

#include <kernel/common.h>
#include <kernel/idt.h>
#include <kernel/vga.h>
#include <kernel/x86.h>

/* last key pressed */
static uint8_t _last_scancode;
static bool _key_pressed = false;

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

static struct intregs *kbd_irq_handler(struct intregs *regs)
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

  if (!(scancode & 0x80)){
    _last_scancode = scancode;
    _key_pressed = true;
  }

  return regs;
}

int main(void)
{
  struct msg_io msg;
  int reply;
  int sender;

  {
    /* request IRQ1 (keyboard) */
    struct msg_kernel msg;
    int response;

    msg.type = MSG_REQUEST_INTERRUPT_FORWARDING;
    msg.data.interrupt.which = 1;
    msg.data.interrupt.handler = kbd_irq_handler;

    /* TODO: error handling */
    ipc_send(0, &msg, sizeof msg, &response, sizeof response);
  }

  while (1){
    sender = ipc_recv(&msg, sizeof msg);

    switch (msg.type){
      case MSG_GETC: {
        while (!_key_pressed)
          ;

          reply = layout[_last_scancode][0];

        _key_pressed = false;
      }

        break;
      default:
        break;
    }

    ipc_reply(sender, &reply, sizeof reply);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

