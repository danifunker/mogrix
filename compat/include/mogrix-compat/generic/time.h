/*
 * mogrix-compat/generic/time.h
 *
 * Wrapper that includes the real time.h and adds GNU extensions
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_TIME_H
#define _MOGRIX_COMPAT_TIME_H

/* Include the real IRIX time.h */
#include_next <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * timegm - Convert broken-down time to time_t in UTC (GNU extension)
 *
 * IRIX doesn't have timegm. We provide our own implementation.
 * This is the inverse of gmtime().
 *
 * Forward-declare struct tm at file scope to avoid prototype-scope
 * tag creation. When this header is pulled in early (e.g., via
 * irix-cc's time.h force-include), struct tm may not yet be fully
 * defined. Without this forward declaration, the struct tm in the
 * timegm prototype would be a prototype-scope tag (C99 6.2.1p4),
 * causing "conflicting types" when timegm is later re-declared
 * with the file-scope struct tm.
 */
struct tm;
#ifndef timegm
time_t timegm(struct tm *tm);
#endif

/*
 * CLOCK_MONOTONIC - IRIX has the clock, under a different name.
 *
 * <sys/ptimers.h> (reached from <time.h> via internal/time_core.h) defines
 * CLOCK_REALTIME and CLOCK_SGI_CYCLE and nothing else. CLOCK_SGI_CYCLE *is*
 * the monotonic clock: a free-running hardware counter with an arbitrary
 * epoch, which is exactly the contract CLOCK_MONOTONIC specifies. So the
 * alias is a rename, not an approximation.
 *
 * Aliasing to CLOCK_REALTIME instead -- the obvious-looking alternative --
 * would be wrong in a way that hides: settimeofday or an NTP step would make
 * elapsed time jump, or run backwards, in every caller that asked for a
 * monotonic clock precisely so that could not happen.
 *
 * One caveat worth stating. Defining this makes `#ifdef CLOCK_MONOTONIC`
 * succeed, so a package may now take a branch it previously skipped. That is
 * the intent for timing code. It is NOT safe for pthread_condattr_setclock,
 * which IRIX does not have at all -- see compat/rust/rust_compat.c, which
 * stubs it. Code reaching for a monotonic condvar still has to be handled
 * there rather than here.
 */
/* <sys/ptimers.h> is where the CLOCK_* numbers live, and IRIX's <time.h>
 * only reaches it when (_POSIX93 || _ABIAPI || _XOPEN5) && _NO_ANSIMODE hold
 * at that moment. Those are not all set when irix-cc force-includes <time.h>
 * at the top of every translation unit, so CLOCK_SGI_CYCLE is genuinely not
 * defined yet here -- it arrives later, via whatever pulls <sys/time.h> in.
 * Include it directly rather than testing for a macro that shows up after we
 * have already run. */
#include <sys/ptimers.h>

#ifndef CLOCK_MONOTONIC
#ifdef CLOCK_SGI_CYCLE
#define CLOCK_MONOTONIC CLOCK_SGI_CYCLE
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_TIME_H */
