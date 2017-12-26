/*
 *
 * print.c
 *
 * Created at:  26 Dec 2017 16:23:40 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdarg.h>
#include <stddef.h>

#include <kernel/print.h>
#include <kernel/vga.h>

static inline void putch(char **buf, size_t space_left, char ch)
{
    **buf = ch;
    (*buf)++;
}

static inline void puts(char **buf, size_t space_left, const char *s)
{
    while (*s != '\0')
        putch(buf, space_left, *s++);
}

/*
 * Prints a given character <ch> as either an alphanumeric character (0-9) or
 * a lowercase hexadecimal (10-15)
 */
static inline void put_digit(char **buf, size_t space_left, char ch)
{
    static char digits[] = "0123456789abcdef";

    putch(buf, space_left, digits[ch % 16]);
}

/*
 * Print a given value <v> as a decimal integer.
 * NOTE: It's quite buggy with big(ger) numbers.
 */
static inline void putd(char **buf, size_t space_left, int v)
{
    int out[10 /* ceil(log10(2^32)) */] = {0};
    int i = 0;

    if (v == 0) {
        putch(buf, space_left, '0');
        return;
    }

    if (v < 0) {
        v = -v;
        putch(buf, space_left, '-');
    }

    for (; v > 0; v /= 10)
        out[i++] = v % 10;

    i--;

    while (i >= 0)
        put_digit(buf, space_left, out[i--]);
}

/*
 * Print the given value <v> as a hexadecimal double word
 */
static inline void puthd(char **buf, size_t space_left, uint32_t v)
{
    uint32_t mask = (uint32_t)0xf0000000;
    uint8_t i = 8; /* number of digits in a dword */

    for (; mask > 0; mask >>= 4, i--)
        put_digit(buf, space_left, (mask & v) >> (i * 4 - 4));
}

/*
 * Print the given value <v> as a hexadecimal word
 */
static inline void puthw(char **buf, size_t space_left, uint16_t v)
{
    uint16_t mask = (uint16_t)0xf000;
    uint8_t i = 4; /* number of digits in a word */

    for (; mask > 0; mask >>= 4, i--)
        put_digit(buf, space_left, (mask & v) >> (i * 4 - 4));
}

/*
 * Print the given value <v> as a hexadecimal byte
 */
static inline void puthb(char **buf, size_t space_left, uint8_t v)
{
    uint8_t mask = (uint8_t)0xf0;
    uint8_t i = 2; /* number of digits in a byte */

    for (; mask > 0; mask >>= 4, i--)
        put_digit(buf, space_left, (mask & v) >> (i * 4 - 4));
}

void kformat(char *buf, size_t size, char *fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    kvformat(buf, size, fmt, vl);
    va_end(vl);
}

void kvformat(char *buf, size_t size, char *fmt, va_list vl)
{
    /* TODO FIXME check for `size` */
    char *p = fmt;

    for (; *p != '\0'; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case '%':
                    putch(&buf, size, '%');
                    break;
                case 'c':
                    putch(&buf, size, (char)va_arg(vl, int));
                    break;
                case 'd':
                    putd(&buf, size, va_arg(vl, int));
                    break;
                case 'x':
                    puthd(&buf, size, va_arg(vl, uint32_t));
                    break;
                case 's':
                    puts(&buf, size, va_arg(vl, const char *));
                    break;
            }
        } else {
            *(buf++) = *p;
        }
    }

    *buf = '\0';
}

void kprintf(const char *fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    vga_vprintf(fmt, vl);
    va_end(vl);
}

/*
 * vi: ft=c:ts=4:sw=4:expandtab
 */
