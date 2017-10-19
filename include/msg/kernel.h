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

#include <kernel/idt.h>

struct msg_kernel {
  enum {
    MSG_REQUEST_INTERRUPT_FORWARDING,
    MSG_MAP_MEMORY,
    MSG_PORT_IN_BYTE,
    MSG_PORT_OUT_BYTE,
  } type;

  union {
    struct {
      int which;
      irq_handler_t handler;
    } interrupt;

    struct {
      uintptr_t paddr;
      size_t length;
    } map_memory;

    struct {
      uint16_t which;
      uint32_t data;
    } port;
  } data;
};

#endif /* !MSG_KERNEL_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

