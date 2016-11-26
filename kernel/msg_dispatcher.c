/*
 *
 * msg_dispatcher.c
 *
 * Created at:  26 Nov 2016 21:34:02 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <ipc.h>

/*
 * This is the 'kernel process'
 */
void msg_dispatcher(void)
{
  int msg, response;
  int sender;

  while (1){
    sender = ipc_recv(&msg, sizeof msg);
    response = 1;
    ipc_reply(sender, &response, sizeof response);
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

