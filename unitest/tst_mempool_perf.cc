#include "rte_eal.h"
#include "rte_malloc.h"
#include "rte_mempool.h"

#include <gtest/gtest.h>

#define N 65536
#define TIME_S 5
#define MEMPOOL_ELT_SIZE 2048
#define MAX_KEEP 128
#define MEMPOOL_SIZE ((rte_lcore_count()*(MAX_KEEP+RTE_MEMPOOL_CACHE_MAX_SIZE))-1)

#define LOG_ERR() printf("test failed at %s():%d\n", __func__, __LINE__)
#define RET_ERR() do {							\
		LOG_ERR();						\
		return -1;						\
	} while (0)
#define GOTO_ERR(var, label) do {					\
		LOG_ERR();						\
		var = -1;						\
		goto label;						\
	} while (0)

static int use_external_cache;
static unsigned external_cache_size = RTE_MEMPOOL_CACHE_MAX_SIZE;
static rte_atomic32_t synchro;

/* number of objects in one bulk operation (get or put) */
static unsigned n_get_bulk;
static unsigned n_put_bulk;

/* number of objects retrived from mempool before putting them back */
static unsigned n_keep;

/* number of enqueues / dequeues */
struct mempool_test_stats {
	uint64_t enq_count;
} __rte_cache_aligned;

struct mempool_perf_struct {
    struct rte_mempool *mp;
    int core;
};

static struct mempool_test_stats stats[RTE_MAX_LCORE];

void*
per_lcore_mempool_test(void *arg)
{
    void *obj_table[MAX_KEEP];
    int* pret = (int*)malloc(sizeof(int));
    struct mempool_perf_struct* mps = (struct mempool_perf_struct*)arg;
	unsigned i, idx;
	struct rte_mempool *mp = mps->mp;
	unsigned lcore_id = mps->core;
	int ret = 0;
	uint64_t start_cycles, end_cycles;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(lcore_id, &cpuset);
    struct rte_mempool_cache *cache;

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        fprintf(stderr, "cpu %d is not runing\n", lcore_id);
        perror("");
		*pret = -1;
		pthread_exit((void*)ret);
        return NULL;
    }

    if (use_external_cache) {
		/* Create a user-owned mempool cache. */
		cache = rte_mempool_cache_create(external_cache_size,
						 SOCKET_ID_ANY);
		if (cache == NULL) {
            *pret = -1;
            pthread_exit((void*)ret);
            return NULL;
        }
	} else {
		/* May be NULL if cache is disabled. */
		cache = rte_mempool_default_cache(mp, lcore_id);
	}

    /* n_get_bulk and n_put_bulk must be divisors of n_keep */
	if (((n_keep / n_get_bulk) * n_get_bulk) != n_keep)
		GOTO_ERR(ret, out);
	if (((n_keep / n_put_bulk) * n_put_bulk) != n_keep)
		GOTO_ERR(ret, out);
    
    stats[lcore_id].enq_count = 0;

    
}

/* launch all the per-lcore test, and display the result */
static int
launch_cores(struct rte_mempool *mp, unsigned int cores)
{
    unsigned lcore_id;
	uint64_t rate;
	int ret;
	unsigned cores_save = cores;
    int ncpus;
    pthread_t tid_list[RTE_MAX_LCORE];

    rte_atomic32_set(&synchro, 0);

    /* reset stats */
	memset(stats, 0, sizeof(stats));

	printf("mempool_autotest cache=%u cores=%u n_get_bulk=%u "
	       "n_put_bulk=%u n_keep=%u ",
	       use_external_cache ?
		   external_cache_size : (unsigned) mp->cache_size,
	       cores, n_get_bulk, n_put_bulk, n_keep);
    
    if (rte_mempool_avail_count(mp) != MEMPOOL_SIZE) {
		printf("mempool is not full\n");
		return -1;
	}
    ncpus = sysconf( _SC_NPROCESSORS_CONF);
    for(int i = 0; i < ncpus; ++i) {
        struct mempool_perf_struct mps = {
            .mp = mp,
            .core = i
        };
        ret = pthread_create(&tid_list[i], NULL, per_lcore_mempool_test, &mps);
        if (ret != 0) {
            return ret;
        }
    }

    for(int i = 0; i < ncpus; ++i)
    {
        int* pret;
		pthread_join(tid_list[i], (void**)&pret);
		if(*pret, 0) {
            return *pret;
        }
    }

    free(tid_list);
}   

/*
 * save the object number in the first 4 bytes of object data. All
 * other bytes are set to 0.
 */
static void
my_obj_init(struct rte_mempool *mp, __attribute__((unused)) void *arg,
	    void *obj, unsigned i)
{
	uint32_t *objnum = (uint32_t*)obj;
	memset(obj, 0, mp->elt_size);
	*objnum = i;
}

static int 
do_one_mempool_nocache_test(int cores)
{
    struct rte_mempool *mp = rte_mempool_create("perf_test_nocache", MEMPOOL_SIZE,
					MEMPOOL_ELT_SIZE, 0, 0,
					NULL, NULL,
					my_obj_init, NULL,
					SOCKET_ID_ANY, 0);
    if (mp == NULL) {
        fprintf(stderr, "Failed to create mempool\n");
        return -1;
    }

    unsigned bulk_tab_get[] = { 1, 4, 32, 0 };
	unsigned bulk_tab_put[] = { 1, 4, 32, 0 };
	unsigned keep_tab[] = { 32, 128, 0 };
	unsigned *get_bulk_ptr;
	unsigned *put_bulk_ptr;
	unsigned *keep_ptr;
	int ret;

    for (get_bulk_ptr = bulk_tab_get; *get_bulk_ptr; get_bulk_ptr++) {
		for (put_bulk_ptr = bulk_tab_put; *put_bulk_ptr; put_bulk_ptr++) {
			for (keep_ptr = keep_tab; *keep_ptr; keep_ptr++) {

				n_get_bulk = *get_bulk_ptr;
				n_put_bulk = *put_bulk_ptr;
				n_keep = *keep_ptr;
				ret = launch_cores(mp, cores);

				if (ret < 0)
					return -1;
			}
		}
	}
    return 0;
}

static int
do_one_mempool_cache_test(int cores)
{

}

TEST(test_rte, do_one_mempool_nocache_test)
{
    ASSERT_EQ(do_one_mempool_nocache_test(1), 0);
    ASSERT_EQ(do_one_mempool_nocache_test(2), 0);
    int ncpus = sysconf( _SC_NPROCESSORS_CONF);
    ASSERT_EQ(do_one_mempool_nocache_test(ncpus), 0);
}

int main(int argc, char** argv)
{
#ifdef MASTER
    int ret = rte_eal_init(argc, argv);
	if (ret < 0) {
        fprintf(stderr, "Failed to init rte\n");
        exit(-1);
    }
#else
	int ret = rte_eal_attach(argc, argv);
	if (ret < 0) {
		fprintf(stderr, "Failed to attach rte\n");
		exit(-1);
	}
#endif // MASTER
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
