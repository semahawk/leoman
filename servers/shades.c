/*
 *
 * shades.c
 *
 * Created at:  27 Jul 2016 13:55:59 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <ipc.h>

int main(void)
{
  /* TODO have the driver ask another driver for the permissions for the video
   * memory, and probably the address too */
  short *mem = (short *)0xb8000;
  struct msg msg;

  while (1){
    /* FIXME specifying the first argument doesn't change a thing and so any
     * message from any other process get's fetched here */
    if (ipc_recv(1, &msg)){
      /* display the received data in the top-left corner */
      *(mem + 0) = 0x3e00 | (msg.data % (1 << 8));
    }

    for (unsigned i = 0; i < 10000000; i++);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

