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

#endif /* !KERNEL_IPC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

