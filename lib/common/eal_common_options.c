/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation.
 * Copyright(c) 2014 6WIND S.A.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE
#include <syslog.h>
#include <getopt.h>
#include <pthread.h>
#include <sched.h>

#include <rte_eal.h>
#include <rte_log.h>
#include <rte_version.h>
#include <rte_lcore.h>

#include "eal_internal_cfg.h"
#include "eal_options.h"
#include "eal_filesystem.h"

const char
eal_short_options[] =
	"d:" /* driver */
	"h"  /* help */
	"m:" /* memory size */
	"n:" /* memory channels */
	"r:" /* memory ranks */
	"v"  /* version */
	;

const struct option
eal_long_options[] = {
	{OPT_BASE_VIRTADDR,     1, NULL, OPT_BASE_VIRTADDR_NUM    },
	{OPT_FILE_PREFIX,       1, NULL, OPT_FILE_PREFIX_NUM      },
	{OPT_HELP,              0, NULL, OPT_HELP_NUM             },
	{OPT_HUGE_DIR,          1, NULL, OPT_HUGE_DIR_NUM         },
	{OPT_HUGE_UNLINK,       0, NULL, OPT_HUGE_UNLINK_NUM      },
	{OPT_LOG_LEVEL,         1, NULL, OPT_LOG_LEVEL_NUM        },
	{OPT_MASTER_LCORE,      1, NULL, OPT_MASTER_LCORE_NUM     },
	{OPT_NO_HUGE,           0, NULL, OPT_NO_HUGE_NUM          },
	{OPT_PROC_TYPE,         1, NULL, OPT_PROC_TYPE_NUM        },
	{OPT_SOCKET_MEM,        1, NULL, OPT_SOCKET_MEM_NUM       },
	{OPT_SYSLOG,            1, NULL, OPT_SYSLOG_NUM           },
	{0,                     0, NULL, 0                        }
};

static int master_lcore_parsed;

void
eal_reset_internal_config(struct internal_config *internal_cfg)
{
	int i;

	internal_cfg->memory = 0;
	internal_cfg->force_nrank = 0;
	internal_cfg->force_nchannel = 0;
	internal_cfg->hugefile_prefix = HUGEFILE_PREFIX_DEFAULT;
	internal_cfg->hugepage_dir = NULL;
	internal_cfg->force_sockets = 0;
	/* zero out the NUMA config */
	for (i = 0; i < RTE_MAX_NUMA_NODES; i++)
		internal_cfg->socket_mem[i] = 0;
	/* zero out hugedir descriptors */
	for (i = 0; i < MAX_HUGEPAGE_SIZES; i++)
		internal_cfg->hugepage_info[i].lock_descriptor = -1;
	internal_cfg->base_virtaddr = 0;

	internal_cfg->syslog_facility = LOG_DAEMON;
}

static void
eal_auto_detect_cores(struct rte_config *cfg)
{
	unsigned int lcore_id;
	unsigned int removed = 0;
	rte_cpuset_t affinity_set;
	pthread_t tid = pthread_self();

	if (pthread_getaffinity_np(tid, sizeof(rte_cpuset_t),
				&affinity_set) < 0)
		CPU_ZERO(&affinity_set);

	for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
		if (cfg->lcore_role[lcore_id] == ROLE_RTE &&
		    !CPU_ISSET(lcore_id, &affinity_set)) {
			cfg->lcore_role[lcore_id] = ROLE_OFF;
			removed++;
		}
	}

	cfg->lcore_count -= removed;
}

/* Changes the lcore id of the master thread */
static int
eal_parse_master_lcore(const char *arg)
{
	char *parsing_end;
	struct rte_config *cfg = rte_eal_get_configuration();

	errno = 0;
	cfg->master_lcore = (uint32_t) strtol(arg, &parsing_end, 0);
	if (errno || parsing_end[0] != 0)
		return -1;
	if (cfg->master_lcore >= RTE_MAX_LCORE)
		return -1;
    master_lcore_parsed = 1;

	/* ensure master core is not used as service core */
	if (lcore_config[cfg->master_lcore].core_role == ROLE_SERVICE) {
		RTE_LOG(ERR, EAL, "Error: Master lcore is used as a service core.\n");
		return -1;
	}

	return 0;
}

int
eal_check_common_options(struct internal_config *internal_cfg)
{
	struct rte_config *cfg = rte_eal_get_configuration();

	if (cfg->lcore_role[cfg->master_lcore] != ROLE_RTE) {
		RTE_LOG(ERR, EAL, "Master lcore is not enabled for DPDK\n");
		return -1;
	}

	if (internal_cfg->process_type == RTE_PROC_INVALID) {
		RTE_LOG(ERR, EAL, "Invalid process type specified\n");
		return -1;
	}
	if (index(internal_cfg->hugefile_prefix, '%') != NULL) {
		RTE_LOG(ERR, EAL, "Invalid char, '%%', in --"OPT_FILE_PREFIX" "
			"option\n");
		return -1;
	}
	if (internal_cfg->force_sockets == 1) {
		RTE_LOG(ERR, EAL, "Options -m and --"OPT_SOCKET_MEM" cannot "
			"be specified at the same time\n");
		return -1;
	}
	if (internal_cfg->no_hugetlbfs && internal_cfg->force_sockets == 1) {
		RTE_LOG(ERR, EAL, "Option --"OPT_SOCKET_MEM" cannot "
			"be specified together with --"OPT_NO_HUGE"\n");
		return -1;
	}

	if (internal_cfg->no_hugetlbfs && internal_cfg->hugepage_unlink) {
		RTE_LOG(ERR, EAL, "Option --"OPT_HUGE_UNLINK" cannot "
			"be specified together with --"OPT_NO_HUGE"\n");
		return -1;
	}

	return 0;
}

static enum rte_proc_type_t
eal_parse_proc_type(const char *arg)
{
	if (strncasecmp(arg, "primary", sizeof("primary")) == 0)
		return RTE_PROC_PRIMARY;
	if (strncasecmp(arg, "secondary", sizeof("secondary")) == 0)
		return RTE_PROC_SECONDARY;
	if (strncasecmp(arg, "auto", sizeof("auto")) == 0)
		return RTE_PROC_AUTO;

	return RTE_PROC_INVALID;
}

static int
eal_parse_syslog(const char *facility, struct internal_config *conf)
{
	int i;
	static struct {
		const char *name;
		int value;
	} map[] = {
		{ "auth", LOG_AUTH },
		{ "cron", LOG_CRON },
		{ "daemon", LOG_DAEMON },
		{ "ftp", LOG_FTP },
		{ "kern", LOG_KERN },
		{ "lpr", LOG_LPR },
		{ "mail", LOG_MAIL },
		{ "news", LOG_NEWS },
		{ "syslog", LOG_SYSLOG },
		{ "user", LOG_USER },
		{ "uucp", LOG_UUCP },
		{ "local0", LOG_LOCAL0 },
		{ "local1", LOG_LOCAL1 },
		{ "local2", LOG_LOCAL2 },
		{ "local3", LOG_LOCAL3 },
		{ "local4", LOG_LOCAL4 },
		{ "local5", LOG_LOCAL5 },
		{ "local6", LOG_LOCAL6 },
		{ "local7", LOG_LOCAL7 },
		{ NULL, 0 }
	};

	for (i = 0; map[i].name; i++) {
		if (!strcmp(facility, map[i].name)) {
			conf->syslog_facility = map[i].value;
			return 0;
		}
	}
	return -1;
}

static int
eal_parse_log_level(const char *arg)
{
	char *end, *str, *type, *level;
	unsigned long tmp;

	str = strdup(arg);
	if (str == NULL)
		return -1;

	if (strchr(str, ',') == NULL) {
		type = NULL;
		level = str;
	} else {
		type = strsep(&str, ",");
		level = strsep(&str, ",");
	}

	errno = 0;
	tmp = strtoul(level, &end, 0);

	/* check for errors */
	if ((errno != 0) || (level[0] == '\0') ||
		    end == NULL || (*end != '\0'))
		goto fail;

	/* log_level is a uint32_t */
	if (tmp >= UINT32_MAX)
		goto fail;

	if (type == NULL) {
		rte_log_set_global_level(tmp);
	} else if (rte_log_set_level_regexp(type, tmp) < 0) {
		printf("cannot set log level %s,%lu\n",
			type, tmp);
		goto fail;
	}

	free(str);
	return 0;

fail:
	free(str);
	return -1;
}

int
eal_adjust_config(struct internal_config *internal_cfg)
{
	int i;
	struct rte_config *cfg = rte_eal_get_configuration();

	eal_auto_detect_cores(cfg);

	if (internal_config.process_type == RTE_PROC_AUTO)
		internal_config.process_type = eal_proc_type_detect();

	/* default master lcore is the first one */
	if (!master_lcore_parsed) {
		cfg->master_lcore = rte_get_next_lcore(-1, 0, 0);
		lcore_config[cfg->master_lcore].core_role = ROLE_RTE;
	}

	/* if no memory amounts were requested, this will result in 0 and
	 * will be overridden later, right after eal_hugepage_info_init() */
	for (i = 0; i < RTE_MAX_NUMA_NODES; i++)
		internal_cfg->memory += internal_cfg->socket_mem[i];

	return 0;
}

int
eal_parse_common_option(int opt, const char *optarg,
			struct internal_config *conf)
{
	switch (opt) {
	/* size of memory */
	case 'm':
		conf->memory = atoi(optarg);
		conf->memory *= 1024ULL;
		conf->memory *= 1024ULL;
		break;
	/* force number of channels */
	case 'n':
		conf->force_nchannel = atoi(optarg);
		if (conf->force_nchannel == 0) {
			RTE_LOG(ERR, EAL, "invalid channel number\n");
			return -1;
		}
		break;
	/* force number of ranks */
	case 'r':
		conf->force_nrank = atoi(optarg);
		if (conf->force_nrank == 0 ||
		    conf->force_nrank > 16) {
			RTE_LOG(ERR, EAL, "invalid rank number\n");
			return -1;
		}
		break;
	case 'v':
		/* since message is explicitly requested by user, we
		 * write message at highest log level so it can always
		 * be seen
		 * even if info or warning messages are disabled */
		RTE_LOG(CRIT, EAL, "RTE Version: '%s'\n", rte_version());
		break;

	/* long options */
	case OPT_HUGE_UNLINK_NUM:
		conf->hugepage_unlink = 1;
		break;

	case OPT_NO_HUGE_NUM:
		conf->no_hugetlbfs = 1;
		break;

	case OPT_PROC_TYPE_NUM:
		conf->process_type = eal_parse_proc_type(optarg);
		break;

	case OPT_MASTER_LCORE_NUM:
		if (eal_parse_master_lcore(optarg) < 0) {
			RTE_LOG(ERR, EAL, "invalid parameter for --"
					OPT_MASTER_LCORE "\n");
			return -1;
		}
		break;

	case OPT_SYSLOG_NUM:
		if (eal_parse_syslog(optarg, conf) < 0) {
			RTE_LOG(ERR, EAL, "invalid parameters for --"
					OPT_SYSLOG "\n");
			return -1;
		}
		break;

	case OPT_LOG_LEVEL_NUM: {
		if (eal_parse_log_level(optarg) < 0) {
			RTE_LOG(ERR, EAL,
				"invalid parameters for --"
				OPT_LOG_LEVEL "\n");
			return -1;
		}
		break;
	}

	/* don't know what to do, leave this to caller */
	default:
		return 1;

	}

	return 0;
bw_used:
	RTE_LOG(ERR, EAL, "Options blacklist (-b) and whitelist (-w) "
		"cannot be used at the same time\n");
	return -1;
}

void
eal_common_usage(void)
{
	printf("[options]\n\n"
	       "EAL common options:\n"
	       "  --"OPT_MASTER_LCORE" ID   Core ID that is used as master\n"
	       "  -n CHANNELS         Number of memory channels\n"
	       "  -m MB               Memory to allocate (see also --"OPT_SOCKET_MEM")\n"
	       "  -r RANKS            Force number of memory ranks (don't detect)\n"
	       "  --"OPT_PROC_TYPE"         Type of this process (primary|secondary|auto)\n"
	       "  --"OPT_SYSLOG"            Set syslog facility\n"
	       "  --"OPT_LOG_LEVEL"=<int>   Set global log level\n"
	       "  --"OPT_LOG_LEVEL"=<type-regexp>,<int>\n"
	       "                      Set specific log level\n"
	       "  -v                  Display version information on startup\n"
	       "  -h, --help          This help\n"
	       "\nEAL options for DEBUG use only:\n"
	       "  --"OPT_HUGE_UNLINK"       Unlink hugepage files after init\n"
	       "  --"OPT_NO_HUGE"           Use malloc instead of hugetlbfs\n"
	       "\n", RTE_MAX_LCORE);
}
