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

/* values to put into msg.type */
/* this probably shouldn't be an enum, but a set of defines - if some processes
 * wanted to 'add' 'fields' to the type */
enum msg_type {
  IPC_DUMMY_AND_NOT_USED,
};

struct msg {
  int sender;
  int receiver;
  enum msg_type type;
  int data;
};

int ipc_send(int receiver, struct msg *msg);
int ipc_recv(int sender, struct msg *msg);

#endif /* !IPC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

