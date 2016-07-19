/*
 *
 * idle.c
 *
 * Created at:  02 Jul 2016 21:50:50 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  static int i;

  /* feel the power of nolibc! */
  while (1){
    char *msg = "\1";
    int len = 1;

    /* call syscall #4 (write) */
    __asm volatile("movl %0, %%edx"::"r"(len));
    __asm volatile("movl %0, %%ecx"::"r"(msg));
    __asm volatile("movl $0, %%ebx":::"ebx");
    __asm volatile("movl $4, %%eax":::"eax");
    __asm volatile("int $0x80");

    /* sleep a bit */
    __asm volatile("sti\nhlt\ncli");
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

