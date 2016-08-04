/*
 *
 * fairy.h
 *
 * Created at:  04 Aug 2016 21:27:45 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef FAIRY_H
#define FAIRY_H

/* msg.type */
#define FAIRY_REQUEST_FAILED       0x0
#define FAIRY_REQUEST_VIDEO_MEMORY 0x1
#define FAIRY_VIDEO_MEMORY_ADDRESS 0x2

#ifndef __ASSEMBLY__
void proc_fairy(void);
#endif /* !__ASSEMBLY__ */

#endif /* !FAIRY_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

