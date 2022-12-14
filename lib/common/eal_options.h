/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2014 6WIND S.A.
 */

#ifndef EAL_OPTIONS_H
#define EAL_OPTIONS_H

enum {
	/* long options mapped to a short option */
#define OPT_HELP              "help"
	OPT_HELP_NUM            = 'h',
#define OPT_PCI_BLACKLIST     "pci-blacklist"
	OPT_PCI_BLACKLIST_NUM   = 'b',
#define OPT_PCI_WHITELIST     "pci-whitelist"
	OPT_PCI_WHITELIST_NUM   = 'w',

	/* first long only option value must be >= 256, so that we won't
	 * conflict with short options */
	OPT_LONG_MIN_NUM = 256,
#define OPT_BASE_VIRTADDR     "base-virtaddr"
	OPT_BASE_VIRTADDR_NUM,
#define OPT_CREATE_UIO_DEV    "create-uio-dev"
	OPT_CREATE_UIO_DEV_NUM,
#define OPT_FILE_PREFIX       "file-prefix"
	OPT_FILE_PREFIX_NUM,
#define OPT_HUGE_DIR          "huge-dir"
	OPT_HUGE_DIR_NUM,
#define OPT_HUGE_UNLINK       "huge-unlink"
	OPT_HUGE_UNLINK_NUM,
#define OPT_LCORES            "lcores"
	OPT_LCORES_NUM,
#define OPT_LOG_LEVEL         "log-level"
	OPT_LOG_LEVEL_NUM,
#define OPT_MASTER_LCORE      "master-lcore"
	OPT_MASTER_LCORE_NUM,
#define OPT_MBUF_POOL_OPS_NAME "mbuf-pool-ops-name"
	OPT_MBUF_POOL_OPS_NAME_NUM,
#define OPT_PROC_TYPE         "proc-type"
	OPT_PROC_TYPE_NUM,
#define OPT_NO_HPET           "no-hpet"
	OPT_NO_HPET_NUM,
#define OPT_NO_HUGE           "no-huge"
	OPT_NO_HUGE_NUM,
#define OPT_NO_PCI            "no-pci"
	OPT_NO_PCI_NUM,
#define OPT_NO_SHCONF         "no-shconf"
	OPT_NO_SHCONF_NUM,
#define OPT_SOCKET_MEM        "socket-mem"
	OPT_SOCKET_MEM_NUM,
#define OPT_SYSLOG            "syslog"
	OPT_SYSLOG_NUM,
#define OPT_VDEV              "vdev"
	OPT_VDEV_NUM,
#define OPT_VFIO_INTR         "vfio-intr"
	OPT_VFIO_INTR_NUM,
#define OPT_VMWARE_TSC_MAP    "vmware-tsc-map"
	OPT_VMWARE_TSC_MAP_NUM,
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
