/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/queue.h>
#include <sys/syscall.h>

#include <rte_debug.h>
#include <rte_atomic.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_per_lcore.h>
#include <rte_eal.h>
#include <rte_lcore.h>

#include "eal_private.h"
#include "eal_thread.h"

RTE_DEFINE_PER_LCORE(unsigned, _lcore_id) = LCORE_ID_ANY;
RTE_DEFINE_PER_LCORE(unsigned, _socket_id) = (unsigned)SOCKET_ID_ANY;
RTE_DEFINE_PER_LCORE(rte_cpuset_t, _cpuset);

/* set affinity for current EAL thread */
static int
eal_thread_set_affinity(void)
{
	unsigned lcore_id = rte_lcore_id();
	struct rte_config* cfg = rte_eal_get_configuration();

	/* acquire system unique id  */
	rte_gettid();

	/* update EAL thread core affinity */
	return rte_thread_set_affinity(&cfg->cpu_config->lcore_config[lcore_id].cpuset);
}

/* require calling thread tid by gettid() */
int rte_sys_gettid(void)
{
	return (int)syscall(SYS_gettid);
}

int rte_thread_setname(pthread_t id, const char *name)
{
	int ret = -1;
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 12)
	ret = pthread_setname_np(id, name);
#endif
#endif
	RTE_SET_USED(id);
	RTE_SET_USED(name);
	return ret;
}
