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
  while (1){
    puts("hello from userspace (times infinity)!\n");
    for (volatile int i = 0; i < 80000000; i++);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

