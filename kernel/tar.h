/*
 *
 * tar.h
 *
 * Created at:  Thu 27 Nov 17:39:18 2014 17:39:18
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef TAR_H
#define TAR_H

#include "common.h"

struct tar_header {
  char fname[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag[1];
} __PACKED;

unsigned tar_calculate_size(const char *);
unsigned tar_get_size(const void *, const char *);
struct tar_header *tar_lookup(const void *, const char *);
void *tar_get_contents(const void *, const char *);

#endif /* TAR_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

