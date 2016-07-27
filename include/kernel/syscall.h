/*
 *
 * syscall.h
 *
 * Created at:  Sat Sep 26 09:16:52 2015 09:16:52
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef SYSCALL_H
#define SYSCALL_H

/* one of this values is to be passed via eax */
/* we're poor :D */
typedef enum {
  SYS_write  = 4,
  SYS_getpid = 20,
  SYS_NUM
} syscall_t;

void syscall_install();

#endif /* SYSCALL_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

