/*
 *
 * puts.c
 *
 * Created at:  07 Aug 2016 22:18:05 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <ipc.h>

int puts(const char *s)
{
  struct msg msg;

  for (; *s; s++){
    msg.data = *s;
    ipc_send(2, &msg);

    /* FIXME lousy way of not-losing messages */
    for (unsigned i = 0; i < 200000; i++);
  }

  return 1;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

