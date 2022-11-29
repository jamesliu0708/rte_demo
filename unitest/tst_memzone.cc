#include "rte_eal.h"

#include "rte_memzone.h"
#include "rte_eal_memconfig.h"
#include "rte_malloc.h"
#include "malloc_elem.h"
#include "rte_random.h"

#include <gtest/gtest.h>

/*
 * Memzone
 * =======
 *
 * - Search for three reserved zones or reserve them if they do not exist:
 *
 *   - One is on any socket id.
 *   - The second is on socket 0.
 *   - The last one is on socket 1 (if socket 1 exists).
 *
 * - Check that the zones exist.
 *
 * - Check that the zones are cache-aligned.
 *
 * - Check that zones do not overlap.
 *
 * - Check that the zones are on the correct socket id.
 *
 * - Check that a lookup of the first zone returns the same pointer.
 *
 * - Check that it is not possible to create another zone with the
 *   same name as an existing zone.
 *
 * - Check flags for specific huge page size reservation
 */

#define TEST_MEMZONE_NAME(suffix) "MZ_TEST_" suffix

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

/* Test if memory overlaps: return 1 if true, or 0 if false. */
static int
is_memory_overlap(uintptr_t ptr1, size_t len1, uintptr_t ptr2, size_t len2)
{
	if (ptr2 >= ptr1 && (ptr2 - ptr1) < len1)
		return 1;
	else if (ptr2 < ptr1 && (ptr1 - ptr2) < len2)
		return 1;
	return 0;
}

TEST(test_memzone, test_memzone_basic)
{
    const struct rte_memzone *memzone1;
    const struct rte_memzone *memzone2;
	const struct rte_memzone *memzone3;
	const struct rte_memzone *memzone4;
	const struct rte_memzone *mz;
    int memzone_cnt_after, memzone_cnt_expected;

    int memzone_cnt_before =
			rte_eal_get_configuration()->mem_config->memzone_cnt;

	memzone1 = rte_memzone_reserve(TEST_MEMZONE_NAME("testzone1"), 100,
				SOCKET_ID_ANY, 0);

	memzone2 = rte_memzone_reserve(TEST_MEMZONE_NAME("testzone2"), 1000,
				0, 0);

	memzone3 = rte_memzone_reserve(TEST_MEMZONE_NAME("testzone3"), 1000,
				1, 0);

	memzone4 = rte_memzone_reserve(TEST_MEMZONE_NAME("testzone4"), 1024,
				SOCKET_ID_ANY, 0);

	/* memzone3 may be NULL if we don't have NUMA */
    ASSERT_NE((uintptr_t)memzone1, NULL);
    ASSERT_NE((uintptr_t)memzone2, NULL);
    ASSERT_NE((uintptr_t)memzone4, NULL);
	
    /* check how many memzones we are expecting */
    memzone_cnt_expected = memzone_cnt_before +
			(memzone1 != NULL) + (memzone2 != NULL) +
			(memzone3 != NULL) + (memzone4 != NULL);

    memzone_cnt_after =
			rte_eal_get_configuration()->mem_config->memzone_cnt;
    
    ASSERT_EQ(memzone_cnt_after, memzone_cnt_expected);

    rte_memzone_dump(stdout);

    /* check cache-line alignments */
	printf("check alignments and lengths\n");

    ASSERT_EQ((uintptr_t)memzone1->addr & RTE_CACHE_LINE_MASK, 0);
    ASSERT_EQ((uintptr_t)memzone2->addr & RTE_CACHE_LINE_MASK, 0);
    if (memzone3 != NULL) {
        ASSERT_EQ((uintptr_t)memzone3->addr & RTE_CACHE_LINE_MASK, 0);
    }
    ASSERT_NE(memzone1->len, 0);
    ASSERT_EQ(memzone1->len & RTE_CACHE_LINE_MASK, 0);
    ASSERT_NE(memzone2->len, 0);
    ASSERT_EQ(memzone2->len & RTE_CACHE_LINE_MASK, 0);
    if (memzone3 != NULL) {
        ASSERT_NE(memzone3->len, 0);
        ASSERT_EQ(memzone3->len & RTE_CACHE_LINE_MASK, 0);
    }
    ASSERT_EQ(memzone4->len, 1024);

    /* check that zones don't overlap */
	printf("check overlapping\n");

    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone1->addr, memzone1->len, (uintptr_t)memzone2->addr, memzone2->len), 0);
    if (memzone3 != NULL) {
        ASSERT_EQ(is_memory_overlap((uintptr_t)memzone3->addr, memzone3->len, (uintptr_t)memzone1->addr, memzone1->len), 0);
        ASSERT_EQ(is_memory_overlap((uintptr_t)memzone3->addr, memzone3->len, (uintptr_t)memzone2->addr, memzone2->len), 0);
    }

    printf("check socket ID\n");

    ASSERT_EQ(memzone2->socket_id, 0);
    if (memzone3 != NULL) {
        ASSERT_EQ(memzone3->socket_id, 1);
    }

    printf("test zone lookup\n");    printf("test zone lookup\n");


    mz = rte_memzone_lookup(TEST_MEMZONE_NAME("testzone1"));
    ASSERT_EQ(mz, memzone1);

    printf("test duplcate zone name\n");
	mz = rte_memzone_reserve(TEST_MEMZONE_NAME("testzone1"), 100,
			SOCKET_ID_ANY, 0);
    
    ASSERT_EQ((uintptr_t)mz, NULL);

    ASSERT_EQ(rte_memzone_free(memzone1), 0);
    ASSERT_EQ(rte_memzone_free(memzone2), 0);
    if (memzone3 != NULL) {
        ASSERT_EQ(rte_memzone_free(memzone3), 0);
    }
    ASSERT_EQ(rte_memzone_free(memzone4), 0);

    memzone_cnt_after =
			rte_eal_get_configuration()->mem_config->memzone_cnt;
    ASSERT_EQ(memzone_cnt_after, memzone_cnt_before);
}

TEST(test_memzone, test_memzone_free)
{
    const struct rte_memzone *mz[RTE_MAX_MEMZONE + 1];
	int i;
	char name[20];

	mz[0] = rte_memzone_reserve(TEST_MEMZONE_NAME("tempzone0"), 2000,
			SOCKET_ID_ANY, 0);
	mz[1] = rte_memzone_reserve(TEST_MEMZONE_NAME("tempzone1"), 4000,
			SOCKET_ID_ANY, 0);

    ASSERT_LE(mz[0], mz[1]);

    ASSERT_NE((uintptr_t)rte_memzone_lookup(TEST_MEMZONE_NAME("tempzone0")), 0);
    ASSERT_NE((uintptr_t)rte_memzone_lookup(TEST_MEMZONE_NAME("tempzone1")), 0);



    ASSERT_EQ(rte_memzone_free(mz[0]), 0);
    // Found previously free memzone - tempzone0
    ASSERT_EQ((uintptr_t)rte_memzone_lookup(TEST_MEMZONE_NAME("tempzone0")), 0);
    mz[2] = rte_memzone_reserve(TEST_MEMZONE_NAME("tempzone2"), 2000,
			SOCKET_ID_ANY, 0);
    // tempzone2 should have gotten the free entry from tempzone0
    ASSERT_LE((uintptr_t)mz[2], (uintptr_t)mz[1]);
    ASSERT_EQ(rte_memzone_free(mz[2]), 0);
    ASSERT_EQ((uintptr_t)rte_memzone_lookup(TEST_MEMZONE_NAME("tempzone2")), 0);

    ASSERT_EQ(rte_memzone_free(mz[1]), 0);
    //Found previously free memzone - tempzone1
    ASSERT_EQ((uintptr_t)rte_memzone_lookup(TEST_MEMZONE_NAME("tempzone1")), 0);

    i = 0;
	do {
		snprintf(name, sizeof(name), TEST_MEMZONE_NAME("tempzone%u"),
				i);
		mz[i] = rte_memzone_reserve(name, 1, SOCKET_ID_ANY, 0);
	} while (mz[i++] != NULL);

	ASSERT_EQ(rte_memzone_free(mz[0]), 0);

	mz[0] = rte_memzone_reserve(TEST_MEMZONE_NAME("tempzone0new"), 0,
			SOCKET_ID_ANY, 0);

	ASSERT_NE((uintptr_t)mz[0], 0);

	for (i = i - 2; i >= 0; i--) {
		ASSERT_EQ(rte_memzone_free(mz[i]), 0);
	}

}

TEST(test_memzone, test_memzone_reserving_zone_size_bigger_than_the_maximum)
{
    const struct rte_memzone * mz;

    ASSERT_EQ((uintptr_t)rte_memzone_lookup(TEST_MEMZONE_NAME("zone_size_bigger_than_the_maximum")), 0);
    ASSERT_EQ((uintptr_t)rte_memzone_reserve(TEST_MEMZONE_NAME("zone_size_bigger_than_the_maximum"),
			(size_t)-1, SOCKET_ID_ANY, 0), 0);
}

TEST(test_memzone, test_memzone_reserve_flags)
{
    const struct rte_memzone *mz;
	const struct rte_memseg *ms;
	int hugepage_2MB_avail = 0;
	int hugepage_1GB_avail = 0;
	int hugepage_16MB_avail = 0;
	int hugepage_16GB_avail = 0;
    const size_t size = 100;
	int i = 0;
	ms = rte_eal_get_physmem_layout();
	for (i = 0; i < RTE_MAX_MEMSEG; i++) {
		if (ms[i].hugepage_sz == RTE_PGSIZE_2M)
			hugepage_2MB_avail = 1;
		if (ms[i].hugepage_sz == RTE_PGSIZE_1G)
			hugepage_1GB_avail = 1;
		if (ms[i].hugepage_sz == RTE_PGSIZE_16M)
			hugepage_16MB_avail = 1;
		if (ms[i].hugepage_sz == RTE_PGSIZE_16G)
			hugepage_16GB_avail = 1;
	}
    /* Display the availability of 2MB ,1GB, 16MB, 16GB pages */
	if (hugepage_2MB_avail)
		printf("2MB Huge pages available\n");
	if (hugepage_1GB_avail)
		printf("1GB Huge pages available\n");
	if (hugepage_16MB_avail)
		printf("16MB Huge pages available\n");
	if (hugepage_16GB_avail)
		printf("16GB Huge pages available\n");
    /*
	 * If 2MB pages available, check that a small memzone is correctly
	 * reserved from 2MB huge pages when requested by the RTE_MEMZONE_2MB flag.
	 * Also check that RTE_MEMZONE_SIZE_HINT_ONLY flag only defaults to an
	 * available page size (i.e 1GB ) when 2MB pages are unavailable.
	 */
    if (hugepage_2MB_avail) {
        mz = rte_memzone_reserve(TEST_MEMZONE_NAME("flag_zone_2M"),
				size, SOCKET_ID_ANY, RTE_MEMZONE_2MB);
        ASSERT_NE((uintptr_t)mz, 0);
        ASSERT_EQ(mz->hugepage_sz, RTE_PGSIZE_2M);
        ASSERT_EQ(rte_memzone_free(mz), 0);
        mz = rte_memzone_reserve(TEST_MEMZONE_NAME("flag_zone_2M_HINT"),
				size, SOCKET_ID_ANY,
				RTE_MEMZONE_2MB|RTE_MEMZONE_SIZE_HINT_ONLY);
        ASSERT_NE((uintptr_t)mz, 0);
        ASSERT_EQ(mz->hugepage_sz, RTE_PGSIZE_2M);
        ASSERT_EQ(rte_memzone_free(mz), 0);

        /* Check if 1GB huge pages are unavailable, that function fails unless
	    * HINT flag is indicated
	    */
       if (hugepage_1GB_avail) {
            mz = rte_memzone_reserve(
					TEST_MEMZONE_NAME("flag_zone_1G_HINT"),
					size, SOCKET_ID_ANY,
					RTE_MEMZONE_1GB|RTE_MEMZONE_SIZE_HINT_ONLY);
            ASSERT_NE((uintptr_t)mz, 0);
            ASSERT_EQ(mz->hugepage_sz, RTE_PGSIZE_2M);
            ASSERT_EQ(rte_memzone_free(mz), 0);

            mz = rte_memzone_reserve(
					TEST_MEMZONE_NAME("flag_zone_1G"), size,
					SOCKET_ID_ANY, RTE_MEMZONE_1GB);
            ASSERT_EQ((uintptr_t)mz, 0);
       }
    }


}

TEST(test_memzone, test_memzone_aligned)
{
    const struct rte_memzone *memzone_aligned_32;
	const struct rte_memzone *memzone_aligned_128;
	const struct rte_memzone *memzone_aligned_256;
	const struct rte_memzone *memzone_aligned_512;
	const struct rte_memzone *memzone_aligned_1024;

    /* memzone that should automatically be adjusted to align on 64 bytes */
	memzone_aligned_32 = rte_memzone_reserve_aligned(
			TEST_MEMZONE_NAME("aligned_32"), 100, SOCKET_ID_ANY, 0,
			32);
    
    /* memzone that is supposed to be aligned on a 128 byte boundary */
	memzone_aligned_128 = rte_memzone_reserve_aligned(
			TEST_MEMZONE_NAME("aligned_128"), 100, SOCKET_ID_ANY, 0,
			128);
    
    /* memzone that is supposed to be aligned on a 256 byte boundary */
	memzone_aligned_256 = rte_memzone_reserve_aligned(
			TEST_MEMZONE_NAME("aligned_256"), 100, SOCKET_ID_ANY, 0,
			256);

	/* memzone that is supposed to be aligned on a 512 byte boundary */
	memzone_aligned_512 = rte_memzone_reserve_aligned(
			TEST_MEMZONE_NAME("aligned_512"), 100, SOCKET_ID_ANY, 0,
			512);

	/* memzone that is supposed to be aligned on a 1024 byte boundary */
	memzone_aligned_1024 = rte_memzone_reserve_aligned(
			TEST_MEMZONE_NAME("aligned_1024"), 100, SOCKET_ID_ANY,
			0, 1024);
    
    printf("check alignments and lengths\n");
    ASSERT_NE((uintptr_t)memzone_aligned_32, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_32->addr & RTE_CACHE_LINE_MASK, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_32->len & RTE_CACHE_LINE_MASK, 0);

    ASSERT_NE((uintptr_t)memzone_aligned_128, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_128->addr & 127, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_128->len & RTE_CACHE_LINE_MASK, 0);

    ASSERT_NE((uintptr_t)memzone_aligned_256, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_256->addr & 255, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_256->len & RTE_CACHE_LINE_MASK, 0);

    ASSERT_NE((uintptr_t)memzone_aligned_256, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_256->addr & 255, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_256->len & RTE_CACHE_LINE_MASK, 0);

    ASSERT_NE((uintptr_t)memzone_aligned_512, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_512->addr & 511, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_512->len & RTE_CACHE_LINE_MASK, 0);

    ASSERT_NE((uintptr_t)memzone_aligned_1024, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_1024->addr & 1023, 0);
    ASSERT_EQ((uintptr_t)memzone_aligned_1024->len & RTE_CACHE_LINE_MASK, 0);

    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_32->addr, memzone_aligned_32->len,
					(uintptr_t)memzone_aligned_128->addr, memzone_aligned_128->len), 0);
    
    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_32->addr, memzone_aligned_32->len,
					(uintptr_t)memzone_aligned_256->addr, memzone_aligned_256->len), 0);

    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_32->addr, memzone_aligned_32->len,
					(uintptr_t)memzone_aligned_512->addr, memzone_aligned_512->len), 0);
    
    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_32->addr, memzone_aligned_32->len,
					(uintptr_t)memzone_aligned_1024->addr, memzone_aligned_1024->len), 0);
    
    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_256->addr, memzone_aligned_256->len,
					(uintptr_t)memzone_aligned_128->addr, memzone_aligned_128->len), 0);
    
    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_256->addr, memzone_aligned_256->len,
					(uintptr_t)memzone_aligned_512->addr, memzone_aligned_512->len), 0);
    
    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_256->addr, memzone_aligned_256->len,
					(uintptr_t)memzone_aligned_1024->addr, memzone_aligned_1024->len), 0);
    
    ASSERT_EQ(is_memory_overlap((uintptr_t)memzone_aligned_512->addr, memzone_aligned_512->len,
					(uintptr_t)memzone_aligned_1024->addr, memzone_aligned_1024->len), 0);

    ASSERT_EQ(rte_memzone_free(memzone_aligned_32), 0);
    ASSERT_EQ(rte_memzone_free(memzone_aligned_128), 0);
    ASSERT_EQ(rte_memzone_free(memzone_aligned_256), 0);
    ASSERT_EQ(rte_memzone_free(memzone_aligned_512), 0);
    ASSERT_EQ(rte_memzone_free(memzone_aligned_1024), 0);
}

static int
check_memzone_bounded(const char *name, uint32_t len,  uint32_t align,
	uint32_t bound)
{
    const struct rte_memzone *mz;
    uint64_t bmask = ~((uint64_t)bound - 1);

    if ((mz = rte_memzone_reserve_bounded(name, len, SOCKET_ID_ANY, 0,
			align, bound)) == NULL) {
		printf("%s(%s): memzone creation failed\n",
			__func__, name);
		return -1;
	}

    if (((uintptr_t) mz->addr & ((uintptr_t)align - 1)) != 0) {
		printf("%s(%s): invalid virtual addr alignment\n",
			__func__, mz->name);
		return -1;
	}

    if ((mz->len & RTE_CACHE_LINE_MASK) != 0 || mz->len < len ||
			mz->len < RTE_CACHE_LINE_SIZE) {
		printf("%s(%s): invalid length\n",
			__func__, mz->name);
		return -1;
	}

    if (((uintptr_t)mz->addr & bmask) !=
			((uintptr_t)(mz->addr + mz->len - 1) & bmask)) {
		printf("%s(%s): invalid memzone boundary %u crossed\n",
			__func__, mz->name, bound);
		return -1;
	}

    rte_memzone_free(mz);
    return 0;
}

TEST(test_memzone, test_memzone_bounded)
{
    const struct rte_memzone *memzone_err;
	int rc;

    memzone_err = rte_memzone_reserve_bounded(
			TEST_MEMZONE_NAME("bounded_error_31"), 100,
			SOCKET_ID_ANY, 0, 32, UINT32_MAX);
    
    ASSERT_EQ((uintptr_t)memzone_err, 0);
    ASSERT_EQ(check_memzone_bounded(TEST_MEMZONE_NAME("bounded_128"), 100, 128,
			128), 0);
    ASSERT_EQ(check_memzone_bounded(TEST_MEMZONE_NAME("bounded_128"), 100, 256,
			128), 0);
    ASSERT_EQ(check_memzone_bounded(TEST_MEMZONE_NAME("bounded_128"), 100, 64,
			1024), 0);
    ASSERT_EQ(check_memzone_bounded(TEST_MEMZONE_NAME("bounded_128"), 0, 64,
			1024), 0);
}

TEST(test_combine, test_memzone_invalid_alignment)
{
    const struct rte_memzone * mz;

	mz = rte_memzone_lookup(TEST_MEMZONE_NAME("invalid_alignment"));
    ASSERT_EQ((uintptr_t)mz, 0);
    mz = rte_memzone_reserve_aligned(TEST_MEMZONE_NAME("invalid_alignment"),
					 100, SOCKET_ID_ANY, 0, 100);
    ASSERT_EQ((uintptr_t)mz, 0);
}

/* Find the heap with the greatest free block size */
static size_t
find_max_block_free_size(const unsigned _align)
{
	struct rte_malloc_socket_stats stats;
	unsigned i, align = _align;
	size_t len = 0;

	for (i = 0; i < RTE_MAX_NUMA_NODES; i++) {
		rte_malloc_get_socket_stats(i, &stats);
		if (stats.greatest_free_size > len)
			len = stats.greatest_free_size;
	}

	if (align < RTE_CACHE_LINE_SIZE)
		align = RTE_CACHE_LINE_ROUNDUP(align+1);

	if (len <= MALLOC_ELEM_OVERHEAD + align)
		return 0;

	return len - MALLOC_ELEM_OVERHEAD - align;
}

TEST(test_combine, test_memzone_reserve_max)
{
    const struct rte_memzone *mz;
	size_t maxlen;

	maxlen = find_max_block_free_size(0);

    if (maxlen == 0) {
		printf("There is no space left!\n");
	}
    mz = rte_memzone_reserve(TEST_MEMZONE_NAME("max_zone"), 0,
			SOCKET_ID_ANY, 0);
	ASSERT_NE((uintptr_t)mz, 0);

    ASSERT_EQ(mz->len, maxlen);

    rte_memzone_free(mz);
}

TEST(test_combine, test_memzone_reserve_max_aligned)
{
    const struct rte_memzone *mz;
	size_t maxlen = 0;

	/* random alignment */
	rte_srand((unsigned)rte_rdtsc());
	const unsigned align = 1 << ((rte_rand() % 8) + 5); /* from 128 up to 4k alignment */

	maxlen = find_max_block_free_size(align);

	if (maxlen == 0) {
		printf("There is no space left for biggest %u-aligned memzone!\n", align);
	}

    mz = rte_memzone_reserve_aligned(TEST_MEMZONE_NAME("max_zone_aligned"),
			0, SOCKET_ID_ANY, 0, align);
    ASSERT_NE((uintptr_t)mz, 0);

    ASSERT_EQ(mz->len, maxlen);

    rte_memzone_free(mz);
}

int main(int argc, char** argv)
{
// #ifdef MASTER
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "Failed to init rte\n");
        exit(-1);
    }
// #else
	// int ret = rte_eal_attach(argc, argv);
	// if (ret < 0) {
	// 	fprintf(stderr, "Failed to attach rte\n");
	// 	exit(-1);
	// }
// #endif // MASTER
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}