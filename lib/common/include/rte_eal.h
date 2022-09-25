/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2018 Intel Corporation
 */

#ifndef _RTE_EAL_H_
#define _RTE_EAL_H_

/**
 * @file
 *
 * EAL Configuration API
 */

#include <stdint.h>
#include <sched.h>
#include <time.h>

#include <rte_config.h>
#include <rte_per_lcore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTE_MAGIC 19820526 /**< Magic number written by the main partition when ready. */

/* Maximum thread_name length. */
#define RTE_MAX_THREAD_NAME_LEN 16

/**
 * The lcore role (used in RTE or not).
 */
enum rte_lcore_role_t {
	ROLE_RTE,
	ROLE_OFF,
	ROLE_SERVICE,
};

/**
 * The type of process in a linuxapp, multi-process setup
 */
enum rte_proc_type_t {
	RTE_PROC_AUTO = -1,   /* allow auto-detection of primary/secondary */
	RTE_PROC_PRIMARY = 0, /* set to zero, so primary is the default */
	RTE_PROC_SECONDARY,

	RTE_PROC_INVALID
};

/**
 * The global RTE configuration structure.
 */
struct rte_config {
	uint32_t master_lcore;       /**< Id of the master lcore */
	uint32_t lcore_count;        /**< Number of available logical cores. */
	uint32_t service_lcore_count;/**< Number of available service cores. */
	enum rte_lcore_role_t lcore_role[RTE_MAX_LCORE]; /**< State of cores. */

	/** Primary or secondary configuration */
	enum rte_proc_type_t process_type;

	/**
	 * Pointer to memory configuration, which may be shared across multiple
	 * DPDK instances
	 */
	struct rte_mem_config *mem_config;
} __attribute__((__packed__));

/**
 * Get the global configuration structure.
 *
 * @return
 *   A pointer to the global configuration structure.
 */
struct rte_config *rte_eal_get_configuration(void);

/**
 * Get a lcore's role.
 *
 * @param lcore_id
 *   The identifier of the lcore.
 * @return
 *   The role of the lcore.
 */
enum rte_lcore_role_t rte_eal_lcore_role(unsigned lcore_id);


/**
 * Get the process type in a multi-process setup
 *
 * @return
 *   The process type
 */
enum rte_proc_type_t rte_eal_process_type(void);

/**
 * Request iopl privilege for all RPL.
 *
 * This function should be called by pmds which need access to ioports.

 * @return
 *   - On success, returns 0.
 *   - On failure, returns -1.
 */
int rte_eal_iopl_init(void);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice
 *
 * Clean up the Environment Abstraction Layer (EAL)
 *
 * This function must be called to release any internal resources that EAL has
 * allocated during rte_eal_init(). After this call, no DPDK function calls may
 * be made. It is expected that common usage of this function is to call it
 * just before terminating the process.
 *
 * @return 0 Successfully released all internal EAL resources
 * @return -EFAULT There was an error in releasing all resources.
 */
int __rte_experimental rte_eal_cleanup(void);

/**
 * Check if a primary process is currently alive
 *
 * This function returns true when a primary process is currently
 * active.
 *
 * @param config_file_path
 *   The config_file_path argument provided should point at the location
 *   that the primary process will create its config file. If NULL, the default
 *   config file path is used.
 *
 * @return
 *  - If alive, returns 1.
 *  - If dead, returns 0.
 */
int rte_eal_primary_proc_alive(const char *config_file_path);

/**
 * Usage function typedef used by the application usage function.
 *
 * Use this function typedef to define and call rte_set_application_usage_hook()
 * routine.
 */
typedef void	(*rte_usage_hook_t)(const char * prgname);

/**
 * Add application usage routine callout from the eal_usage() routine.
 *
 * This function allows the application to include its usage message
 * in the EAL system usage message. The routine rte_set_application_usage_hook()
 * needs to be called before the rte_eal_init() routine in the application.
 *
 * This routine is optional for the application and will behave as if the set
 * routine was never called as the default behavior.
 *
 * @param usage_func
 *   The func argument is a function pointer to the application usage routine.
 *   Called function is defined using rte_usage_hook_t typedef, which is of
 *   the form void rte_usage_func(const char * prgname).
 *
 *   Calling this routine with a NULL value will reset the usage hook routine and
 *   return the current value, which could be NULL.
 * @return
 *   - Returns the current value of the rte_application_usage pointer to allow
 *     the caller to daisy chain the usage routines if needing more then one.
 */
rte_usage_hook_t
rte_set_application_usage_hook(rte_usage_hook_t usage_func);

/**
 * macro to get the lock of tailq in mem_config
 */
#define RTE_EAL_TAILQ_RWLOCK         (&rte_eal_get_configuration()->mem_config->qlock)

/**
 * macro to get the multiple lock of mempool shared by mutiple-instance
 */
#define RTE_EAL_MEMPOOL_RWLOCK            (&rte_eal_get_configuration()->mem_config->mplock)

/**
 * Whether EAL is using huge pages (disabled by --no-huge option).
 * The no-huge mode cannot be used with UIO poll-mode drivers like igb/ixgbe.
 * It is useful for NIC drivers (e.g. librte_pmd_mlx4, librte_pmd_vmxnet3) or
 * crypto drivers (e.g. librte_crypto_nitrox) provided by third-parties such
 * as 6WIND.
 *
 * @return
 *   Nonzero if hugepages are enabled.
 */
int rte_eal_has_hugepages(void);

/**
 * A wrap API for syscall gettid.
 *
 * @return
 *   On success, returns the thread ID of calling process.
 *   It is always successful.
 */
int rte_sys_gettid(void);

/**
 * Get system unique thread id.
 *
 * @return
 *   On success, returns the thread ID of calling process.
 *   It is always successful.
 */
static inline int rte_gettid(void)
{
	static RTE_DEFINE_PER_LCORE(int, _thread_id) = -1;
	if (RTE_PER_LCORE(_thread_id) == -1)
		RTE_PER_LCORE(_thread_id) = rte_sys_gettid();
	return RTE_PER_LCORE(_thread_id);
}

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice
 *
 * Get user provided pool ops name for mbuf
 *
 * @return
 *   returns user provided pool ops name.
 */
const char * __rte_experimental
rte_eal_mbuf_user_pool_ops(void);

/**
 * Get default pool ops name for mbuf
 *
 * @return
 *   returns default pool ops name.
 */
const char *
rte_eal_mbuf_default_mempool_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_EAL_H_ */
