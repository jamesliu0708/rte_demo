#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <gtest/gtest.h>

#include "rte_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

int
rte_eal_cleanup(void)
{
	return 0;
}

#ifdef __cplusplus
}
#endif

/* use fork() to test rte_panic() */
TEST(test_rte, test_panic)
{
    int pid;
	int status;

	pid = fork();

	if (pid == 0)
		rte_panic("Test Debug\n");
	else if (pid < 0){
		printf("Fork Failed\n");
        return;
	}
	wait(&status);
	if(status == 0){
		printf("Child process terminated normally!\n");
		return;
	} else
		printf("Child process terminated as expected - Test passed!\n");

	return;
}

/* use fork() to test rte_exit() */
static int
test_exit_val(int exit_val)
{
	int pid;
	int status;

	pid = fork();

	if (pid == 0)
		rte_exit(exit_val, __func__);
	else if (pid < 0){
		printf("Fork Failed\n");
		return -1;
	}
	wait(&status);
	printf("Child process status: %d\n", status);
#ifndef RTE_EAL_ALWAYS_PANIC_ON_ERROR
	if(!WIFEXITED(status) || WEXITSTATUS(status) != (uint8_t)exit_val){
		printf("Child process terminated with incorrect status (expected = %d)!\n",
				exit_val);
		return -1;
	}
#endif
	return 0;
}

TEST(test_rte, test_exit)
{
	int test_vals[] = { 0, 1, 2, 255, -1 };
	unsigned i;
	for (i = 0; i < sizeof(test_vals) / sizeof(test_vals[0]); i++){
		ASSERT_EQ (test_exit_val(test_vals[i]) , 0);
	}
}

TEST(test_rte, test_debug)
{
    rte_dump_stack();
	rte_dump_registers();
}

int main(int argc,char**argv){

    testing::InitGoogleTest(&argc,argv);

    return RUN_ALL_TESTS();
}