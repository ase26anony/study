/* test_omp_partitioning.c
 * 
 * This program exercises OpenMP offload constructs to trigger
 * all data partitioning type cases in GCC's runtime string mapping function.
 * Each test function corresponds to a specific partitioning case.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = ARRAY_SIZE;

/* Test functions with noinline to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    /* Case 0: gang redundant - scalar replicated across all gangs */
    int factor = 2;  /* Will be firstprivate in teams region */
    int N = volatile_N;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(64)
    {
        int local_factor = factor;  /* Becomes gang-redundant */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    int N = volatile_N;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(64)
    {
        /* dst is gang-partitioned through map clause */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + i;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - variable partitioned across workers */
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        /* Each worker gets its own partition of the loop */
        int worker_val = omp_get_thread_num();  /* Worker-specific */
        dst[i] = src[i] * worker_val;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        /* Combined gang and worker partitioning */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + gang_id * 1000 + worker_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vector partitioning */
    int N = volatile_N;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* Vector-partitioned computation */
        dst[i] = src[i] * 3;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned */
    int N = volatile_N;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Gang and vector partitioning */
        int gang_id = omp_get_team_num();
        dst[i] = src[i] + gang_id * 10;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned */
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Worker and vector partitioning */
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * worker_id;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned (gang+worker+vector) */
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        /* Fully partitioned across all levels */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int lane = i % 2;  /* Simulated vector lane */
        dst[i] = src[i] + gang_id * 100 + worker_id * 10 + lane;
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mixed test with multiple clauses to trigger various cases */
    int N = volatile_N;
    int factor = 3;
    
    /* Use nowait to vary runtime behavior */
    #pragma omp target map(tofrom: dst[0:N]) map(to: src[0:N]) nowait
    #pragma omp teams distribute parallel for simd \
                private(factor) num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < N; i++) {
        factor = omp_get_thread_num() + 1;
        dst[i] = src[i] * factor;
    }
    #pragma omp taskwait
}

int main() {
    int N = volatile_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i % 100;  /* Prevent overflow in computations */
        dst[i] = 0;
    }
    
    printf("Starting OpenMP partitioning tests...\n");
    
    /* Execute all test cases to trigger different partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_partitioning(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:N]) \
                reduction(+:checksum) num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    
    return 0;
}
