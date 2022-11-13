/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <string.h>

#include <rte_lcore.h>
#include <rte_memory.h>
#include <rte_log.h>

#include "eal_thread.h"

RTE_DECLARE_PER_LCORE(unsigned , _socket_id);

unsigned rte_socket_id(void)
{
	return RTE_PER_LCORE(_socket_id);
}

int
rte_lcore_has_role(unsigned int lcore_id, enum rte_lcore_role_t role)
{
	struct rte_config *cfg = rte_eal_get_configuration();

	if (lcore_id >= RTE_MAX_LCORE)
		return -EINVAL;

	if (cfg->cpu_config->lcore_role[lcore_id] == role)
		return 0;

	return -EINVAL;
}

int eal_cpuset_socket_id(rte_cpuset_t *cpusetp)
{
	unsigned cpu = 0;
	int socket_id = SOCKET_ID_ANY;
	int sid;

	if (cpusetp == NULL)
		return SOCKET_ID_ANY;

	do {
		if (!CPU_ISSET(cpu, cpusetp))
			continue;

		if (socket_id == SOCKET_ID_ANY)
			socket_id = eal_cpu_socket_id(cpu);

		sid = eal_cpu_socket_id(cpu);
		if (socket_id != sid) {
			socket_id = SOCKET_ID_ANY;
			break;
		}

	} while (++cpu < RTE_MAX_LCORE);

	return socket_id;
}

int
rte_thread_set_affinity(rte_cpuset_t *cpusetp)
{
	int s;
	unsigned lcore_id;
	pthread_t tid;

	tid = pthread_self();
	struct rte_config *cfg = rte_eal_get_configuration();

	s = pthread_setaffinity_np(tid, sizeof(rte_cpuset_t), cpusetp);
	if (s != 0) {
		RTE_LOG(ERR, EAL, "pthread_setaffinity_np failed\n");
		return -1;
	}

	/* store socket_id in TLS for quick access */
	RTE_PER_LCORE(_socket_id) =
		eal_cpuset_socket_id(cpusetp);

	/* store cpuset in TLS for quick access */
	memmove(&RTE_PER_LCORE(_cpuset), cpusetp,
		sizeof(rte_cpuset_t));

	lcore_id = rte_lcore_id();
	if (lcore_id != (unsigned)LCORE_ID_ANY) {
		/* EAL thread will update lcore_config */
		cfg->cpu_config->lcore_config[lcore_id].socket_id = RTE_PER_LCORE(_socket_id);
		memmove(&cfg->cpu_config->lcore_config[lcore_id].cpuset, cpusetp,
			sizeof(rte_cpuset_t));
	}

	return 0;
}

void
rte_thread_get_affinity(rte_cpuset_t *cpusetp)
{
	assert(cpusetp);
	memmove(cpusetp, &RTE_PER_LCORE(_cpuset),
		sizeof(rte_cpuset_t));
}
