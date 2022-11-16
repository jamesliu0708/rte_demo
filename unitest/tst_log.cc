#include "rte_log.h"

#define RTE_LOGTYPE_TESTAPP1 RTE_LOGTYPE_USER1
#define RTE_LOGTYPE_TESTAPP2 RTE_LOGTYPE_USER2

/*
 * Logs
 * ====
 *
 * - Enable log types.
 * - Set log level.
 * - Send logs with different types and levels, some should not be displayed.
 */
static int
test_legacy_logs(void)
{
	printf("== static log types\n");

	/* set logtype level low to so we can test global level */
	rte_log_set_level(RTE_LOGTYPE_TESTAPP1, RTE_LOG_DEBUG);
	rte_log_set_level(RTE_LOGTYPE_TESTAPP2, RTE_LOG_DEBUG);

	/* log in error level */
	rte_log_set_global_level(RTE_LOG_ERR);
	RTE_LOG(ERR, TESTAPP1, "error message\n");
	RTE_LOG(CRIT, TESTAPP1, "critical message\n");

	/* log in critical level */
	rte_log_set_global_level(RTE_LOG_CRIT);
	RTE_LOG(ERR, TESTAPP2, "error message (not displayed)\n");
	RTE_LOG(CRIT, TESTAPP2, "critical message\n");

	/* bump up single log type level above global to test it */
	rte_log_set_level(RTE_LOGTYPE_TESTAPP2, RTE_LOG_EMERG);

	/* log in error level */
	rte_log_set_global_level(RTE_LOG_ERR);
	RTE_LOG(ERR, TESTAPP1, "error message\n");
	RTE_LOG(ERR, TESTAPP2, "error message (not displayed)\n");

	return 0;
}

static int
test_logs(void)
{
	int logtype1, logtype2;
	int ret;

	printf("== dynamic log types\n");

	logtype1 = rte_log_register("logtype1");
	if (logtype1 < 0) {
		printf("Cannot register logtype1\n");
		return -1;
	}
	logtype2 = rte_log_register("logtype2");
	if (logtype2 < 0) {
		printf("Cannot register logtype2\n");
		return -1;
	}

	/* set logtype level low to so we can test global level */
	rte_log_set_level(logtype1, RTE_LOG_DEBUG);
	rte_log_set_level(logtype2, RTE_LOG_DEBUG);

	/* log in error level */
	rte_log_set_global_level(RTE_LOG_ERR);
	rte_log(RTE_LOG_ERR, logtype1, "error message\n");
	rte_log(RTE_LOG_CRIT, logtype1, "critical message\n");

	/* log in critical level */
	rte_log_set_global_level(RTE_LOG_CRIT);
	rte_log(RTE_LOG_ERR, logtype2, "error message (not displayed)\n");
	rte_log(RTE_LOG_CRIT, logtype2, "critical message\n");

	/* bump up single log type level above global to test it */
	rte_log_set_level(logtype2, RTE_LOG_EMERG);

	/* log in error level */
	rte_log_set_global_level(RTE_LOG_ERR);
	rte_log(RTE_LOG_ERR, logtype1, "error message\n");
	rte_log(RTE_LOG_ERR, logtype2, "error message (not displayed)\n");

	return 0;
}

int main (int argc,char**argv) {
    test_legacy_logs();
    test_logs();
}