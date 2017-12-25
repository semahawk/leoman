/*
 *
 * io.h
 *
 * Created at:  18 Oct 2016 20:19:53 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef IO_H
#define IO_H

#define MSG_IO_BUFSIZE 64

struct msg_io {
  enum {
    MSG_GETC,
    MSG_PUTS,
    MSG_PUTC,

    /* THIS IS A HACK */
    /* we should use msg_irq for this, but then, how should
     * the kbd server distinguish between the two? */
    MSG_IRQ,
  } type;

  unsigned char one_char;
  unsigned char chars[MSG_IO_BUFSIZE];
  unsigned int which_irq;
};

#endif /* !IO_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

