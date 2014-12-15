#include <stdint.h>

int main(void)
{
  static uint16_t *buf = (uint16_t *)0xb8000;
  /* feel the power of nolibc! */
  /*while (1){*/
    *buf++ = 0x0f01;
    /*buf = (uint16_t *)((uint32_t)buf % (25 * 80));*/
  /*}*/

  return 7;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

