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
    MSG_PUTS,
  } type;

  unsigned char chars[MSG_IO_BUFSIZE];
};

#endif /* !IO_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

