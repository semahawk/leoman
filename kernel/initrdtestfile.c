#include <stdint.h>

int main(void)
{
  static uint16_t *buf = (uint16_t *)0xb8000;

  /* feel the power of nolibc! */
  while (1){
    /* a white smiley face on a black background */
    *buf++ = 0x0f01;
    /* yup, no buffer overflow handling, yay */

    for (int i = 0; i < 3000000; i++)
      ;
  }

  return 7;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

