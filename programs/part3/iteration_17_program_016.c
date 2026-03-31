/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the runtime string mapping function in omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement cases 0-7 for gang/worker/vector partitioning.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = ARRAY_SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int N = volatile_N;
    int factor = 2;  /* Will be firstprivate */
    
    #pragma omp target teams map(to: src[0:N]) map(from: dst[0:N]) \
                             firstprivate(factor) num_teams(4) thread_limit(128)
    {
        int local_factor = factor;  /* gang-redundant (case 0) */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int offset = 5;
    
    /* gang partitioned (case 1) - array partitioned across gangs */
    #pragma omp target teams map(to: src[0:N]) map(tofrom: dst[0:N]) \
                             map(to: offset) num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] += src[i] + offset;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* worker partitioned (case 2) - within parallel region */
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = i % 16;  /* worker-partitioned variable */
        dst[i] = src[i] * worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int shared_offset = 3;
    
    /* gang+worker partitioned (case 3) - two-level partitioning */
    #pragma omp target teams map(to: src[0:N], shared_offset) map(tofrom: dst[0:N]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int g = 0; g < N/64; g++) {
            #pragma omp parallel for
            for (int w = 0; w < 64; w++) {
                int i = g * 64 + w;
                if (i < N) {
                    int worker_val = w;  /* worker partitioned */
                    dst[i] = src[i] + shared_offset + worker_val;
                }
            }
        }
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* vector partitioned (case 4) - SIMD vectorization */
    #pragma omp target teams distribute simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < N; i++) {
        int vector_private = i & 0xF;  /* vector partitioned */
        dst[i] = src[i] ^ vector_private;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* gang+vector partitioned (case 5) */
    #pragma omp target teams distribute simd \
                map(to: src[0:N]) map(tofrom: dst[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int lane = i % 4;  /* vector partitioned within gang partitioned */
        dst[i] = src[i] * lane;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* worker+vector partitioned (case 6) */
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int worker_lane = (i / 4) % 4;  /* combined worker+vector partitioning */
        dst[i] = src[i] + worker_lane;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* fully partitioned (case 7) - gang, worker, and vector levels */
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:N]) map(tofrom: dst[0:N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        /* Each iteration has unique gang, worker, and vector context */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int vector_lane = i % 2;
        dst[i] = src[i] + gang_id * 1000 + worker_id * 100 + vector_lane;
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    int N = volatile_N;
    
    /* Mixed clauses to trigger various runtime paths */
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                private(N) firstprivate(src) shared(dst) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < volatile_N; i++) {
        int private_var = i;  /* private to each thread */
        dst[i] = src[i] * private_var;
    }
}

int main() {
    int N = volatile_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i % 100;
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
    test_mixed_partitioning(src, dst);
    
    /* Final reduction to compute checksum and prevent optimization */
    int checksum = 0;
    #pragma omp target teams map(tofrom: checksum) map(to: dst[0:N]) \
                             reduction(+:checksum) num_teams(1)
    {
        #pragma omp distribute parallel for reduction(+:checksum)
        for (int i = 0; i < N; i++) {
            checksum += dst[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional host-side verification */
    int host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    printf("Host verification checksum: %d\n", host_checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
