/*
 * Copyright (C) Yusuke Suzuki
 *
 * Keio University
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __BEIGNET_PLATFORM_H__
#define __BEIGNET_PLATFORM_H__


#include <linux/version.h>
#ifdef __KERNEL__ /* OS functions */
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/types.h>
#else /* user-space functions */
#include <errno.h> /* ENOMEN, etc. */
#include <sched.h> /* sched_yield, etc. */
#include <stdint.h> /* uint32_t, etc.*/
#include <stdio.h> /* printf, etc. */
#include <stdlib.h> /* malloc/free, etc. */
#include <string.h> /* memcpy, etc. */
#endif

#ifdef __KERNEL__ /* OS functions */
#define BEIGNET_PRINT(fmt, arg...) printk("[beignet] " fmt, ##arg)
#ifdef BEIGNET_DEBUG_PRINT
#define BEIGNET_DPRINT(fmt, arg...) printk("[beignet:debug] " fmt, ##arg)
#else
#define BEIGNET_DPRINT(fmt, arg...)
#endif
#define MALLOC(x) vmalloc(x)
#define FREE(x) vfree(x)
#define SCHED_YIELD() schedule()
#define MB() mb()
#define COPY_FROM_USER(dst, src, size) \
		copy_from_user(dst, (void __user *) src, size)
#define COPY_TO_USER(dst, src, size) \
		copy_to_user((void __user *) dst, src, size)
#define IOREAD32(addr) ioread32((void /*__force __iomem*/ *)addr)
#define IOWRITE32(val, addr) iowrite32(val, (void /*__force __iomem*/ *)addr)

#define GETTID() task_pid_nr(current)
typedef struct mutex* LOCK_T;
#define LOCK_INIT(l) mutex_init(l)
#define LOCK(l)         mutex_lock(l)
#define UNLOCK(l) mutex_unlock(l)
typedef struct timeval TIME_T;
#define GETTIME(t) do_gettimeofday(t)
#define YIELD() yield()


#else /* user-space functions */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h> /* instead of spinlock.h*/

#define GETTID() syscall(SYS_gettid)
typedef pthread_mutex_t LOCK_T;
#define LOCK_INIT(l) pthread_mutex_init(l, NULL)
#define LOCK(l)         pthread_mutex_lock(l)
#define UNLOCK(l) pthread_mutex_unlock(l)
typedef struct timespec TIME_T;
#define GETTIME(t) clock_gettime(CLOCK_MONOTONIC, t);
#define YIELD() sched_yield()


#define BEIGNET_PRINT(fmt, arg...) fprintf(stderr, "[beignet] " fmt, ##arg)
#ifdef BEIGNET_DEBUG_PRINT
#define BEIGNET_DPRINT(fmt, arg...)					\
	if (BEIGNET_DEBUG_PRINT)							\
		fprintf(stderr, "[beignet:debug] " fmt, ##arg)
#else
#define BEIGNET_DPRINT(fmt, arg...)
#endif
#define MALLOC(x) malloc(x)
#ifdef BEIGNET_SCHED_DISABLED
#define FREE(x) free(x)
#else
#define FREE(x) memset(x, 0, sizeof(*x))
#endif
#ifdef SCHED_DEADLINE
#define SCHED_YIELD() if (sched_getscheduler(getpid()) != SCHED_DEADLINE) sched_yield();
#else
#define SCHED_YIELD() sched_yield()
#endif
#if (__GNUC__ * 100 + __GNUC_MINOR__ >= 404)
#define MB() __sync_synchronize()
#else
#define MB()
#endif
/* should never used */
#define COPY_FROM_USER(dst, src, size) memcpy(dst, src, size) 
/* should never used */
#define COPY_TO_USER(dst, src, size) memcpy(dst, src, size)
#define IOREAD32(addr) *(uint32_t *)(addr)
#define IOWRITE32(val, addr) *(uint32_t *)(addr) = val
#endif

#ifdef __KERNEL__ /* OS functions */
static inline char* STRDUP(const char *str) {
	size_t len;
	char *buf;
	if (!str) {
		return NULL;
	}
	len = strlen(str) + 1;
	buf = MALLOC(len);
	if (buf) {
		memcpy(buf, str, len);
	}
	return buf;
}
#else /* user-space functions */
#define STRDUP strdup
#endif

#endif  /* __GDEV_PLATFORM_H__ */
