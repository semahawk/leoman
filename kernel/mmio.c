/*
 *
 * mmio.c
 *
 * Created at:  18 Dec 2017 20:24:31 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include <kernel/mmio.h>

uint32_t mmio_read32(void *addr)
{
    return *(volatile uint32_t *)addr;
}

void mmio_write32(void *addr, uint32_t value)
{
    *(volatile uint32_t *)addr = value;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
