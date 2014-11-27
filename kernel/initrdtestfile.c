#include <stdint.h>

int main(void)
{
  /* yeeah, no libc */
  *(uint16_t *)0xb8000 = 0x1020;

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

