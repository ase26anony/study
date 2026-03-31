/* test_omp_partitioning.c
 * Comprehensive test for OpenMP data partitioning types
 * Designed to trigger all cases in omp-oacc-neuter-broadcast.cc:335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_N = ARRAY_SIZE;
volatile int g_seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    int N = g_volatile_N;
    int factor = g_seed % 10 + 1;  /* Runtime-dependent */
    
    /* Case 0: gang redundant - firstprivate scalar replicated across gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(64)
    {
        int local_factor = factor;  /* firstprivate by default in teams region */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    
    /* Case 1: gang partitioned - array mapped across gangs */
    #pragma omp target teams distribute map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                         num_teams(8)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + i;  /* Simple gang-partitioned computation */
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    int offset = g_seed % 100;
    
    /* Case 2: worker partitioned - variable in parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(32)
    for (int i = 0; i < N; i++) {
        int worker_local = offset + omp_get_thread_num();
        dst[i] = src[i] * worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4)
    for (int i = 0; i < N; i++) {
        /* Both gang and worker dimensions active */
        dst[i] = src[i] * (omp_get_team_num() + 1) * (omp_get_thread_num() + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    int vector_factor = g_seed % 5 + 1;
    
    /* Case 4: vector partitioned - SIMD with vector-private variable */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < N; i++) {
        int vector_private = vector_factor * (i % 16);
        dst[i] = src[i] + vector_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Both gang and vector dimensions active */
        dst[i] = src[i] * (omp_get_team_num() + 1) * ((i % 4) + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    
    /* Case 6: worker+vector partitioned - parallel for SIMD */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Worker and vector dimensions active */
        dst[i] = src[i] * (omp_get_thread_num() + 1) * ((i % 4) + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    int N = g_volatile_N;
    
    /* Case 7: fully partitioned - all three levels active */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < N; i++) {
        /* Gang, worker, and vector dimensions all active */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int vector_id = i % 2;
        dst[i] = src[i] * (gang_id + 1) * (worker_id + 1) * (vector_id + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_clauses(int *src, int *dst) {
    int N = g_volatile_N;
    
    /* Additional test with mixed data clauses to trigger various paths */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                private(src) firstprivate(N) \
                num_teams(3) num_threads(16)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * 2;
    }
}

__attribute__((noinline,optimize("O0")))
void test_nowait_regions(int *src, int *dst, int *dst2) {
    int N = g_volatile_N;
    
    /* Test with nowait to create multiple concurrent regions */
    #pragma omp target teams nowait \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * 3;
        }
    }
    
    #pragma omp target teams nowait \
                map(tofrom: dst2[0:N]) map(to: src[0:N]) \
                num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst2[i] = src[i] * 4;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    int N = g_volatile_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *dst2 = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !dst2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + 1;
        dst[i] = 0;
        dst2[i] = 0;
    }
    
    printf("Starting OpenMP partitioning tests...\n");
    
    /* Execute all test functions to trigger different partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_clauses(src, dst);
    test_nowait_regions(src, dst, dst2);
    
    /* Final reduction to compute checksum and prevent optimization */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(dst2);
    
    return 0;
}
