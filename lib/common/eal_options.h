/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2014 6WIND S.A.
 */

#ifndef EAL_OPTIONS_H
#define EAL_OPTIONS_H

enum {
	/* long options mapped to a short option */
#define OPT_HELP              "help"
	OPT_HELP_NUM            = 'h',

	/* first long only option value must be >= 256, so that we won't
	 * conflict with short options */
	OPT_LONG_MIN_NUM = 256,
#define OPT_BASE_VIRTADDR     "base-virtaddr"
	OPT_BASE_VIRTADDR_NUM,
#define OPT_FILE_PREFIX       "file-prefix"
	OPT_FILE_PREFIX_NUM,
#define OPT_HUGE_DIR          "huge-dir"
	OPT_HUGE_DIR_NUM,
#define OPT_HUGE_UNLINK       "huge-unlink"
	OPT_HUGE_UNLINK_NUM,
#define OPT_LOG_LEVEL         "log-level"
	OPT_LOG_LEVEL_NUM,
#define OPT_MASTER_LCORE      "master-lcore"
	OPT_MASTER_LCORE_NUM,
#define OPT_PROC_TYPE         "proc-type"
	OPT_PROC_TYPE_NUM,
#define OPT_NO_HUGE           "no-huge"
	OPT_NO_HUGE_NUM,
#define OPT_SOCKET_MEM        "socket-mem"
	OPT_SOCKET_MEM_NUM,
#define OPT_SYSLOG            "syslog"
	OPT_SYSLOG_NUM,
	OPT_LONG_MAX_NUM
};

extern const char eal_short_options[];
extern const struct option eal_long_options[];

int eal_parse_common_option(int opt, const char *argv,
			    struct internal_config *conf);
int eal_adjust_config(struct internal_config *internal_cfg);
int eal_check_common_options(struct internal_config *internal_cfg);
void eal_common_usage(void);
enum rte_proc_type_t eal_proc_type_detect(void);

#endif /* EAL_OPTIONS_H */
