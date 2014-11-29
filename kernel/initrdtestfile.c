#include <stdint.h>

int main(void)
{
  /* yeeah, no libc */
  *(uint16_t *)0xb8000 = 0x0f01; /* white smiley face on black background */

  return 7;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

