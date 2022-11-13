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

#define LCORE_ID_ANY     UINT32_MAX       /**< Any lcore. */

#if defined(__linux__)
	typedef	cpu_set_t rte_cpuset_t;
#elif defined(__FreeBSD__)
#include <pthread_np.h>
	typedef cpuset_t rte_cpuset_t;
#endif

/**
 * The lcore role (used in RTE or not).
 */
enum rte_lcore_role_t {
	ROLE_OFF,
	ROLE_RTE,
};

/**
 * Structure storing internal configuration (per-lcore)
 */
struct lcore_config {
	unsigned detected;         /**< true if lcore was detected */
	unsigned socket_id;        /**< physical socket id for this lcore */
	unsigned core_id;          /**< core number on socket for this lcore */
	int core_index;            /**< relative index, starting from 0 */
	rte_cpuset_t cpuset;       /**< cpu set which the lcore affinity to */
	uint8_t core_role;         /**< role of core eg: OFF, RTE */
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
 * The configuration structure for the cpu num, role, stat
 */
struct rte_cpuinfo {
	uint32_t lcore_count;        /**< Number of available logical cores. */
	enum rte_lcore_role_t lcore_role[RTE_MAX_LCORE]; /**< State of cores. */
	struct lcore_config lcore_config[RTE_MAX_LCORE]; /**< Internal configuration */

	/* address of mem_config in primary process. used to map shared config into
	 * exact same address the primary process maps it.
	 */
	uint64_t cpu_cfg_addr;
};

/**
 * The global RTE configuration structure.
 */
struct rte_config {

	/** Primary or secondary configuration */
	enum rte_proc_type_t process_type;

	/**
	 * Pointer to cpu configuration, which may be shared across multiple instance
	 */
	struct rte_cpuinfo *cpu_config;

	/**
	 * Pointer to cpu configuration, which may be shared across multiple
	 *  instances
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
int rte_eal_cleanup(void);

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
 * Attach the Environment Abstraction Layer (EAL)
 * This function is to be executed on the PRIMARY lcore only, as soon
 * as possible in the application's main() function.
 *  
  * @param argc
 *   A non-negative value.  If it is greater than 0, the array members
 *   for argv[0] through argv[argc] (non-inclusive) shall contain pointers
 *   to strings.
 * @param argv
 *   An array of strings.  The contents of the array, as well as the strings
 *   which are pointed to by the array, may be modified by this function.
 * @return
 *   - On success, the number of parsed arguments, which is greater or
 *     equal to zero. After the call to rte_eal_init(),
 *     all arguments argv[x] with x < ret may have been modified by this
 *     function call and should not be further interpreted by the
 *     application.  The EAL does not take any ownership of the memory used
 *     for either the argv array, or its members.
 *   - On failure, -1 and rte_errno is set to a value indicating the cause
 *     for failure.  In some instances, the application will need to be
 *     restarted as part of clearing the issue.
 *
 *   Error codes returned via rte_errno:
 *     EACCES indicates a permissions issue.
 *
 *     EAGAIN indicates either a bus or system resource was not available,
 *            setup may be attempted again.
 *
 *     EALREADY indicates that the rte_eal_init function has already been
 *              called, and cannot be called again.
 *
 *     EFAULT indicates the tailq configuration name was not found in
 *            memory configuration.
 *
 *     EINVAL indicates invalid parameters were passed as argv/argc.
 *
 *     ENOMEM indicates failure likely caused by an out-of-memory condition.
 *
 *     ENODEV indicates memory setup issues.
 *
 *     ENOTSUP indicates that the EAL cannot initialize on this system.
 *
 *     EPROTO indicates that the PCI bus is either not present, or is not
 *            readable by the eal.
 *
 *     ENOEXEC indicates that a service core failed to launch successfully.
 */
int rte_eal_attach(int argc, char** argv);

/**
 * Initialize the Environment Abstraction Layer (EAL).
 *
 * This function is to be executed on the MASTER lcore only, as soon
 * as possible in the application's main() function.
 *
 * The function finishes the initialization process before main() is called.
 * It puts the SLAVE lcores in the WAIT state.
 *
 * When the multi-partition feature is supported, depending on the
 * configuration (if CONFIG_RTE_EAL_MAIN_PARTITION is disabled), this
 * function waits to ensure that the magic number is set before
 * returning. See also the rte_eal_get_configuration() function. Note:
 * This behavior may change in the future.
 *
 * @param argc
 *   A non-negative value.  If it is greater than 0, the array members
 *   for argv[0] through argv[argc] (non-inclusive) shall contain pointers
 *   to strings.
 * @param argv
 *   An array of strings.  The contents of the array, as well as the strings
 *   which are pointed to by the array, may be modified by this function.
 * @return
 *   - On success, the number of parsed arguments, which is greater or
 *     equal to zero. After the call to rte_eal_init(),
 *     all arguments argv[x] with x < ret may have been modified by this
 *     function call and should not be further interpreted by the
 *     application.  The EAL does not take any ownership of the memory used
 *     for either the argv array, or its members.
 *   - On failure, -1 and rte_errno is set to a value indicating the cause
 *     for failure.  In some instances, the application will need to be
 *     restarted as part of clearing the issue.
 *
 *   Error codes returned via rte_errno:
 *     EACCES indicates a permissions issue.
 *
 *     EAGAIN indicates either a bus or system resource was not available,
 *            setup may be attempted again.
 *
 *     EALREADY indicates that the rte_eal_init function has already been
 *              called, and cannot be called again.
 *
 *     EFAULT indicates the tailq configuration name was not found in
 *            memory configuration.
 *
 *     EINVAL indicates invalid parameters were passed as argv/argc.
 *
 *     ENOMEM indicates failure likely caused by an out-of-memory condition.
 *
 *     ENODEV indicates memory setup issues.
 *
 *     ENOTSUP indicates that the EAL cannot initialize on this system.
 *
 *     EPROTO indicates that the PCI bus is either not present, or is not
 *            readable by the eal.
 *
 *     ENOEXEC indicates that a service core failed to launch successfully.
 */
int rte_eal_init(int argc, char **argv);

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

#ifdef __cplusplus
}
#endif

#endif /* _RTE_EAL_H_ */
