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
#include <kernel/fairy.h>
#include <ipc.h>

#include "screen.h"

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

int main(void)
{
  struct msg msg;

  /* FIXME there's apparently still some problems with loading (ELF) executables
   *       properly - which is why the initialization here */
  column = 0, row = 0;
  fg_color = LIGHT_BROWN, bg_color = BLACK;

  msg.type = FAIRY_REQUEST_VIDEO_MEMORY;
  ipc_send(1, &msg);

  while (1){
    /* block until we get the message with the video memory address back */
    if (ipc_recv(1, &msg)){

      if (msg.type == FAIRY_VIDEO_MEMORY_ADDRESS){
        video_memory = (void *)msg.data;
        break;
      }
    }
  }

  clear();

  while (1){
    /* TODO add some functionality to block a process which checked for any
     * messages but didn't find any to avoid busy looping like we are here if
     * nobody wants to print stuff */
    if (ipc_recv(0, &msg)){
      put_char((char)((char)msg.data & 0xff));
    }
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

