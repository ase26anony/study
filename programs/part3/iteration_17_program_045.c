/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the switch statement in GCC's runtime that maps partitioning
 * type codes to human-readable strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = ARRAY_SIZE;
volatile int volatile_seed = 42;

/* Non-inlined test functions to ensure separate compilation units */

__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int N = volatile_N;
    int factor = volatile_seed % 10 + 1;
    
    /* Case 0: gang redundant - scalar replicated across all gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        int private_factor = factor;  /* firstprivate by default in teams region */
        
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * private_factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams distribute map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                         num_teams(4) thread_limit(32)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + i;
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int offset = volatile_seed % 100;
    
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:N]) \
                                                    map(to: src[0:N]) \
                                                    num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int local_offset = offset;  /* private to each worker */
        dst[i] = src[i] + local_offset + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:N]) \
                                                    map(to: src[0:N]) \
                                                    num_teams(4)
    for (int i = 0; i < N; i++) {
        /* Array access pattern that depends on both gang and worker */
        dst[i] = src[i] * omp_get_team_num() + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int factor = volatile_seed % 5 + 1;
    
    /* Case 4: vector partitioned - SIMD regions */
    #pragma omp target teams distribute simd map(tofrom: dst[0:N]) \
                                             map(to: src[0:N]) \
                                             num_teams(2) simdlen(8)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd map(tofrom: dst[0:N]) \
                                             map(to: src[0:N]) \
                                             num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + omp_get_team_num() * 1000;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:N]) \
                                                         map(to: src[0:N]) \
                                                         num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + omp_get_thread_num() * 100;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 7: fully partitioned - all three levels active */
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:N]) \
                                                         map(to: src[0:N]) \
                                                         num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        /* Computation using all partitioning levels */
        int team = omp_get_team_num();
        int thread = omp_get_thread_num();
        dst[i] = src[i] + team * 1000 + thread * 10 + (i % 2);
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    int N = volatile_N;
    int scalar = volatile_seed;
    
    /* Mixed data clauses to trigger various runtime paths */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) \
                map(to: src[0:N]) \
                firstprivate(scalar) \
                private(N) \
                num_teams(3) \
                nowait  /* Vary runtime behavior */
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + scalar + i;
    }
    #pragma omp taskwait
}

int main() {
    int N = volatile_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + (volatile_seed % 7);
    }
    
    /* Clear destination array */
    for (int i = 0; i < N; i++) {
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
    
    /* Final computation with reduction to prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:N]) \
                reduction(+:checksum) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional host-side verification */
    long long host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    printf("Host verification checksum: %lld\n", host_checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
