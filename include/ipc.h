/*
 *
 * ipc.h
 *
 * Created at:  28 Jul 2016 00:24:29 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef IPC_H
#define IPC_H

#include <kernel/common.h> /* for bool type */

struct msg {
  int sender;
  int receiver;
  int type;
  int data;
};

/*
 * Send the <msg> to process <receiver>
 *
 * This call is non-blocking
 */
bool ipc_send(int receiver, struct msg *msg);

/*
 * Check if any messages were sent to the current process
 *
 * If a message was in the current process' queue, it's filled into <msg>
 *
 * If <sender> is different than 0, then only messages from the
 * specified <sender> are taken into account
 *
 * This call is non-blocking
 */
bool ipc_recv(int sender, struct msg *msg);

#endif /* !IPC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

