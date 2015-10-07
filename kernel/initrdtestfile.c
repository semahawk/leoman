#include <stdint.h>

int main(void)
{
  /* feel the power of nolibc! */
  while (1){
    char *msg = "\1";
    int len = 1;

    /* call syscall #4 (write) */
    __asm volatile("movl %0, %%edx"::"r"(len));
    __asm volatile("movl %0, %%ecx"::"r"(msg));
    __asm volatile("movl $0, %ebx");
    __asm volatile("movl $4, %eax");
    __asm volatile("int $0x80");

    for (int i = 0; i < 3000000; i++)
      /* wait a bit */;
  }

  return 7;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

