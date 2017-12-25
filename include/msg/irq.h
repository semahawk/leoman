/*
 *
 * irq.h
 *
 * Created at:  25 Dec 2017 14:14:17 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef IRQ_H
#define IRQ_H

/* this message is sent by the kernel to any process that has requested
 * an interrupt forward (and, obviously, only when the requested interrupt
 * was raised) */
struct msg_irq {
    int which;
};

#endif /* !IRQ_H */

/*
 * vi: ft=c:ts=4:sw=4:expandtab
 */
