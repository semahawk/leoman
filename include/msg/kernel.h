/*
 *
 * kernel.h
 *
 * Created at:  15 Dec 2016 17:55:43 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef MSG_KERNEL_H
#define MSG_KERNEL_H

#include <stdint.h>

struct msg_kernel {
  enum {
    MSG_REQUEST_INTERRUPT_FORWARDING,
    MSG_MAP_MEMORY,
  } type;

  union {
    struct {
      int which;
    } interrupt;

    struct {
      uintptr_t paddr;
      size_t length;
    } map_memory;
  } data;
};

#endif /* !MSG_KERNEL_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

