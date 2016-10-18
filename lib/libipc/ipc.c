/*
 *
 * ipc.c
 *
 * Created at:  27 Jul 2016 23:12:35 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/common.h> /* for bool type */
#include <kernel/syscall.h>
#include <ipc.h>

bool ipc_send(int receiver, void *msg)
{
  /* TODO */
  return false;
}

bool ipc_recv(int sender, void *msg)
{
  /* TODO */
  return false;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

