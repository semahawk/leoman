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
#if 0
  for (ipc_dummy(); strlen("go go go"););
#else
  static int i;
  /* feel the power of nolibc! */

  while (1){
    char *msg = "s";
    int len = 1;

    /* call syscall #4 (write) */
    __asm volatile("movl %0, %%edx"::"r"(len));
    __asm volatile("movl %0, %%ecx"::"r"(msg));
    __asm volatile("movl $0, %%ebx":::"ebx");
    __asm volatile("movl $4, %%eax":::"eax");
    __asm volatile("int $0x80");

    for (i = 0; i < 1000000; i++)
      /* wait a bit */;
  }
#endif

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

