/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sched.h>

#include <rte_log.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include <rte_common.h>
#include <rte_debug.h>

#include "eal_internal_cfg.h"
#include "eal_private.h"
#include "eal_thread.h"

/*
 * Parse /sys/devices/system/cpu to get the number of physical and logical
 * processors on the machine. The function will fill the cpu_info
 * structure.
 */
int
rte_eal_cpu_init(void)
{
	unsigned lcore_id;
	unsigned count = 0;

	struct rte_config *cfg = rte_eal_get_configuration();

	/*
	 * Parse the maximum set of logical cores, detect the subset of running
	 * ones and enable them by default.
	 */
	for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
		cfg->cpu_config->lcore_config[lcore_id].core_index = count;

		/* init cpuset for per lcore config */
		CPU_ZERO(&cfg->cpu_config->lcore_config[lcore_id].cpuset);

		/* in 1:1 mapping, record related cpu detected state */
		cfg->cpu_config->lcore_config[lcore_id].detected = eal_cpu_detected(lcore_id);
		if (cfg->cpu_config->lcore_config[lcore_id].detected == 0) {
			cfg->cpu_config->lcore_role[lcore_id] = ROLE_OFF;
			cfg->cpu_config->lcore_config[lcore_id].core_index = -1;
			continue;
		}

		/* By default, lcore 1:1 map to cpu id */
		CPU_SET(lcore_id, &cfg->cpu_config->lcore_config[lcore_id].cpuset);

		/* By default, each detected core is enabled */
		cfg->cpu_config->lcore_role[lcore_id] = ROLE_RTE;
		cfg->cpu_config->lcore_config[lcore_id].core_role = ROLE_RTE;
		cfg->cpu_config->lcore_config[lcore_id].core_id = eal_cpu_core_id(lcore_id);
		cfg->cpu_config->lcore_config[lcore_id].socket_id = eal_cpu_socket_id(lcore_id);
		if (cfg->cpu_config->lcore_config[lcore_id].socket_id >= RTE_MAX_NUMA_NODES) {
#ifdef RTE_EAL_ALLOW_INV_SOCKET_ID
			cfg->cpu_config->lcore_config[lcore_id].socket_id = 0;
#else
			RTE_LOG(ERR, EAL, "Socket ID (%u) is greater than "
				"RTE_MAX_NUMA_NODES (%d)\n",
				cfg->cpu_config->lcore_config[lcore_id].socket_id,
				RTE_MAX_NUMA_NODES);
			return -1;
#endif
		}
		RTE_LOG(DEBUG, EAL, "Detected lcore %u as "
				"core %u on socket %u\n",
				lcore_id, cfg->cpu_config->lcore_config[lcore_id].core_id,
				cfg->cpu_config->lcore_config[lcore_id].socket_id);
		count++;
	}
	/* Set the count of enabled logical cores of the EAL configuration */
	cfg->cpu_config->lcore_count = count;
	RTE_LOG(DEBUG, EAL,
		"Support maximum %u logical core(s) by configuration.\n",
		RTE_MAX_LCORE);
	RTE_LOG(INFO, EAL, "Detected %u lcore(s)\n", cfg->cpu_config->lcore_count);

	return 0;
}
