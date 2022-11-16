#include <gtest/gtest.h>/
#include "rte_eal.h"
#include "rte_debug.h"
#include "rte_log.h"
#include "process.h"

const char *prgname;
#define no_huge "--no-huge"
#define no_shconf "--no-shconf"
#define mp_flag "--proc-type=secondary"
#define launch_proc(ARGV) process_dup(ARGV, \
		sizeof(ARGV)/(sizeof(ARGV[0])), __func__)
  
/*
 * Test that the app doesn't run with invalid -n flag option.
 * Final test ensures it does run with valid options as sanity check
 * Since -n is not compulsory for MP, we instead use --no-huge and --no-shconf
 * flags.
 */
TEST(test_eal, test_invalid_n_flag)
{
    /* -n flag but no value */
    const char *argv1[] = { prgname, no_huge, no_shconf, "-c", "1", "-n"};
    /* bad numeric value */
    const char *argv2[] = { prgname, no_huge, no_shconf, "-c", "1", "-n", "e" };
    /* zero is invalid */
    const char *argv3[] = { prgname, no_huge, no_shconf, "-c", "1", "-n", "0" };
    /* sanity test - check with good value */
    const char *argv4[] = { prgname, no_huge, no_shconf, "-c", "1", "-n", "2" };
    /* sanity test - check with no -n flag */
    const char *argv5[] = { prgname, no_huge, no_shconf, "-c", "1"};

    ASSERT_NE(launch_proc(argv1), 0)
}

/*
 * Test that the app runs with --no-huge and doesn't run when --socket-mem are
 * specified with --no-huge.
 */
TEST(test_eal, test_no_huge_flag)
{

}

/*
 * Tests for correct handling of -m and --socket-mem flags
 */
TEST(test_eal, test_memory_flags)
{

}

TEST(test_eal, test_file_prefix)
{

}

int main(int argc,char**argv){

    testing::InitGoogleTest(&argc,argv);

    return RUN_ALL_TESTS();
}