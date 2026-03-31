/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the runtime's data partitioning type string mapping function.
 * Each test function targets a specific case in the switch statement:
 *   case 0: gang redundant
 *   case 1: gang partitioned  
 *   case 2: worker partitioned
 *   case 3: gang+worker partitioned
 *   case 4: vector partitioned
 *   case 5: gang+vector partitioned
 *   case 6: worker+vector partitioned
 *   case 7: fully partitioned
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
    int N = volatile_N;
    int factor = 2;  /* Will be firstprivate in teams region */
    
    #pragma omp target teams map(to: src[0:N]) map(from: dst[0:N]) \
                             firstprivate(factor) num_teams(4)
    {
        /* factor is gang redundant - replicated across all teams/gangs */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline))  
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Array is gang partitioned across teams */
    #pragma omp target teams map(to: src[0:N]) map(from: dst[0:N]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + omp_get_team_num();
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        /* Each worker thread gets its own private copy of thread_id */
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * thread_id;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Two-level partitioning: across gangs and workers */
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(4) num_threads(4)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] + team_id * 100 + thread_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Vector partitioning in SIMD loops */
    #pragma omp target teams distribute simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < N; i++) {
        /* Vector-private computation */
        dst[i] = src[i] * (i % 8);
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Combined gang and vector partitioning */
    #pragma omp target teams distribute simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        dst[i] = src[i] + team_id * (i % 4);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Worker+vector partitioning using parallel for simd */
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(1) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * thread_id + (i % 4);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Fully partitioned across gangs, workers, and vectors */
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] + team_id * 1000 + thread_id * 100 + lane;
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    int N = volatile_N;
    
    /* Mix different data clauses to trigger various runtime paths */
    int private_var = 42;
    int firstprivate_var = 7;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                private(private_var) firstprivate(firstprivate_var) \
                num_teams(3) num_threads(2)
    for (int i = 0; i < N; i++) {
        private_var = omp_get_thread_num();
        dst[i] = src[i] + private_var + firstprivate_var;
    }
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
        src[i] = i;
        dst[i] = 0;
    }
    
    printf("Testing OpenMP offload data partitioning patterns...\n");
    
    /* Execute all test patterns */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_clauses(src, dst);
    
    /* Final reduction to compute checksum and prevent optimizations */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(to: dst[0:N]) map(tofrom: checksum) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    
    return 0;
}
