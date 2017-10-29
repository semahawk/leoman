/*
 *
 * screen.c
 *
 * Created at:  27 Jul 2016 13:55:59 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include <ipc.h>
#include <msg/kernel.h>
#include <msg/io.h>

#include "screen.h"

static uint16_t *video_memory;
/* current column / row */
static int column = 0, row = 0;
/* current foreground and background colors */
static enum color fg_color = 0xf, bg_color = 0x0;
/* screen's dimensions */
static const int screen_columns = 80, screen_rows = 25;

static void scrollup(void)
{
  for (unsigned y = 1; y < screen_rows; y++){
    for (unsigned x = 0; x < screen_columns; x++){
      *(video_memory + ((y - 1) * screen_columns + x)) = *(video_memory + (y * screen_columns + x));
    }
  }

  for (unsigned x = 0; x < screen_columns; x++){
    *(video_memory + ((screen_rows - 1) * screen_columns + x)) = '\0';
  }
}

static void put_char_at(char ch, int x, int y)
{
  const unsigned idx = y * screen_columns + x;

  *(video_memory + idx) = ((bg_color << 4 | fg_color) << 8) | ch;
}

static void put_char(char ch)
{
  if (column >= screen_columns || ch == '\n'){
    column = 0;
    row++;
  }

  if (row >= screen_rows){
    scrollup();
    row--;
  }

  if (ch == '\n')
    return;

  put_char_at(ch, column, row);
  column++;
}

static void clear(void)
{
  for (unsigned y = 0; y < screen_rows; y++)
    for (unsigned x = 0; x < screen_columns; x++)
      put_char_at('\0', x, y);
}

int main(void)
{
  struct msg_io msg;
  int reply;
  int sender;

  /* initialize the variables */
  /* FIXME: apparently we don't handle BSS to well */
  column = row = 0;
  /* current foreground and background colors */
  fg_color = 0xb, bg_color = 0x0;

  {
    struct msg_kernel msg;
    int response;

    msg.type = MSG_MAP_MEMORY;
    msg.data.map_memory.paddr  = 0xb8000;
    msg.data.map_memory.length = (screen_columns * screen_rows) * sizeof(*video_memory);

    for (volatile int i = 0; i < 100000000; i++);

    ipc_send(0, &msg, sizeof msg, &response, sizeof response);

    video_memory = (void *)response;
  }

  clear();

  while (1){
    sender = ipc_recv(&msg, sizeof msg);

    switch (msg.type){
      case MSG_PUTS: {
        /* print the buffer in msg.chars untill we
         *   a) hit the first nul or
         *   b) hit the end of the buffer */
        for (unsigned char *p = msg.chars; *p != '\0'; p++){
          if ((uintptr_t)p - (uintptr_t)msg.chars >= MSG_IO_BUFSIZE)
            break;

          put_char(*p);
        }
      }
      case MSG_PUTC: {
        put_char(msg.one_char);
      }

        break;
      default:
        break;
    }

    reply = 0x11babe11;
    ipc_reply(sender, &reply, sizeof reply);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

