/*
 *
 * com.c
 *
 * Created at:  26 Dec 2017 17:37:37 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include <kernel/com.h>
#include <kernel/x86.h>

/*
 * reference: http://wiki.osdev.org/Serial_Ports
 */

#define PORT 0x3f8 /* COM1 */
#define BAUDRATE 115200

void com_init(void)
{
    outb(PORT + 1, 0x00);              // Disable all interrupts
    outb(PORT + 3, 0x80);              // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 115200 / BAUDRATE); // Set divisor to 1 (lo byte) 115200 baud
    outb(PORT + 1, 0x00);              //                  (hi byte)
    outb(PORT + 3, 0x03);              // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);              // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);              // IRQs enabled, RTS/DSR set
}

void com_putc(uint8_t ch)
{
    /* wait until transmit is empty */
    while ((inb(PORT + 5) & 0x20) == 0);

    outb(PORT, ch);
}

void com_puts(const char *s)
{
    while (*s != '\0')
        com_putc(*s++);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
