/*
 *
 * getc.c
 *
 * Created at:  07 Oct 2017 18:47:57 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <ipc.h>
#include <msg/io.h>

int getc(void)
{
  struct msg_io msg;
  int result;
  int kbd = find_by_name("kbd");

  msg.type = MSG_GETC;

  ipc_send(kbd, &msg, sizeof msg, &result, sizeof result);

  return result;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

