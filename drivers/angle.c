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

/* admittedly - this isn't a real driver */
/* it's just a dummy code to test IPC with the server */
int main(void)
{
  ipc_send(1, 0x0, 0x0, 0x0, 0x0);

  /* send another dummy message to make it visible that the previous transaction
   * was completed */
  ipc_send(1, 0x0, 0x0, 0x0, 0x0);

  while (1);
  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

