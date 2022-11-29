#include "rte_eal.h"
#include "rte_malloc.h"
#include "rte_mempool.h"

#include <gtest/gtest.h>

#define MEMPOOL_ELT_SIZE 2048
#define MAX_KEEP 16
#define MEMPOOL_SIZE ((rte_lcore_count()*(MAX_KEEP+RTE_MEMPOOL_CACHE_MAX_SIZE))-1)

static inline uint64_t
rte_rdtsc(void)
{
	union {
		uint64_t tsc_64;
		RTE_STD_C11
		struct {
			uint32_t lo_32;
			uint32_t hi_32;
		};
	} tsc;

	asm volatile("rdtsc" :
		     "=a" (tsc.lo_32),
		     "=d" (tsc.hi_32));
	return tsc.tsc_64;
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

static void
walk_cb(struct rte_mempool *mp, void *userdata __rte_unused)
{
	printf("\t%s\n", mp->name);
}

TEST(test_rte, test_mempool_ops)
{
    int ret = -1;
	struct rte_mempool *mp_cache = NULL;
	struct rte_mempool *mp_nocache = NULL;
	struct rte_mempool *mp_stack = NULL;
	struct rte_mempool *default_pool = NULL;

    /* create a mempool (without cache) */
	mp_nocache = rte_mempool_create("test_nocache", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE, 0, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);
    
    ASSERT_NE((uintptr_t)mp_nocache, NULL);

    /* create a mempool (with cache) */
	mp_cache = rte_mempool_create("test_cache", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE,
		RTE_MEMPOOL_CACHE_MAX_SIZE, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)mp_cache, NULL);

	ASSERT_EQ((uintptr_t)rte_mempool_lookup("test_nocache"), (uintptr_t)mp_nocache);
	ASSERT_EQ((uintptr_t)rte_mempool_lookup("test_cache"), (uintptr_t)mp_cache);

	printf("Walk into mempools:\n");
	rte_mempool_walk(walk_cb, NULL);
	printf("Dump mempool\n");
	rte_mempool_list_dump(stdout);
	rte_mempool_free(mp_nocache);
	rte_mempool_free(mp_cache);

}

TEST(test_rte, test_nocache_use_external_cache_mempool_basic)
{
	uint32_t *objnum;
	void **objtable;
	void *obj, *obj2;
	char *obj_data;
	int ret = 0;
	unsigned i, j;
	int offset;
	struct rte_mempool_cache *cache;
	struct rte_mempool *mp_nocache = NULL;
	/* create a mempool (with cache) */
	mp_nocache = rte_mempool_create("test_nocache", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE, 0, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)mp_nocache, NULL);

	cache = rte_mempool_cache_create(RTE_MEMPOOL_CACHE_MAX_SIZE,
						 SOCKET_ID_ANY);
	ASSERT_NE((uintptr_t)cache, NULL);

	/* dump the mempool status */
	rte_mempool_dump(stdout, mp_nocache);

	printf("get an object\n");
	ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &obj, 1, cache), 0);
	rte_mempool_dump(stdout, mp_nocache);

	/* tests that improve coverage */
	printf("get object count\n");
	offset = cache->len;
	/* We have to count the extra caches, one in this case. */
	ASSERT_EQ(rte_mempool_avail_count(mp_nocache) + offset, MEMPOOL_SIZE - 1);

	printf("get private data\n");
	ASSERT_EQ((uintptr_t)rte_mempool_get_priv(mp_nocache), (uintptr_t)((char *)mp_nocache +
			MEMPOOL_HEADER_SIZE(mp_nocache, mp_nocache->cache_size)));
	
	printf("put the object back\n");
	rte_mempool_generic_put(mp_nocache, &obj, 1, cache);
	rte_mempool_dump(stdout, mp_nocache);

	printf("get 2 objects\n");
	ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &obj, 1, cache), 0);
	ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &obj2, 1, cache), 0);
	rte_mempool_dump(stdout, mp_nocache);

	printf("put the objects back\n");
	rte_mempool_generic_put(mp_nocache, &obj, 1, cache);
	rte_mempool_generic_put(mp_nocache, &obj2, 1, cache);
	rte_mempool_dump(stdout, mp_nocache);

	objtable = (void**)malloc(MEMPOOL_SIZE * sizeof(void *));
	ASSERT_NE((uintptr_t)objtable, NULL);
	for (i = 0; i < MEMPOOL_SIZE; i++) {
		ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &objtable[i], 1, cache), 0);
	}

	/*
	 * for each object, check that its content was not modified,
	 * and put objects back in pool
	 */
	while (i--) {
		obj = objtable[i];
		obj_data = (char*)obj;
		objnum = (uint32_t*)obj;
		ASSERT_LE(*objnum, MEMPOOL_SIZE);
		for (j = sizeof(*objnum); j < mp_nocache->elt_size; j++) {
			ASSERT_EQ(obj_data[j], 0);
		}
		rte_mempool_generic_put(mp_nocache, &objtable[i], 1, cache);
	}

	free(objtable);
	rte_mempool_cache_flush(cache, mp_nocache);
	rte_mempool_cache_free(cache);
	rte_mempool_free(mp_nocache);
}

TEST(test_rte, test_nocache_mempool_basic)
{
	uint32_t *objnum;
	void **objtable;
	void *obj, *obj2;
	char *obj_data;
	int ret = 0;
	unsigned i, j;
	struct rte_mempool_cache *cache;
	struct rte_mempool *mp_nocache = NULL;
	/* create a mempool (with cache) */
	mp_nocache = rte_mempool_create("test_nocache", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE, 0, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)mp_nocache, NULL);

	cache = rte_mempool_default_cache(mp_nocache, 0);

	/* dump the mempool status */
	rte_mempool_dump(stdout, mp_nocache);

	printf("get an object\n");
	ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &obj, 1, cache), 0);
	rte_mempool_dump(stdout, mp_nocache);

	/* tests that improve coverage */
	printf("get object count\n");
	/* We have to count the extra caches, one in this case. */
	ASSERT_EQ(rte_mempool_avail_count(mp_nocache), MEMPOOL_SIZE - 1);

	printf("get private data\n");
	ASSERT_EQ((uintptr_t)rte_mempool_get_priv(mp_nocache), (uintptr_t)((char *)mp_nocache +
			MEMPOOL_HEADER_SIZE(mp_nocache, mp_nocache->cache_size)));
	
	printf("put the object back\n");
	rte_mempool_generic_put(mp_nocache, &obj, 1, cache);
	rte_mempool_dump(stdout, mp_nocache);

	printf("get 2 objects\n");
	ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &obj, 1, cache), 0);
	ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &obj2, 1, cache), 0);
	rte_mempool_dump(stdout, mp_nocache);

	printf("put the objects back\n");
	rte_mempool_generic_put(mp_nocache, &obj, 1, cache);
	rte_mempool_generic_put(mp_nocache, &obj2, 1, cache);
	rte_mempool_dump(stdout, mp_nocache);

	objtable = (void**)malloc(MEMPOOL_SIZE * sizeof(void *));
	ASSERT_NE((uintptr_t)objtable, NULL);
	for (i = 0; i < MEMPOOL_SIZE; i++) {
		ASSERT_EQ(rte_mempool_generic_get(mp_nocache, &objtable[i], 1, cache), 0);
	}

	/*
	 * for each object, check that its content was not modified,
	 * and put objects back in pool
	 */
	while (i--) {
		obj = objtable[i];
		obj_data = (char*)obj;
		objnum = (uint32_t*)obj;
		ASSERT_LE(*objnum, MEMPOOL_SIZE);
		for (j = sizeof(*objnum); j < mp_nocache->elt_size; j++) {
			ASSERT_EQ(obj_data[j], 0);
		}
		rte_mempool_generic_put(mp_nocache, &objtable[i], 1, cache);
	}

	free(objtable);
	rte_mempool_free(mp_nocache);
}

TEST(test_rte, test_cache_mempool_basic)
{
	uint32_t *objnum;
	void **objtable;
	void *obj, *obj2;
	char *obj_data;
	int ret = 0;
	unsigned i, j;
	struct rte_mempool_cache *cache;
	struct rte_mempool *mp_cache = NULL;
	/* create a mempool (with cache) */
	mp_cache = rte_mempool_create("test_cache", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE,
		RTE_MEMPOOL_CACHE_MAX_SIZE, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)mp_cache, NULL);

	cache = rte_mempool_default_cache(mp_cache, 0);

	/* dump the mempool status */
	rte_mempool_dump(stdout, mp_cache);

	printf("get an object\n");
	ASSERT_EQ(rte_mempool_generic_get(mp_cache, &obj, 1, cache), 0);
	rte_mempool_dump(stdout, mp_cache);

	/* tests that improve coverage */
	printf("get object count\n");
	/* We have to count the extra caches, one in this case. */
	ASSERT_EQ(rte_mempool_avail_count(mp_cache), MEMPOOL_SIZE - 1);

	printf("get private data\n");
	ASSERT_EQ((uintptr_t)rte_mempool_get_priv(mp_cache), (uintptr_t)((char *)mp_cache +
			MEMPOOL_HEADER_SIZE(mp_cache, mp_cache->cache_size)));
	
	printf("put the object back\n");
	rte_mempool_generic_put(mp_cache, &obj, 1, cache);
	rte_mempool_dump(stdout, mp_cache);

	printf("get 2 objects\n");
	ASSERT_EQ(rte_mempool_generic_get(mp_cache, &obj, 1, cache), 0);
	ASSERT_EQ(rte_mempool_generic_get(mp_cache, &obj2, 1, cache), 0);
	rte_mempool_dump(stdout, mp_cache);

	printf("put the objects back\n");
	rte_mempool_generic_put(mp_cache, &obj, 1, cache);
	rte_mempool_generic_put(mp_cache, &obj2, 1, cache);
	rte_mempool_dump(stdout, mp_cache);

	objtable = (void**)malloc(MEMPOOL_SIZE * sizeof(void *));
	ASSERT_NE((uintptr_t)objtable, NULL);
	for (i = 0; i < MEMPOOL_SIZE; i++) {
		ASSERT_EQ(rte_mempool_generic_get(mp_cache, &objtable[i], 1, cache), 0);
	}

	/*
	 * for each object, check that its content was not modified,
	 * and put objects back in pool
	 */
	while (i--) {
		obj = objtable[i];
		obj_data = (char*)obj;
		objnum = (uint32_t*)obj;
		ASSERT_LE(*objnum, MEMPOOL_SIZE);
		for (j = sizeof(*objnum); j < mp_cache->elt_size; j++) {
			ASSERT_EQ(obj_data[j], 0);
		}
		rte_mempool_generic_put(mp_cache, &objtable[i], 1, cache);
	}

	free(objtable);
	rte_mempool_free(mp_cache);
}

TEST(test_rte, test_mempool_basic_ex)
{
	unsigned i;
	void **obj;
	void *err_obj;
	int ret = -1;
	struct rte_mempool *mp = NULL;
	/* create a mempool (with cache) */
	mp = rte_mempool_create("test_nocache", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE, 0, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)mp, NULL);

	obj = (void**)rte_calloc("test_mempool_basic_ex", MEMPOOL_SIZE,
		sizeof(void *), 0);
	ASSERT_NE((uintptr_t)obj, NULL);
	printf("test_mempool_basic_ex now mempool (%s) has %u free entries\n",
		mp->name, rte_mempool_in_use_count(mp));
	ASSERT_EQ(rte_mempool_full(mp), 1);

	for (i = 0; i < MEMPOOL_SIZE; i ++) {
		ASSERT_EQ(rte_mempool_get(mp, &obj[i]), 0);
	}

	ASSERT_NE(rte_mempool_get(mp, &err_obj), 0);
	printf("number: %u\n", i);
	
	ASSERT_EQ(rte_mempool_empty(mp), 1);

	for (i = 0; i < MEMPOOL_SIZE; i++)
		rte_mempool_put(mp, obj[i]);

	ASSERT_NE(rte_mempool_empty(mp), 1);
	rte_mempool_free(mp);
}

static void
my_mp_init(struct rte_mempool *mp, __attribute__((unused)) void *arg)
{
	printf("mempool name is %s\n", mp->name);
	/* nothing to be implemented here*/
	return ;
}

static struct rte_mempool *mp_spsc;
static rte_spinlock_t scsp_spinlock;
static void *scsp_obj_table[MAX_KEEP];
static int duration = 1000000000;

/*
 * single producer function
 */
static int test_mempool_single_producer(void)
{
	unsigned int i;
	void *obj = NULL;
	uint64_t start_cycles, end_cycles;

	start_cycles = rte_rdtsc();
	while (1) {
		end_cycles = rte_rdtsc();
		/* duration uses up, stop producing */
		if (start_cycles + duration < end_cycles)
			break;
		rte_spinlock_lock(&scsp_spinlock);
		for (i = 0; i < MAX_KEEP; i ++) {
			if (NULL != scsp_obj_table[i]) {
				obj = scsp_obj_table[i];
				break;
			}
		}
		rte_spinlock_unlock(&scsp_spinlock);
		if (i >= MAX_KEEP) {
			continue;
		}
		if (rte_mempool_from_obj(obj) != mp_spsc) {
			printf("obj not owned by this mempool\n");
		}
		rte_mempool_put(mp_spsc, obj);
		rte_spinlock_lock(&scsp_spinlock);
		scsp_obj_table[i] = NULL;
		rte_spinlock_unlock(&scsp_spinlock);
	}

	return 0;
}

/*
 * single consumer function
 */
void* test_mempool_single_consumer(void* arg)
{
	int* ret = (int*)malloc(sizeof(int));
	*ret = 0;
	int cpu = *(int*)arg - 1;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        fprintf(stderr, "cpu %d is not runing\n", cpu);
        perror("");
		*ret = -1;
		pthread_exit((void*)ret);
        return NULL;
    }

	unsigned int i;
	void * obj;
	uint64_t start_cycles, end_cycles;

	start_cycles = rte_rdtsc();
	while (1) {
		end_cycles = rte_rdtsc();
		/* duration uses up, stop consuming */
		if (start_cycles + duration < end_cycles)
			break;
		rte_spinlock_lock(&scsp_spinlock);
		for (i = 0; i < MAX_KEEP; i ++) {
			if (NULL == scsp_obj_table[i])
				break;
		}
		rte_spinlock_unlock(&scsp_spinlock);
		if (i >= MAX_KEEP)
			continue;
		if (rte_mempool_get(mp_spsc, &obj) < 0)
			break;
		rte_spinlock_lock(&scsp_spinlock);
		scsp_obj_table[i] = obj;
		rte_spinlock_unlock(&scsp_spinlock);
	}

	pthread_exit((void*)ret);
	return 0;
}

void *
test_mempool_launch_single_consumer(void *arg)
{
	test_mempool_single_consumer(arg);
}

TEST(test_rte, test_mempool_sp_sc)
{
	int cpu = 0;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
	pthread_t tid;
	int ret;
    CPU_SET(cpu, &cpuset);

    ASSERT_EQ(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset), 0);

	mp_spsc = rte_mempool_create("test_mempool_sp_sc", MEMPOOL_SIZE,
			MEMPOOL_ELT_SIZE, 0, 0,
			my_mp_init, NULL,
			my_obj_init, NULL,
			SOCKET_ID_ANY,
			MEMPOOL_F_NO_CACHE_ALIGN | MEMPOOL_F_SP_PUT |
			MEMPOOL_F_SC_GET);
	ASSERT_NE((uintptr_t)mp_spsc, NULL);

	ASSERT_EQ((uintptr_t)rte_mempool_lookup("test_mempool_sp_sc"), (uintptr_t)mp_spsc);

	int i = 1;
	ret = pthread_create(&tid, NULL, test_mempool_launch_single_consumer, &i);
	// Failed to create testing thread
	ASSERT_EQ(ret, 0);

	ASSERT_EQ(test_mempool_single_producer(), 0);

	int* pret;
	pthread_join(tid, (void**)&pret);
	ASSERT_EQ(*pret, 0);
	free(pret);
	rte_mempool_free(mp_spsc);
}

TEST(test_rte, test_mempool_creation_with_exceeded_cache_size)
{
	struct rte_mempool *mp_cov;

	mp_cov = rte_mempool_create("test_mempool_cache_too_big",
		MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE,
		RTE_MEMPOOL_CACHE_MAX_SIZE + 32, 0,
		NULL, NULL,
		my_obj_init, NULL,
		SOCKET_ID_ANY, 0);

	ASSERT_EQ((uintptr_t)mp_cov, NULL);
}

TEST(test_rte, test_mempool_same_name_twice_creation)
{
	struct rte_mempool *mp_tc, *mp_tc2;

	mp_tc = rte_mempool_create("test_mempool_same_name", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE, 0, 0,
		NULL, NULL,
		NULL, NULL,
		SOCKET_ID_ANY, 0);

	ASSERT_NE((uintptr_t)mp_tc, NULL);

	mp_tc2 = rte_mempool_create("test_mempool_same_name", MEMPOOL_SIZE,
		MEMPOOL_ELT_SIZE, 0, 0,
		NULL, NULL,
		NULL, NULL,
		SOCKET_ID_ANY, 0);

	ASSERT_EQ((uintptr_t)mp_tc2, NULL);

	rte_mempool_free(mp_tc);
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
