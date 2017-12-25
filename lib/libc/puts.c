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
#include <msg/io.h>

int puts(const char *s)
{
  struct msg_io msg;
  int result;
  int screen = find_by_name("screen");

  msg.type = MSG_PUTS;

  for (int i = 0; *s && i < MSG_IO_BUFSIZE; s++)
    msg.chars[i++] = *s;

  ipc_send(screen, &msg, sizeof msg, &result, sizeof result);

  return result;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

