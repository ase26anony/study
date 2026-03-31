/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the runtime string mapping function for all switch cases.
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -o test test_omp_partitioning.c
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
    int offset = 10; /* Will be private */
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        factor = 3; /* Modification to test firstprivate behavior */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            /* Each gang gets its own copy of factor (gang redundant) */
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int shared_var = 5; /* Will be gang partitioned through map */
    
    #pragma omp target teams map(tofrom: dst[0:N], shared_var) map(to: src[0:N]) \
                             num_teams(8)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            /* shared_var is partitioned across gangs */
            dst[i] = src[i] + shared_var + omp_get_team_num();
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        /* Each worker thread gets private loop variable i */
        int worker_local = omp_get_thread_num();
        dst[i] = src[i] * worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int tile_size = 16;
    
    #pragma omp target teams distribute parallel for collapse(2) \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i += tile_size) {
        for (int j = 0; j < tile_size && (i + j) < N; j++) {
            /* Combined gang+worker partitioning */
            int gang_id = omp_get_team_num();
            int worker_id = omp_get_thread_num();
            dst[i + j] = src[i + j] + gang_id * 100 + worker_id;
        }
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < N; i++) {
        /* Vector partitioning within SIMD lanes */
        dst[i] = src[i] * 2 + (i % 4); /* i%4 simulates vector lane */
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Combined gang+vector partitioning */
        int gang_id = omp_get_team_num();
        dst[i] = src[i] + gang_id * 10 + (i % 4);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Combined worker+vector partitioning */
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * worker_id + (i % 4);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int tile = 8;
    
    /* Fully partitioned: gang × worker × vector */
    #pragma omp target teams distribute parallel for simd collapse(2) \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i += tile) {
        for (int j = 0; j < tile && (i + j) < N; j++) {
            int gang_id = omp_get_team_num();
            int worker_id = omp_get_thread_num();
            int vector_lane = j % 2;
            dst[i + j] = src[i + j] + gang_id * 1000 + worker_id * 100 + vector_lane;
        }
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    int N = volatile_N;
    
    /* Mix different data clauses in same region */
    int gang_private = 1;
    int worker_private = 2;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                private(worker_private) firstprivate(gang_private) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < N; i++) {
        worker_private = omp_get_thread_num();
        dst[i] = src[i] + gang_private * 100 + worker_private;
    }
}

int main() {
    int N = volatile_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + (rand() % 10); /* Prevent optimization */
        dst[i] = 0;
    }
    
    printf("Testing all OpenMP offload partitioning types...\n");
    
    /* Exercise all partitioning cases */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_partitioning(src, dst);
    
    /* Final reduction to compute checksum and prevent elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
