#include <stdint.h>
#include <stdio.h>

#include <gtest/gtest.h>

#include "rte_memcpy.h"
#include "rte_random.h"

/*
 * Set this to the maximum buffer size you want to test. If it is 0, then the
 * values in the buf_sizes[] array below will be used.
 */
#define TEST_VALUE_RANGE        0

/* List of buffer sizes to test */
#if TEST_VALUE_RANGE == 0
static size_t buf_sizes[] = {
	0, 1, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255,
	256, 257, 320, 384, 511, 512, 513, 1023, 1024, 1025, 1518, 1522, 1600,
	2048, 3072, 4096, 5120, 6144, 7168, 8192
};
/* MUST be as large as largest packet size above */
#define SMALL_BUFFER_SIZE       8192
#else /* TEST_VALUE_RANGE != 0 */
static size_t buf_sizes[TEST_VALUE_RANGE];
#define SMALL_BUFFER_SIZE       TEST_VALUE_RANGE
#endif /* TEST_VALUE_RANGE == 0 */

/*
 * Arrays of this size are used for measuring uncached memory accesses by
 * picking a random location within the buffer. Make this smaller if there are
 * memory allocation errors.
 */
#define LARGE_BUFFER_SIZE       (100 * 1024 * 1024)

/* How many times to run timing loop for performance tests */
#define TEST_ITERATIONS         1000000
#define TEST_BATCH_SIZE         100

/* Data is aligned on this many bytes (power of 2) */
#define ALIGNMENT_UNIT          16

/* Structure with base memcpy func pointer, and number of bytes it copies */
struct base_memcpy_func {
	void (*func)(uint8_t *dst, const uint8_t *src);
	unsigned size;
};

/*
 * Create two buffers, and initialise one with random values. These are copied
 * to the second buffer and then compared to see if the copy was successful.
 * The bytes outside the copied area are also checked to make sure they were not
 * changed.
 */
static void
test_single_memcpy(unsigned int off_src, unsigned int off_dst, size_t size)
{
	unsigned int i;
	uint8_t dest[SMALL_BUFFER_SIZE + ALIGNMENT_UNIT];
	uint8_t src[SMALL_BUFFER_SIZE + ALIGNMENT_UNIT];
	void * ret;

	/* Setup buffers */
	for (i = 0; i < SMALL_BUFFER_SIZE + ALIGNMENT_UNIT; i++) {
		dest[i] = 0;
		src[i] = (uint8_t) rte_rand();
	}

	/* Do the copy */
	ret = rte_memcpy(dest + off_dst, src + off_src, size);
	ASSERT_EQ((uintptr_t)ret, (uintptr_t)dest + off_dst);

	/* Check nothing before offset is affected */
	for (i = 0; i < off_dst; i++) {
		ASSERT_EQ(dest[i], 0);
	}

	/* Check everything was copied */
	for (i = 0; i < size; i++) {
		ASSERT_EQ(dest[i + off_dst], src[i + off_src]);
	}

	/* Check nothing after copy was affected */
	for (i = size; i < SMALL_BUFFER_SIZE; i++) {
		ASSERT_EQ(dest[i + off_dst], 0);
	}
}

/*
 * Check functionality for various buffer sizes and data offsets/alignments.
 */
TEST(test_rte, tst_memcpy)
{
	unsigned int off_src, off_dst, i;
	unsigned int num_buf_sizes = sizeof(buf_sizes) / sizeof(buf_sizes[0]);
	int ret;

	for (off_src = 0; off_src < ALIGNMENT_UNIT; off_src++) {
		for (off_dst = 0; off_dst < ALIGNMENT_UNIT; off_dst++) {
			for (i = 0; i < num_buf_sizes; i++) {
				test_single_memcpy(off_src, off_dst,
				                         buf_sizes[i]);
			}
		}
	}
}

int main(int argc,char**argv){

  testing::InitGoogleTest(&argc,argv);

  return RUN_ALL_TESTS();
}