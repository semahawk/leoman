/*
 *
 * com.h
 *
 * Created at:  26 Dec 2017 17:37:44 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_COM_H
#define KERNEL_COM_H

#include <stdint.h>

void com_init(void);
void com_putc(uint8_t ch);
void com_puts(const char *s);

#endif /* !KERNEL_COM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

