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
#include <msg/io.h>

#include "screen.h"

/* IPC doesn't yet work so don't bother */
#if 0
static uint16_t *video_memory;
/* current column / row */
static int column, row;
/* current foreground and background colors */
static enum color fg_color, bg_color;
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
#endif

int main(void)
{
  struct msg_io msg;
  int reply;
  int sender;

  while (1){
    for (volatile int i = 0; i < 400000000; i++);
    sender = ipc_recv(&msg, sizeof msg);
    for (volatile int i = 0; i < 400000000; i++);
    reply = 0;
    ipc_reply(sender, &reply, sizeof reply);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

