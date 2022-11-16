#include "rte_malloc.h"
#include "rte_eal.h"
#include "rte_common.h"
#include "rte_cycles.h"
#include "rte_random.h"

#include <gtest/gtest.h>

#define N 10000

TEST(test_malloc, test_str_to_size)
{
    struct {
		const char *str;
		uint64_t value;
	} test_values[] =
	{{ "5G", (uint64_t)5 * 1024 * 1024 *1024 },
			{"0x20g", (uint64_t)0x20 * 1024 * 1024 *1024},
			{"10M", 10 * 1024 * 1024},
			{"050m", 050 * 1024 * 1024},
			{"8K", 8 * 1024},
			{"15k", 15 * 1024},
			{"0200", 0200},
			{"0x103", 0x103},
			{"432", 432},
			{"-1", 0}, /* negative values return 0 */
			{"  -2", 0},
			{"  -3MB", 0},
			{"18446744073709551616", 0} /* ULLONG_MAX + 1 == out of range*/
	};
	unsigned i;
	for (i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++)
		ASSERT_EQ (rte_str_to_size(test_values[i].str), test_values[i].value);
}

TEST(test_malloc, test_zero_aligned_alloc)
{
    char *p1 = (char*)rte_malloc(NULL,1024, 0);
	ASSERT_NE(p1, NULL);
	ASSERT_EQ(rte_is_aligned(p1, RTE_CACHE_LINE_SIZE), 0);
	rte_free(p1);
}

TEST(test_malloc, test_malloc_bad_params)
{
    const char *type = NULL;
	size_t size = 0;
	unsigned align = RTE_CACHE_LINE_SIZE;

	/* rte_malloc expected to return null with inappropriate size */
	char *bad_ptr = (char*)rte_malloc(type, size, align);
	ASSERT_EQ(bad_ptr, NULL);

	/* rte_malloc expected to return null with inappropriate alignment */
	align = 17;
	size = 1024;

	bad_ptr = (char*)rte_malloc(type, size, align);
	ASSERT_EQ(bad_ptr, NULL);
}

TEST(test_malloc, test_realloc)
{
	const char hello_str[] = "Hello, world!";
	const unsigned size1 = 1024;
	const unsigned size2 = size1 + 1024;
	const unsigned size3 = size2;
	const unsigned size4 = size3 + 1024;

	/* test data is the same even if element is moved*/
	char *ptr1 = (char*)rte_zmalloc(NULL, size1, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr1, NULL);

	snprintf(ptr1, size1, "%s" ,hello_str);
	char *ptr2 = (char*)rte_realloc(ptr1, size2, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr2, NULL);

	if (ptr1 == ptr2){
		printf("unexpected - ptr1 == ptr2\n");
	}

	ASSERT_EQ(strcmp(ptr2, hello_str), 0);
	unsigned i;
	for (i = strnlen(hello_str, sizeof(hello_str)); i < size1; i++)
		ASSERT_EQ(ptr2[i], 0);

	/* now allocate third element, free the second
	 * and resize third. It should not move. (ptr1 is now invalid)
	 */
	char *ptr3 = (char*)rte_zmalloc(NULL, size3, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr3, 0);

	for (i = 0; i < size3; i++)
		ASSERT_EQ(ptr3[i], 0);

	rte_free(ptr2);
	/* first resize to half the size of the freed block */
	char *ptr4 = (char*)rte_realloc(ptr3, size4, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr4, 0);

	ASSERT_EQ(ptr3, ptr4);

	/* now resize again to the full size of the freed block */
	ptr4 = (char*)rte_realloc(ptr3, size3 + size2 + size1, RTE_CACHE_LINE_SIZE);
	ASSERT_EQ(ptr3, ptr4);
	rte_free(ptr4);

	/* now try a resize to a smaller size, see if it works */
	const unsigned size5 = 1024;
	const unsigned size6 = size5 / 2;
	char *ptr5 = (char*)rte_malloc(NULL, size5, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr5, NULL);
	char *ptr6 = (char*)rte_realloc(ptr5, size6, RTE_CACHE_LINE_SIZE);
	// NULL pointer returned from rte_realloc
    ASSERT_NE(ptr6, NULL);
    // resizing to a smaller size moved data
	ASSERT_EQ(ptr5, ptr6);
	rte_free(ptr6);

	/* check for behaviour changing alignment */
	const unsigned size7 = 1024;
	const unsigned orig_align = RTE_CACHE_LINE_SIZE;
	unsigned new_align = RTE_CACHE_LINE_SIZE * 2;
	char *ptr7 = (char*)rte_malloc(NULL, size7, orig_align);
	// NULL pointer returned from rte_realloc
    ASSERT_NE(ptr7, NULL);
	/* calc an alignment we don't already have */
	while(RTE_PTR_ALIGN(ptr7, new_align) == ptr7)
		new_align *= 2;
	char *ptr8 = (char*)rte_realloc(ptr7, size7, new_align);
	ASSERT_NE(ptr8, NULL);
    // Failure to re-align data
	ASSERT_EQ(RTE_PTR_ALIGN(ptr8, new_align), ptr8);
	rte_free(ptr8);

	/* test behaviour when there is a free block after current one,
	 * but its not big enough
	 */
	unsigned size9 = 1024, size10 = 1024;
	unsigned size11 = size9 + size10 + 256;
	char *ptr9 = (char*)rte_malloc(NULL, size9, RTE_CACHE_LINE_SIZE);
	// NULL pointer returned from rte_realloc
    ASSERT_NE(ptr9, NULL);
	char *ptr10 = (char*)rte_malloc(NULL, size10, RTE_CACHE_LINE_SIZE);
	// NULL pointer returned from rte_realloc
    ASSERT_NE(ptr10, NULL);
	rte_free(ptr9);
	char *ptr11 = (char*)rte_realloc(ptr10, size11, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr11, NULL);
    // unexpected that realloc has not created new buffer
	ASSERT_NE(ptr11, ptr10);
	rte_free(ptr11);

	/* check we don't crash if we pass null to realloc
	 * We should get a malloc of the size requested*/
	const size_t size12 = 1024;
	size_t size12_check;
	char *ptr12 = (char*)rte_realloc(NULL, size12, RTE_CACHE_LINE_SIZE);
	ASSERT_NE(ptr11, ptr10);
	// ASSERT_EQ(rte_malloc_validate(ptr12, &size12_check) < 0 ||
	// 		size12_check != size12, 0);
	rte_free(ptr12);
}

/*
 * Malloc
 * ======
 *
 * Allocate some dynamic memory from heap (3 areas). Check that areas
 * don't overlap and that alignment constraints match. This test is
 * done many times on different lcores simultaneously.
 */

/* Test if memory overlaps: return 1 if true, or 0 if false. */
static int
is_memory_overlap(void *p1, size_t len1, void *p2, size_t len2)
{
	unsigned long ptr1 = (unsigned long)p1;
	unsigned long ptr2 = (unsigned long)p2;

	if (ptr2 >= ptr1 && (ptr2 - ptr1) < len1)
		return 1;
	else if (ptr2 < ptr1 && (ptr1 - ptr2) < len2)
		return 1;
	return 0;
}

static int
is_aligned(void *p, int align)
{
	unsigned long addr = (unsigned long)p;
	unsigned mask = align - 1;

	if (addr & mask)
		return 0;
	return 1;
}

void *
tst_align_overlap_per_lcore(void *arg)
{
	int cpu = *(int*)arg - 1;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    int i;

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        fprintf(stderr, "cpu %d is not runing\n", cpu);
        perror("");
        return NULL;
    }

	const unsigned align1 = 8,
			align2 = 64,
			align3 = 2048;
	unsigned i,j;
	void *p1 = NULL, *p2 = NULL, *p3 = NULL;
	int ret = 0;

	for (i = 0; i < N; i++) {
		p1 = rte_zmalloc("dummy", 1000, align1);
		if (!p1){
			printf("rte_zmalloc returned NULL (i=%u)\n", i);
			ret = -1;
			break;
		}
		for(j = 0; j < 1000 ; j++) {
			if( *(char *)p1 != 0) {
				printf("rte_zmalloc didn't zero the allocated memory\n");
				ret = -1;
			}
		}
		p2 = rte_malloc("dummy", 1000, align2);
		if (!p2){
			printf("rte_malloc returned NULL (i=%u)\n", i);
			ret = -1;
			rte_free(p1);
			break;
		}
		p3 = rte_malloc("dummy", 1000, align3);
		if (!p3){
			printf("rte_malloc returned NULL (i=%u)\n", i);
			ret = -1;
			rte_free(p1);
			rte_free(p2);
			break;
		}
		if (is_memory_overlap(p1, 1000, p2, 1000)) {
			printf("p1 and p2 overlaps\n");
			ret = -1;
		}
		if (is_memory_overlap(p2, 1000, p3, 1000)) {
			printf("p2 and p3 overlaps\n");
			ret = -1;
		}
		if (is_memory_overlap(p1, 1000, p3, 1000)) {
			printf("p1 and p3 overlaps\n");
			ret = -1;
		}
		if (!is_aligned(p1, align1)) {
			printf("p1 is not aligned\n");
			ret = -1;
		}
		if (!is_aligned(p2, align2)) {
			printf("p2 is not aligned\n");
			ret = -1;
		}
		if (!is_aligned(p3, align3)) {
			printf("p3 is not aligned\n");
			ret = -1;
		}
		rte_free(p1);
		rte_free(p2);
		rte_free(p3);
	}
	rte_malloc_dump_stats(stdout, "dummy");
	pthread_exit((void*)&ret);
	return NULL;
}

TEST(test_malloc, test_align_overlap_per_lcore)
{
    int ncpus = sysconf( _SC_NPROCESSORS_CONF);
    pthread_t* tid_list = (pthread_t*)malloc(ncpus * sizeof(pthread_t));
    int ret;
    if (tid_list == NULL) {
        fprintf(stderr, "Failed to alloc tid vec\n");
        exit(-1);
    }

    for(int i = 0; i < ncpus; ++i)
    {
        ret = pthread_create(&tid_list[i], NULL, tst_align_overlap_per_lcore, &i);
		// Failed to create testing thread
        ASSERT_EQ(ret, 0);
    }

	for(int i = 0; i < ncpus; ++i)
	{
		pthread_join(tid_list[i], NULL);
	}
}

void*
tst_reordered_free_per_lcore(void *arg)
{
	int cpu = *(int*)arg - 1;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    int i;

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        fprintf(stderr, "cpu %d is not runing\n", cpu);
        perror("");
        return NULL;
    }

	const unsigned align1 = 8,
			align2 = 64,
			align3 = 2048;
	unsigned i,j;
	void *p1, *p2, *p3;
	int ret = 0;

	for (i = 0; i < 30; i++) {
		p1 = rte_zmalloc("dummy", 1000, align1);
		if (!p1){
			printf("rte_zmalloc returned NULL (i=%u)\n", i);
			ret = -1;
			break;
		}
		for(j = 0; j < 1000 ; j++) {
			if( *(char *)p1 != 0) {
				printf("rte_zmalloc didn't zero the allocated memory\n");
				ret = -1;
			}
		}
		/* use calloc to allocate 1000 16-byte items this time */
		p2 = rte_calloc("dummy", 1000, 16, align2);
		/* for third request use regular malloc again */
		p3 = rte_malloc("dummy", 1000, align3);
		if (!p2 || !p3){
			printf("rte_malloc returned NULL (i=%u)\n", i);
			ret = -1;
			break;
		}
		if (is_memory_overlap(p1, 1000, p2, 1000)) {
			printf("p1 and p2 overlaps\n");
			ret = -1;
		}
		if (is_memory_overlap(p2, 1000, p3, 1000)) {
			printf("p2 and p3 overlaps\n");
			ret = -1;
		}
		if (is_memory_overlap(p1, 1000, p3, 1000)) {
			printf("p1 and p3 overlaps\n");
			ret = -1;
		}
		if (!is_aligned(p1, align1)) {
			printf("p1 is not aligned\n");
			ret = -1;
		}
		if (!is_aligned(p2, align2)) {
			printf("p2 is not aligned\n");
			ret = -1;
		}
		if (!is_aligned(p3, align3)) {
			printf("p3 is not aligned\n");
			ret = -1;
		}
		/* try freeing in every possible order */
		switch (i%6){
		case 0:
			rte_free(p1);
			rte_free(p2);
			rte_free(p3);
			break;
		case 1:
			rte_free(p1);
			rte_free(p3);
			rte_free(p2);
			break;
		case 2:
			rte_free(p2);
			rte_free(p1);
			rte_free(p3);
			break;
		case 3:
			rte_free(p2);
			rte_free(p3);
			rte_free(p1);
			break;
		case 4:
			rte_free(p3);
			rte_free(p1);
			rte_free(p2);
			break;
		case 5:
			rte_free(p3);
			rte_free(p2);
			rte_free(p1);
			break;
		}
	}
	rte_malloc_dump_stats(stdout, "dummy");

	pthread_exit((void*)&ret);
	return NULL;
}

TEST(test_malloc, test_reordered_free_per_lcore)
{
	int ncpus = sysconf( _SC_NPROCESSORS_CONF);
    pthread_t* tid_list = (pthread_t*)malloc(ncpus * sizeof(pthread_t));
    int ret;
    if (tid_list == NULL) {
        fprintf(stderr, "Failed to alloc tid vec\n");
        exit(-1);
    }

    for(int i = 0; i < ncpus; ++i)
    {
        ret = pthread_create(&tid_list[i], NULL, tst_reordered_free_per_lcore, &i);
		// Failed to create testing thread
        ASSERT_EQ(ret, 0);
    }

	for(int i = 0; i < ncpus; ++i)
	{
		pthread_join(tid_list[i], NULL);
	}
}

void*
tst_random_alloc_free(void *arg)
{
	int cpu = *(int*)arg - 1;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    int i;

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        fprintf(stderr, "cpu %d is not runing\n", cpu);
        perror("");
        return NULL;
    }

	struct mem_list {
		struct mem_list *next;
		char data[0];
	} *list_head = NULL;
	unsigned i;
	unsigned count = 0;

	rte_srand((unsigned)rte_rdtsc());

	for (i = 0; i < N; i++){
		unsigned free_mem = 0;
		size_t allocated_size;
		while (!free_mem){
			const unsigned mem_size = sizeof(struct mem_list) + \
					rte_rand() % (64 * 1024);
			const unsigned align = 1 << (rte_rand() % 12); /* up to 4k alignment */
			struct mem_list *entry = (struct mem_list*)rte_malloc(NULL,
					mem_size, align);
			if (entry == NULL)
				return NULL;
			if (RTE_PTR_ALIGN(entry, align)!= entry)
				return NULL;
			if (rte_malloc_validate(entry, &allocated_size) == -1
					|| allocated_size < mem_size)
				return NULL;
			memset(entry->data, cpu,
					mem_size - sizeof(*entry));
			entry->next = list_head;
			if (rte_malloc_validate(entry, NULL) == -1)
				return NULL;
			list_head = entry;

			count++;
			/* switch to freeing the memory with a 20% probability */
			free_mem = ((rte_rand() % 10) >= 8);
		}
		while (list_head){
			struct mem_list *entry = list_head;
			list_head = list_head->next;
			rte_free(entry);
		}
	}
	printf("Lcore %u allocated/freed %u blocks\n", cpu, count);
	return 0;
}

TEST(test_malloc, test_random_alloc_free)
{
	int ncpus = sysconf( _SC_NPROCESSORS_CONF);
    pthread_t* tid_list = (pthread_t*)malloc(ncpus * sizeof(pthread_t));
    int ret;
    if (tid_list == NULL) {
        fprintf(stderr, "Failed to alloc tid vec\n");
        exit(-1);
    }

    for(int i = 0; i < ncpus; ++i)
    {
        ret = pthread_create(&tid_list[i], NULL, tst_random_alloc_free, &i);
		// Failed to create testing thread
        ASSERT_EQ(ret, 0);
    }

	for(int i = 0; i < ncpus; ++i)
	{
		pthread_join(tid_list[i], NULL);
	}
}

TEST(test_malloc, test_rte_malloc_type_limits)
{
	/* The type-limits functionality is not yet implemented,
	 * so always return 0 no matter what the retval.
	 */
	const char *typename_test = "limit_test";
	rte_malloc_dump_stats(stdout, typename_test);
}

/* Check if memory is available on a specific socket */
static int
is_mem_on_socket(int32_t socket)
{
	const struct rte_memseg *ms = rte_eal_get_physmem_layout();
	unsigned i;

	for (i = 0; i < RTE_MAX_MEMSEG; i++) {
		if (socket == ms[i].socket_id)
			return 1;
	}
	return 0;
}

/*
 * Find what socket a memory address is on. Only works for addresses within
 * memsegs, not heap or stack...
 */
static int32_t
addr_to_socket(void * addr)
{
	const struct rte_memseg *ms = rte_eal_get_physmem_layout();
	unsigned i;

	for (i = 0; i < RTE_MAX_MEMSEG; i++) {
		if ((ms[i].addr <= addr) &&
				((uintptr_t)addr <
				((uintptr_t)ms[i].addr + (uintptr_t)ms[i].len)))
			return ms[i].socket_id;
	}
	return -1;
}

/* Test using rte_[c|m|zm]alloc_socket() on a specific socket */
static int
test_alloc_single_socket(int32_t socket)
{
	const char *type = NULL;
	const size_t size = 10;
	const unsigned align = 0;
	char *mem = NULL;
	int32_t desired_socket = (socket == SOCKET_ID_ANY) ?
			(int32_t)rte_socket_id() : socket;

	/* Test rte_calloc_socket() */
	mem = rte_calloc_socket(type, size, sizeof(char), align, socket);
	if (mem == NULL)
		return -1;
	if (addr_to_socket(mem) != desired_socket) {
		rte_free(mem);
		return -1;
	}
	rte_free(mem);

	/* Test rte_malloc_socket() */
	mem = rte_malloc_socket(type, size, align, socket);
	if (mem == NULL)
		return -1;
	if (addr_to_socket(mem) != desired_socket) {
		return -1;
	}
	rte_free(mem);

	/* Test rte_zmalloc_socket() */
	mem = rte_zmalloc_socket(type, size, align, socket);
	if (mem == NULL)
		return -1;
	if (addr_to_socket(mem) != desired_socket) {
		rte_free(mem);
		return -1;
	}
	rte_free(mem);

	return 0;
}

// TEST(test_malloc, test_alloc_socket)
// {
// 	unsigned socket_count = 0;
// 	unsigned i;

// 	if (test_alloc_single_socket(SOCKET_ID_ANY) < 0)
// 		return -1;

// 	for (i = 0; i < RTE_MAX_NUMA_NODES; i++) {
// 		if (is_mem_on_socket(i)) {
// 			socket_count++;
// 			if (test_alloc_single_socket(i) < 0) {
// 				printf("Fail: rte_malloc_socket(..., %u) did not succeed\n",
// 						i);
// 				return -1;
// 			}
// 		}
// 		else {
// 			if (test_alloc_single_socket(i) == 0) {
// 				printf("Fail: rte_malloc_socket(..., %u) succeeded\n",
// 						i);
// 				return -1;
// 			}
// 		}
// 	}

// 	/* Print warnign if only a single socket, but don't fail the test */
// 	if (socket_count < 2) {
// 		printf("WARNING: alloc_socket test needs memory on multiple sockets!\n");
// 	}

// 	return 0;
// }

int main(int argc, char** argv)
{
    rte_eal_init(0, NULL);

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}