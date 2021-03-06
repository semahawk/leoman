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
#include <kernel/x86.h>
#include <ipc.h>
#include <msg/kernel.h>

/* admittedly - this isn't a real driver */
/* it's just a dummy code to test IPC with the server */
int main(void)
{
  for (volatile int i = 0; i < 300000000; i++)
    ;

  while (1){
    /*(void)outb(0x10, 0x0);*/
    /*__asm volatile("cli\r\nhlt\r\n");*/
    /*puts("angle\n");*/
    for (volatile int i = 0; i < 100000000; i++);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

