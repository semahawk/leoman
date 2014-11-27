/*
 *
 * tar.c
 *
 * Created at:  Thu 27 Nov 17:47:19 2014 17:47:19
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include "tar.h"

unsigned tar_calculate_size(const char *sz)
{
  unsigned size = 0,
           i = 11,
           count = 1;

  for (; i > 0; i--, count *= 8)
    size += ((sz[i - 1] - '0') * count);

  return size;
}

unsigned tar_get_size(const void *file, const char *fname)
{
  struct tar_header *hdr;

  if ((hdr = tar_lookup(file, fname)) != NULL)
    return tar_calculate_size(hdr->size);
  else
    return 0;
}

struct tar_header *tar_lookup(const void *file, const char *fname)
{
  unsigned i = 0;

  for (;; i++){
    struct tar_header *hdr = (struct tar_header *)file;

    if (hdr->fname[0] == '\0')
      break;

    unsigned size = tar_calculate_size(hdr->size);

    /* found it */
    if (!strcmp(fname, hdr->fname))
      return hdr;

    file += ((size / 512) + 1) * 512;

    if (size % 512)
      file += 512;
  }

  return NULL;
}

void *tar_get_contents(const void *file, const char *fname)
{
  struct tar_header *hdr;

  if ((hdr = tar_lookup(file, fname)) != NULL)
    return (void *)hdr + 512;
  else
    return NULL;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

