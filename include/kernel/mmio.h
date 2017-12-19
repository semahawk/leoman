/*
 *
 * mmio.h
 *
 * Created at:  18 Dec 2017 20:24:59 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

uint32_t mmio_read32(void *addr);
void mmio_write32(void *addr, uint32_t value);

#endif /* !MMIO_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
