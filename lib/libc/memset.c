/*
 *
 * memset.c
 *
 * Created at:  03 Oct 2017 19:14:21 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

void *memset(void *str, int ch, size_t len)
{
  while (len-- != 0)
    *(uint8_t *)str++ = ch;

  return str;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

