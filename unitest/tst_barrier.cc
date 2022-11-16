 /*
  * This is a simple functional test for rte_smp_mb() implementation.
  * I.E. make sure that LOAD and STORE operations that precede the
  * rte_smp_mb() call are globally visible across the lcores
  * before the the LOAD and STORE operations that follows it.
  * The test uses simple implementation of Peterson's lock algorithm
  * (https://en.wikipedia.org/wiki/Peterson%27s_algorithm)
  * for two execution units to make sure that rte_smp_mb() prevents
  * store-load reordering to happen.
  * Also when executed on a single lcore could be used as a approxiamate
  * estimation of number of cycles particular implementation of rte_smp_mb()
  * will take.
  */
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include <gtest/gtest.h>

#include "rte_atomic.h"
#include "rte_common.h"

#define ADD_MAX		8
#define ITER_MAX	0x1000000

enum plock_use_type {
	USE_MB,
	USE_SMP_MB,
	USE_NUM
};

struct plock {
	volatile uint32_t flag[2];
	volatile uint32_t victim;
	enum plock_use_type utype;
};

/*
 * Lock plus protected by it two counters.
 */
struct plock_test {
	struct plock lock;
	uint32_t val;
	uint32_t iter;
};

/*
 * For N active lcores we allocate N+1 lcore_plock_test structures.
 * Each active lcore shares one lcore_plock_test structure with its
 * left lcore neighbor and one lcore_plock_test structure with its
 * right lcore neighbor.
 * During the test each lcore updates data in both shared structures and
 * its local copies. Then at validation phase we check that our shared
 * and local data are the same.
 */
static int
plock_test(uint32_t iter, enum plock_use_type utype)
{

}

TEST(test_rte, test_barrier)
{
	int32_t i, ret, rc[USE_NUM];

	for (i = 0; i != RTE_DIM(rc); i++)
		rc[i] = plock_test(ITER_MAX, (plock_use_type)i);

	for (i = 0; i != RTE_DIM(rc); i++) {
		printf("%s for utype=%d %s\n",
			__func__, i, rc[i] == 0 ? "passed" : "failed");
	}

}

int main(int argc,char**argv){

  testing::InitGoogleTest(&argc,argv);

  return RUN_ALL_TESTS();
}