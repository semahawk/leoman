/*
 *
 * sar.h
 *
 * Created at:  Thu 27 Nov 17:39:18 2014 17:39:18
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef SAR_H
#define SAR_H

/*
 * the Simple ARchive format
 */

#include <stdint.h>

#define SAR_MAGIC 0x52415307

/* the archive's header (there's only one per archive) */
struct sar_header {
  uint32_t magic;
} __attribute__((packed));

/* each file's header in the archive (ie. every archived file has one) */
/* they come contiguously after the archive's header */
struct sar_file {
  uint32_t size;     /* the file's (raw) size (not aligned) */
  uint32_t offset;   /* file's content's offset (always 4KiB-aligned) */
  uint32_t namelen;  /* filename's string length (including nul) */
  /* right after that little header comes the NUL-terminated string */
} __attribute__((packed));

/* a handy macro to fetch given file's name */
/* <fhdr> is of type { struct sar_file * } */
#define FHDR_FNAME(fhdr) ((const char *)((char *)(fhdr) + sizeof(struct sar_file)))

struct sar_file *sar_lookup(const void *, const char *);
void *sar_get_contents(const void *, const char *);

#endif /* SAR_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

