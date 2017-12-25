/*
 *
 * ipc.h
 *
 * Created at:  25 Dec 2017 13:54:49 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_IPC_H
#define KERNEL_IPC_H

#include <kernel/proc.h>

#include <ipc.h>

int ipc_send_msg(struct msg_packet *msg, struct proc *receiver);
bool ipc_send_no_irq(int receiver, void *send_buf, size_t send_len, void *recv_buf, size_t recv_len);

#endif /* !KERNEL_IPC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

