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
  for (ipc_dummy(); strlen("go go go"););

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

