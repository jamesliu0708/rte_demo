/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#ifndef EAL_THREAD_H
#define EAL_THREAD_H

#include <rte_lcore.h>

/**
 * basic loop of thread, called for each thread by eal_init().
 *
 * @param arg
 *   opaque pointer
 */
__attribute__((noreturn)) void *eal_thread_loop(void *arg);

/**
 * Init per-lcore info for master thread
 *
 * @param lcore_id
 *   identifier of master lcore
 */
void eal_thread_init_master(unsigned lcore_id);

/**
 * Get the NUMA socket id from cpu id.
 * This function is private to EAL.
 *
 * @param cpu_id
 *   The logical process id.
 * @return
 *   socket_id or SOCKET_ID_ANY
 */
unsigned eal_cpu_socket_id(unsigned cpu_id);

/**
 * Get the NUMA socket id from cpuset.
 * This function is private to EAL.
 *
 * @param cpusetp
 *   The point to a valid cpu set.
 * @return
 *   socket_id or SOCKET_ID_ANY
 */
int eal_cpuset_socket_id(rte_cpuset_t *cpusetp);

/**
 * Default buffer size to use with eal_thread_dump_affinity()
 */
#define RTE_CPU_AFFINITY_STR_LEN            256


#endif /* EAL_THREAD_H */
