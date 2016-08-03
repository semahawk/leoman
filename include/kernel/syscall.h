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

#define SYSCALL_SEND_MSG_VECTOR 186 /* 0xba */
#define SYSCALL_RECV_MSG_VECTOR 190 /* 0xbe */

#ifndef __ASSEMBLY__
void syscall_install();
#endif /* __ASSEMBLY__ */

#endif /* SYSCALL_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

