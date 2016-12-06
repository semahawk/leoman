/*
 *
 * memcpy.c
 *
 * Created at:  06 Dec 2016 19:58:14 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dst, void *src, size_t len)
{
  void *ret = dst;

  while (len-- != 0)
    *(uint8_t *)dst++ = *(uint8_t *)src++;

  return ret;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

