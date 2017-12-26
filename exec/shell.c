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

int main(void)
{
  while (1);
  for (volatile int i = 0; i < 10000000; i++);

  puts("type! ");
  while (1){
    char ch = getc();
    putc(ch);
  }

  /* we have nowhere to return right know, actually */
  /* but keep the compiler happy */
  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

