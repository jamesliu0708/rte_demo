#include "rte_eal.h"
#include "rte_malloc.h"
#include "rte_mempool.h"
#include "rte_ring.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define RING_SIZE 4096
#define MEMPOOL_ELT_SIZE 2048
#define MAX_KEEP 16
#define MEMPOOL_SIZE ((rte_lcore_count()*(MAX_KEEP+RTE_MEMPOOL_CACHE_MAX_SIZE))-1)

const static int duration = 1000000000;

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

static void
my_mp_init(struct rte_mempool *mp, __attribute__((unused)) void *arg)
{
	printf("mempool name is %s\n", mp->name);
	/* nothing to be implemented here*/
	return ;
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

void run_single_producer()
{
    auto mp_spsc = rte_mempool_create("test_mempool_sp_sc", MEMPOOL_SIZE,
			MEMPOOL_ELT_SIZE, 0, 0,
			my_mp_init, NULL,
			my_obj_init, NULL,
			SOCKET_ID_ANY,
			MEMPOOL_F_NO_CACHE_ALIGN | MEMPOOL_F_SP_PUT |
			MEMPOOL_F_SC_GET);
    if (mp_spsc == NULL) {
        fprintf(stderr, "Failed to create mempool\n");
        return;
    }
    void* obj;
    auto ring = rte_ring_create("test", RING_SIZE, SOCKET_ID_ANY, 0);
    if (ring == NULL) {
        fprintf(stderr, "Failed to create ring\n");
    }
    for(int i = 0; i < RING_SIZE * 2; ++i) {
        if (rte_mempool_get(mp_spsc, &obj) == NULL) {
            obj = rte_malloc(NULL, MEMPOOL_ELT_SIZE, 0);
            if (obj == NULL) {
                fprintf(stderr, "Failed to get obj\n");
                exit(-1);
            }
        }
        int* val = (int*)obj;
        *val = i;
        while (te_ring_enqueue(ring, obj) != 0)
            asm volatile("pause":::"memory");
        fprintf(stdout, "produce %i\n", i);
    }
}

void run_single_consumer()
{
    auto mp_spsc = rte_mempool_lookup("test_mempool_sp_sc");
    if (mp_spsc == NULL) {
        fprintf(stderr, "Failed to find mempool\n");
        exit(-1);
    }
    auto ring = rte_ring_lookup("test");
    if (ring == NULL) {
        fprintf(stderr, "Failed to find ring\n");
        exit(-1);
    }
    for(int i = 0; i < RING_SIZE * 2; ++i) {
        void * obj;
        while(rte_ring_dequeue(ring, &obj) != 0)
            asm volatile("pause":::"memory");
        if (rte_mempool_put(mp_spsc, obj) )
    }
}

int main(int argc,char** argv)
{
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Failed to create process");
        exit(-1);
    } else if (pid == 0) {
        int ret = rte_eal_attach(argc, argv);
        if (ret < 0) {
            fprintf(stderr, "Failed to attach rte\n");
            exit(-1);
        }
        run_single_producer();
    }
    else {
        int ret = rte_eal_attach(argc, argv);
        if (ret < 0) {
            fprintf(stderr, "Failed to attach rte\n");
            exit(-1);
        }
        sleep(5);
        run_single_consumer();
    }
    return 0;
}