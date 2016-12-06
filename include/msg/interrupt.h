/*
 *
 * interrupt.h
 *
 * Created at:  27 Nov 2016 09:53:45 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

struct msg_interrupt {
  enum {
    MSG_INTERRUPT_REQUEST_FORWARDING,
  } type;

  /* interrupt number which to forward */
  unsigned which;
};

#endif /* !INTERRUPT_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

