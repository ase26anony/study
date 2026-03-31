/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the switch statement in GCC's runtime that maps partitioning
 * type codes to human-readable strings.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -o test_omp test_omp_partitioning.c
 * Or for host-only testing: gcc -O3 -fopenmp -foffload=disable -o test_omp_host test_omp_partitioning.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = ARRAY_SIZE;
volatile int seed_factor = 1;

/* Non-inlined test functions to ensure separate compilation units */

__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int N = volatile_N;
    int factor = seed_factor + 1;  /* Make runtime-dependent */
    
    /* Case 0: gang redundant - scalar replicated across all gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        int private_scalar = factor;  /* firstprivate by default in teams */
        
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + private_scalar;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * 2;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int factor = seed_factor + 2;
    
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = factor + omp_get_thread_num();
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + gang_id * 100 + worker_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int factor = seed_factor + 3;
    
    /* Case 4: vector partitioned - SIMD vectorization */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        dst[i] = src[i] + gang_id * 10;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * (worker_id + 1);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 7: fully partitioned - three-level nesting */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int lane = i % 2;  /* Simulate vector lane */
        dst[i] = src[i] + gang_id * 1000 + worker_id * 100 + lane;
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    int N = volatile_N;
    int factor = seed_factor + 4;
    
    /* Mixed clauses to trigger various runtime paths */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                private(factor) firstprivate(N) \
                num_teams(3) num_threads(12) \
                depend(inout: dst[0:N]) nowait
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * factor + i;
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
        src[i] = i + seed_factor;
        dst[i] = 0;
    }
    
    printf("Testing OpenMP offload partitioning patterns...\n");
    
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
    
    /* Compute checksum to prevent optimization */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) reduction(+:checksum) \
                num_teams(2) num_threads(16)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    
    return 0;
}
