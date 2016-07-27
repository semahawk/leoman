/*
 *
 * sar.c
 *
 * Created at:  Thu 27 Nov 17:47:19 2014 17:47:19
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include <kernel/common.h>
#include <kernel/sar.h>

struct sar_file *sar_lookup(const void *file, const char *fname)
{
  struct sar_file *fhdr;

  for (fhdr = (struct sar_file *)(file + sizeof(struct sar_header));
       /* all of the fields have to be zero */
       fhdr->size | fhdr->offset | fhdr->namelen;
       fhdr = (struct sar_file *)((char *)fhdr + sizeof(struct sar_file) + fhdr->namelen))
    if (!strcmp(FHDR_FNAME(fhdr), fname))
      return fhdr;

  /*vga_printf("file %s not found\n", fname);*/
  return NULL;
}

void *sar_get_contents(const void *file, const char *fname)
{
  struct sar_file *fhdr;

  if ((fhdr = sar_lookup(file, fname)) != NULL)
    return (void *)file + fhdr->offset;
  else
    return NULL;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

