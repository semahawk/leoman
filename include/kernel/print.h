/*
 *
 * print.h
 *
 * Created at:  26 Dec 2017 16:24:10 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_PRINT_H
#define KERNEL_PRINT_H

#include <stdarg.h>
#include <stddef.h>

#define KERNEL_MAX_PRINT_SIZE 256

void kformat(char *buf, size_t size, char *fmt, ...);
void kvformat(char *buf, size_t size, char *fmt, va_list args);

#endif /* !KERNEL_PRINT_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

