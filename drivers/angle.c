/*
 *
 * angle.c
 *
 * Created at:  27 Jul 2016 18:10:11 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <kernel/proc.h>
#include <ipc.h>
#include <msg/kernel.h>

/* admittedly - this isn't a real driver */
/* it's just a dummy code to test IPC with the server */
int main(void)
{
  struct msg_kernel msg;
  int result;

  msg.type = MSG_MAP_MEMORY;
  msg.data.map_memory.paddr  = 0xb8000;
  msg.data.map_memory.length = 80 * 25;

  ipc_send(0, &msg, sizeof msg, &result, sizeof result);

  uint16_t *addr = (void *)result;

  while (1)
    *addr = 0x0b01;
  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

