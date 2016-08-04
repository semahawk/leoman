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

struct msg {
  int sender;
  int receiver;
  int type;
  int data;
};

int ipc_send(int receiver, struct msg *msg);
int ipc_recv(int sender, struct msg *msg);

#endif /* !IPC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

