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

struct msg_packet {
  struct proc *sender,   *receiver;
  void        *send_buf, *recv_buf;
  size_t       send_len,  recv_len;
  void *phys_send_buf;
};

/*
 * Send <send_len> bytes located at <send_buf> to process <receiver>, which is
 * expected to fill it's result into <recv_buf>.
 *
 * If the <receiver> hasn't yet called ipc_recv() then the current process
 * becomes send-blocked. Once the <receiver> calls ipc_recv() the kernel changes
 * the current process' state to reply-blocked. When the <receiver> calls
 * ipc_reply() the current process becomes ready (gets unblocked).
 *
 * So this call is blocking.
 */
bool ipc_send(int receiver, void *send_buf, size_t send_len, void *recv_buf, size_t recv_len);

/*
 * Check if any messages were sent to the current process
 *
 * If no other process has sent a message to the current process, then it
 * gets receive-blocked.
 * When another process sends a message to the current process then it
 * becomes ready to be scheduled (gets unblocked).
 *
 * If another process has already sent a message to the current process then
 * this call returns with the message (fills it into <msg>)
 *
 * So this call is blocking.
 *
 * Return the process id of the original sender which you can use when replying
 * to the original message.
 */
int ipc_recv(void *recv_buf, size_t recv_len);

/*
 * Send a message (<len> bytes at location <msg>) with a response to the
 * original <sender>. The <sender> then becomes ready (gets unblocked) and able
 * to act upon the reply.
 */
bool ipc_reply(int sender, void *send_buf, size_t send_len);

#endif /* !IPC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

