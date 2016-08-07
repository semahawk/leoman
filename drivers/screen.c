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

#include <kernel/fairy.h>

#include <stdint.h>
#include <ipc.h>

int main(void)
{
  uint16_t *mem;
  struct msg msg;

  msg.type = FAIRY_REQUEST_VIDEO_MEMORY;
  ipc_send(1, &msg);

  while (1){
    /* block until we get the message with the video memory address back */
    if (ipc_recv(1, &msg)){

      if (msg.type == FAIRY_VIDEO_MEMORY_ADDRESS){
        mem = (void *)msg.data;
        break;
      }
    }
  }

  while (1){
    if (ipc_recv(2, &msg)){
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

