/*
 *
 * pm.h
 *
 * Created at:  Tue 16 Dec 16:28:44 2014 16:28:44
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef PM_H
#define PM_H

#include "common.h"

void *palloc(void);
void  pfree(void *);
void *pm_init(struct kern_bootinfo *);

#endif /* PM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

