#include "rte_eal.h"
#include "rte_atomic.h"
#include "rte_malloc.h"
#include "rte_ring.h"
#include "rte_common.h"
#include "rte_random.h"
#include "rte_errno.h"

#include <gtest/gtest.h>

/*
 * Ring
 * ====
 *
 * #. Basic tests: done on one core:
 *
 *    - Using single producer/single consumer functions:
 *
 *      - Enqueue one object, two objects, MAX_BULK objects
 *      - Dequeue one object, two objects, MAX_BULK objects
 *      - Check that dequeued pointers are correct
 *
 *    - Using multi producers/multi consumers functions:
 *
 *      - Enqueue one object, two objects, MAX_BULK objects
 *      - Dequeue one object, two objects, MAX_BULK objects
 *      - Check that dequeued pointers are correct
 *
 * #. Performance tests.
 *
 * Tests done in test_ring_perf.c
 */

#define RING_SIZE 4096
#define MAX_BULK 32

static rte_atomic32_t synchro;

#define	TEST_RING_VERIFY(exp)						\
	if (!(exp)) {							\
		printf("error at %s:%d\tcondition " #exp " failed\n",	\
		    __func__, __LINE__);				\
		rte_ring_dump(stdout, r);				\
		return -1;				\
	}

#define	TEST_RING_FULL_EMTPY_ITER	8

/*
 * helper routine for test_ring_basic
 */
static int
test_ring_basic_full_empty(struct rte_ring *r, void * const src[], void *dst[])
{
	unsigned i, rand;
	const unsigned rsz = RING_SIZE - 1;

	printf("Basic full/empty test\n");

	for (i = 0; TEST_RING_FULL_EMTPY_ITER != i; i++) {

		/* random shift in the ring */
		rand = RTE_MAX(rte_rand() % RING_SIZE, 1UL);
		printf("%s: iteration %u, random shift: %u;\n",
		    __func__, i, rand);
		TEST_RING_VERIFY(rte_ring_enqueue_bulk(r, src, rand,
				NULL) != 0);
		TEST_RING_VERIFY(rte_ring_dequeue_bulk(r, dst, rand,
				NULL) == rand);

		/* fill the ring */
		TEST_RING_VERIFY(rte_ring_enqueue_bulk(r, src, rsz, NULL) != 0);
		TEST_RING_VERIFY(0 == rte_ring_free_count(r));
		TEST_RING_VERIFY(rsz == rte_ring_count(r));
		TEST_RING_VERIFY(rte_ring_full(r));
		TEST_RING_VERIFY(0 == rte_ring_empty(r));

		/* empty the ring */
		TEST_RING_VERIFY(rte_ring_dequeue_bulk(r, dst, rsz,
				NULL) == rsz);
		TEST_RING_VERIFY(rsz == rte_ring_free_count(r));
		TEST_RING_VERIFY(0 == rte_ring_count(r));
		TEST_RING_VERIFY(0 == rte_ring_full(r));
		TEST_RING_VERIFY(rte_ring_empty(r));

		/* check data */
		TEST_RING_VERIFY(0 == memcmp(src, dst, rsz));
		rte_ring_dump(stdout, r);
	}
	return 0;
}

TEST(test_rte, test_ring_basic_ex)
{
    int ret = -1;
	unsigned i;
	struct rte_ring *rp = NULL;
	void **obj = NULL;

    obj = (void**)rte_calloc("test_ring_basic_ex_malloc", RING_SIZE, sizeof(void *), 0);
    ASSERT_NE((uintptr_t)obj, 0);

    rp = rte_ring_create("test_ring_basic_ex", RING_SIZE, SOCKET_ID_ANY,
			RING_F_SP_ENQ | RING_F_SC_DEQ);
    ASSERT_NE((uintptr_t)rp, NULL);

    ASSERT_EQ((uintptr_t)rte_ring_lookup("test_ring_basic_ex"), (uintptr_t)rp);

    ASSERT_EQ(rte_ring_empty(rp), 1);

    printf("%u ring entries are now free\n", rte_ring_free_count(rp));

    for (i = 0; i < RING_SIZE; i ++) {
		rte_ring_enqueue(rp, obj[i]);
	}

    ASSERT_EQ(rte_ring_full(rp), 1);

    for (i = 0; i < RING_SIZE; i ++) {
		rte_ring_dequeue(rp, &obj[i]);
	}

    ASSERT_EQ(rte_ring_empty(rp), 1);

    /* Covering the ring burst operation */
	ret = rte_ring_enqueue_burst(rp, obj, 2, NULL);

    ASSERT_EQ(ret, 2);

    ret = rte_ring_dequeue_burst(rp, obj, 2, NULL);

    ASSERT_EQ(ret, 2);

    rte_ring_free(rp);

    rte_free(obj);
}


TEST(test_rte, test_ring_burst_basic)
{
    void **src = NULL, **cur_src = NULL, **dst = NULL, **cur_dst = NULL;
	int ret;
	unsigned i;
    auto r = rte_ring_create("test", RING_SIZE, SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)r, NULL);

    /* alloc dummy object pointers */
	src = (void**)malloc(RING_SIZE*2*sizeof(void *));
    ASSERT_NE((uintptr_t)src, NULL);

    for (i = 0; i < RING_SIZE*2 ; i++) {
		src[i] = (void *)(unsigned long)i;
	}
	cur_src = src;
    /* alloc some room for copied objects */
	dst = (void**)malloc(RING_SIZE*2*sizeof(void *));
    ASSERT_NE((uintptr_t)dst, NULL);

    memset(dst, 0, RING_SIZE*2*sizeof(void *));
	cur_dst = dst;

	printf("Test SP & SC basic functions \n");
	printf("enqueue 1 obj\n");
	ret = rte_ring_sp_enqueue_burst(r, cur_src, 1, NULL);
	cur_src += 1;

    ASSERT_EQ(ret, 1);

    printf("enqueue 2 objs\n");
	ret = rte_ring_sp_enqueue_burst(r, cur_src, 2, NULL);
	cur_src += 2;
    ASSERT_EQ(ret, 2);

    printf("enqueue MAX_BULK objs\n");
	ret = rte_ring_sp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
	cur_src += MAX_BULK;
    ASSERT_EQ(ret, MAX_BULK);

    printf("dequeue 1 obj\n");
	ret = rte_ring_sc_dequeue_burst(r, cur_dst, 1, NULL);
	cur_dst += 1;
    ASSERT_EQ(ret, 1);

    printf("dequeue 2 objs\n");
	ret = rte_ring_sc_dequeue_burst(r, cur_dst, 2, NULL);
	cur_dst += 2;
    ASSERT_EQ(ret, 2);

    printf("dequeue MAX_BULK objs\n");
	ret = rte_ring_sc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
	cur_dst += MAX_BULK;
    ASSERT_EQ(ret, MAX_BULK);

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("Test enqueue without enough memory space \n");
	for (i = 0; i< (RING_SIZE/MAX_BULK - 1); i++) {
		ret = rte_ring_sp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
		cur_src += MAX_BULK;
		ASSERT_EQ(ret, MAX_BULK);
	}

    printf("Enqueue 2 objects, free entries = MAX_BULK - 2  \n");
	ret = rte_ring_sp_enqueue_burst(r, cur_src, 2, NULL);
	cur_src += 2;
    ASSERT_EQ(ret, 2);

    printf("Enqueue the remaining entries = MAX_BULK - 2  \n");
	/* Always one free entry left */
	ret = rte_ring_sp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
	cur_src += MAX_BULK - 3;
    ASSERT_EQ(ret, MAX_BULK - 3);

    printf("Test if ring is full  \n");
	ASSERT_EQ(rte_ring_full(r), 1);

    printf("Test enqueue for a full entry  \n");
	ret = rte_ring_sp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
    ASSERT_EQ(ret, 0);

    printf("Test dequeue without enough objects \n");
	for (i = 0; i<RING_SIZE/MAX_BULK - 1; i++) {
		ret = rte_ring_sc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
		cur_dst += MAX_BULK;
		ASSERT_EQ(ret, MAX_BULK);
	}
    
    /* Available memory space for the exact MAX_BULK entries */
	ret = rte_ring_sc_dequeue_burst(r, cur_dst, 2, NULL);
	cur_dst += 2;
    ASSERT_EQ(ret, 2);

    ret = rte_ring_sc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
	cur_dst += MAX_BULK - 3;
    ASSERT_EQ(ret, MAX_BULK - 3);

    printf("Test if ring is empty \n");
	/* Check if ring is empty */
	ASSERT_EQ(rte_ring_empty(r), 1);

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("Test MP & MC basic functions \n");

	printf("enqueue 1 obj\n");
    ret = rte_ring_mp_enqueue_burst(r, cur_src, 1, NULL);
	cur_src += 1;
	ASSERT_EQ(ret, 1);

    printf("enqueue 2 objs\n");
	ret = rte_ring_mp_enqueue_burst(r, cur_src, 2, NULL);
	cur_src += 2;
    ASSERT_EQ(ret, 2);

    printf("enqueue MAX_BULK objs\n");
	ret = rte_ring_mp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
	cur_src += MAX_BULK;
	ASSERT_EQ(ret, MAX_BULK);

    printf("dequeue 1 obj\n");
	ret = rte_ring_mc_dequeue_burst(r, cur_dst, 1, NULL);
	cur_dst += 1;
	ASSERT_EQ(ret, 1);

    printf("dequeue 2 objs\n");
	ret = rte_ring_mc_dequeue_burst(r, cur_dst, 2, NULL);
	cur_dst += 2;
	ASSERT_EQ(ret, 2);

    printf("dequeue MAX_BULK objs\n");
	ret = rte_ring_mc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
	cur_dst += MAX_BULK;
    ASSERT_EQ(ret, MAX_BULK);

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("fill and empty the ring\n");
	for (i = 0; i<RING_SIZE/MAX_BULK; i++) {
		ret = rte_ring_mp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
		cur_src += MAX_BULK;
		ASSERT_EQ(ret, MAX_BULK);
		ret = rte_ring_mc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
		cur_dst += MAX_BULK;
		ASSERT_EQ(ret, MAX_BULK);
	}

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("Test enqueue without enough memory space \n");
	for (i = 0; i<RING_SIZE/MAX_BULK - 1; i++) {
		ret = rte_ring_mp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
		cur_src += MAX_BULK;
		ASSERT_EQ(ret, MAX_BULK);
	}

    /* Available memory space for the exact MAX_BULK objects */
	ret = rte_ring_mp_enqueue_burst(r, cur_src, 2, NULL);
	cur_src += 2;
	ASSERT_EQ(ret, 2);

    ret = rte_ring_mp_enqueue_burst(r, cur_src, MAX_BULK, NULL);
	cur_src += MAX_BULK - 3;
	ASSERT_EQ(ret, MAX_BULK - 3);

    printf("Test dequeue without enough objects \n");
	for (i = 0; i<RING_SIZE/MAX_BULK - 1; i++) {
		ret = rte_ring_mc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
		cur_dst += MAX_BULK;
		ASSERT_EQ(ret, MAX_BULK);
	}

	/* Available objects - the exact MAX_BULK */
	ret = rte_ring_mc_dequeue_burst(r, cur_dst, 2, NULL);
	cur_dst += 2;
	ASSERT_EQ(ret, 2);

	ret = rte_ring_mc_dequeue_burst(r, cur_dst, MAX_BULK, NULL);
	cur_dst += MAX_BULK - 3;
	ASSERT_EQ(ret, MAX_BULK - 3);

	/* check data */
	ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

    printf("Covering rte_ring_enqueue_burst functions \n");

	ret = rte_ring_enqueue_burst(r, cur_src, 2, NULL);
	cur_src += 2;
	ASSERT_EQ(ret, 2);

    ret = rte_ring_dequeue_burst(r, cur_dst, 2, NULL);
	cur_dst += 2;
	ASSERT_EQ(ret, 2);

    rte_ring_free(r);

    free(src);
}

TEST(test_rte, test_ring_basic)
{
    auto r = rte_ring_create("test", RING_SIZE, SOCKET_ID_ANY, 0);
    ASSERT_NE((uintptr_t)r, NULL);

    void **src = NULL, **cur_src = NULL, **dst = NULL, **cur_dst = NULL;
	int ret;
	unsigned i, num_elems;

	/* alloc dummy object pointers */
	src = (void**)malloc(RING_SIZE*2*sizeof(void *));
    ASSERT_NE((uintptr_t)src, NULL);

	for (i = 0; i < RING_SIZE*2 ; i++) {
		src[i] = (void *)(unsigned long)i;
	}
	cur_src = src;

	/* alloc some room for copied objects */
	dst = (void**)malloc(RING_SIZE*2*sizeof(void *));
	ASSERT_NE((uintptr_t)dst, NULL);

	memset(dst, 0, RING_SIZE*2*sizeof(void *));
	cur_dst = dst;

    memset(dst, 0, RING_SIZE*2*sizeof(void *));
	cur_dst = dst;

	printf("enqueue 1 obj\n");
	ret = rte_ring_sp_enqueue_bulk(r, cur_src, 1, NULL);
	cur_src += 1;
	ASSERT_NE(ret, 0);

    printf("enqueue 2 objs\n");
	ret = rte_ring_sp_enqueue_bulk(r, cur_src, 2, NULL);
	cur_src += 2;
	ASSERT_NE(ret, 0);

    printf("enqueue MAX_BULK objs\n");
	ret = rte_ring_sp_enqueue_bulk(r, cur_src, MAX_BULK, NULL);
	cur_src += MAX_BULK;
	ASSERT_NE(ret, 0);

    printf("dequeue 1 obj\n");
	ret = rte_ring_sc_dequeue_bulk(r, cur_dst, 1, NULL);
	cur_dst += 1;
	ASSERT_NE(ret, 0);

    printf("dequeue 2 objs\n");
	ret = rte_ring_sc_dequeue_bulk(r, cur_dst, 2, NULL);
	cur_dst += 2;
	ASSERT_NE(ret, 0);

    printf("dequeue MAX_BULK objs\n");
	ret = rte_ring_sc_dequeue_bulk(r, cur_dst, MAX_BULK, NULL);
	cur_dst += MAX_BULK;
	ASSERT_NE(ret, 0);

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("enqueue 1 obj\n");
	ret = rte_ring_mp_enqueue_bulk(r, cur_src, 1, NULL);
	cur_src += 1;
	ASSERT_NE(ret, 0);

    printf("enqueue 2 objs\n");
	ret = rte_ring_mp_enqueue_bulk(r, cur_src, 2, NULL);
	cur_src += 2;
	ASSERT_NE(ret, 0);

	printf("enqueue MAX_BULK objs\n");
	ret = rte_ring_mp_enqueue_bulk(r, cur_src, MAX_BULK, NULL);
	cur_src += MAX_BULK;
	ASSERT_NE(ret, 0);

	printf("dequeue 1 obj\n");
	ret = rte_ring_mc_dequeue_bulk(r, cur_dst, 1, NULL);
	cur_dst += 1;
	ASSERT_NE(ret, 0);

	printf("dequeue 2 objs\n");
	ret = rte_ring_mc_dequeue_bulk(r, cur_dst, 2, NULL);
	cur_dst += 2;
	ASSERT_NE(ret, 0);

	printf("dequeue MAX_BULK objs\n");
	ret = rte_ring_mc_dequeue_bulk(r, cur_dst, MAX_BULK, NULL);
	cur_dst += MAX_BULK;
	ASSERT_NE(ret, 0);

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("fill and empty the ring\n");
	for (i = 0; i<RING_SIZE/MAX_BULK; i++) {
		ret = rte_ring_mp_enqueue_bulk(r, cur_src, MAX_BULK, NULL);
		cur_src += MAX_BULK;
		ASSERT_NE(ret, 0);
		ret = rte_ring_mc_dequeue_bulk(r, cur_dst, MAX_BULK, NULL);
		cur_dst += MAX_BULK;
		ASSERT_NE(ret, 0);
	}

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    ASSERT_EQ(test_ring_basic_full_empty(r, src, dst), 0);

    cur_src = src;
	cur_dst = dst;

	printf("test default bulk enqueue / dequeue\n");
	num_elems = 16;

    cur_src = src;
	cur_dst = dst;

	ret = rte_ring_enqueue_bulk(r, cur_src, num_elems, NULL);
	cur_src += num_elems;
	ASSERT_NE(ret, 0);

    ret = rte_ring_enqueue_bulk(r, cur_src, num_elems, NULL);
	cur_src += num_elems;
    ASSERT_NE(ret, 0);

    ret = rte_ring_dequeue_bulk(r, cur_dst, num_elems, NULL);
	cur_dst += num_elems;
    ASSERT_NE(ret, 0);

    ret = rte_ring_dequeue_bulk(r, cur_dst, num_elems, NULL);
	cur_dst += num_elems;
    ASSERT_NE(ret, 0);

    ASSERT_EQ(memcmp(src, dst, cur_dst - dst), 0);

    cur_src = src;
	cur_dst = dst;

	ret = rte_ring_mp_enqueue(r, cur_src);
	ASSERT_EQ(ret, 0);

	ret = rte_ring_mc_dequeue(r, cur_dst);
    ASSERT_EQ(ret, 0);
    free(src);
	free(dst);
    rte_ring_free(r);
}

/*
 * Test to if a non-power of 2 count causes the create
 * function to fail correctly
 */
TEST(test_rte, test_create_count_odd)
{
	struct rte_ring *r = rte_ring_create("test_ring_count",
			4097, SOCKET_ID_ANY, 0 );
	ASSERT_EQ((uintptr_t)r, NULL);
}

TEST(test_rte, test_lookup_null)
{
    struct rte_ring *rlp = rte_ring_lookup("ring_not_found");
    if (rlp ==NULL) {
        ASSERT_EQ(rte_errno, ENOENT);
    }   
}

TEST(test_rte, test_ring_creation_with_wrong_size)
{
    struct rte_ring * rp = NULL;

	/* Test if ring size is not power of 2 */
	rp = rte_ring_create("test_bad_ring_size", RING_SIZE + 1, SOCKET_ID_ANY, 0);
	ASSERT_EQ((uintptr_t)rp, NULL);

    /* Test if ring size is exceeding the limit */
	rp = rte_ring_create("test_bad_ring_size", (RTE_RING_SZ_MASK + 1), SOCKET_ID_ANY, 0);
	ASSERT_EQ((uintptr_t)rp, NULL);
}

TEST(test_rte, test_ring_creation_with_an_used_name)
{
    struct rte_ring * rp1, *rp2;

	rp1 = rte_ring_create("test", RING_SIZE, SOCKET_ID_ANY, 0);

    ASSERT_NE((uintptr_t)rp1, NULL);

    rp2 = rte_ring_create("test", RING_SIZE, SOCKET_ID_ANY, 0);
	ASSERT_EQ((uintptr_t)rp2, NULL);

    rte_ring_free(rp1);
}

TEST(test_rte, test_ring_with_exact_size)
{
    struct rte_ring *std_ring = NULL, *exact_sz_ring = NULL;
	void *ptr_array[16];
	static const unsigned int ring_sz = RTE_DIM(ptr_array);
	unsigned int i;
	int ret = -1;

    std_ring = rte_ring_create("std", ring_sz, rte_socket_id(),
			RING_F_SP_ENQ | RING_F_SC_DEQ);
    ASSERT_NE((uintptr_t)std_ring, NULL);

    exact_sz_ring = rte_ring_create("exact sz", ring_sz, rte_socket_id(),
			RING_F_SP_ENQ | RING_F_SC_DEQ | RING_F_EXACT_SZ);
    ASSERT_NE((uintptr_t)exact_sz_ring, NULL);

    ASSERT_LT(rte_ring_get_size(std_ring), rte_ring_get_size(exact_sz_ring));

    /*
	 * check that the exact_sz_ring can hold one more element than the
	 * standard ring. (16 vs 15 elements)
	 */
	for (i = 0; i < ring_sz - 1; i++) {
        rte_ring_enqueue(std_ring, NULL);
		rte_ring_enqueue(exact_sz_ring, NULL);
    }

    ASSERT_EQ(rte_ring_enqueue(std_ring, NULL), -ENOBUFS);
    ASSERT_NE(rte_ring_enqueue(exact_sz_ring, NULL), -ENOBUFS);

    ASSERT_EQ(rte_ring_dequeue_burst(exact_sz_ring, ptr_array,
			RTE_DIM(ptr_array), NULL), ring_sz);
    ASSERT_EQ(rte_ring_get_capacity(exact_sz_ring), ring_sz);

    rte_ring_free(std_ring);
	rte_ring_free(exact_sz_ring);
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