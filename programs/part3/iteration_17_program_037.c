/* Test program to exercise OpenMP/OpenACC data partitioning string mapping
   Specifically targets the switch cases in omp-oacc-neuter-broadcast.cc
   lines 335-343 for gang/worker/vector partitioning types */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int dynamic_size = ARRAY_SIZE;
volatile int seed_factor = 1;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    /* Case 0: gang redundant - scalar replicated across gangs */
    int factor = 3;  /* Will be firstprivate in teams region */
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) \
                             map(to: src[0:dynamic_size]) \
                             num_teams(4) thread_limit(64)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            /* factor is gang-redundant (firstprivate by default in teams) */
            dst[i] = src[i] * factor + i;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    int local_factor = 2;
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) \
                             map(to: src[0:dynamic_size]) \
                             num_teams(4) thread_limit(128)
    {
        /* dst array is gang-partitioned */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] * local_factor + (i % 32);
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within parallel region */
    int chunk_size = dynamic_size / 4;
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < dynamic_size; i++) {
        /* Each worker gets its own private copy of worker_var */
        int worker_var = omp_get_thread_num() + 1;
        dst[i] = src[i] * worker_var + (i % 16);
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    int tile_size = 32;
    #pragma omp target teams distribute parallel for collapse(2) \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < dynamic_size; i += tile_size) {
        for (int j = 0; j < tile_size && (i + j) < dynamic_size; j++) {
            /* Combined gang and worker partitioning */
            int idx = i + j;
            dst[idx] = src[idx] * (omp_get_team_num() + 1) + 
                      (omp_get_thread_num() + 1);
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vectorization */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                simdlen(8) num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        /* Vector-private computation */
        dst[i] = src[i] * 5 + (i & 7);
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        /* Combined gang and vector partitioning */
        dst[i] = src[i] * (omp_get_team_num() + 2) + (i & 3);
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        /* Combined worker and vector partitioning */
        dst[i] = src[i] * (omp_get_thread_num() + 3) + (i & 3);
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma omp target teams distribute parallel for simd collapse(2) \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < dynamic_size; i += 16) {
        for (int j = 0; j < 16 && (i + j) < dynamic_size; j++) {
            int idx = i + j;
            /* Fully partitioned across all levels */
            dst[idx] = src[idx] * (omp_get_team_num() + 1) +
                      (omp_get_thread_num() + 1) * 10 +
                      (j & 1);
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mixed case to potentially trigger default case */
    int mode = dynamic_size % 9;  /* 0-8, where 8 is illegal for switch */
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        /* Use mode to vary computation */
        switch (mode) {
            case 0: dst[i] = src[i] * 1; break;
            case 1: dst[i] = src[i] * 2; break;
            case 2: dst[i] = src[i] * 3; break;
            case 3: dst[i] = src[i] * 4; break;
            case 4: dst[i] = src[i] * 5; break;
            case 5: dst[i] = src[i] * 6; break;
            case 6: dst[i] = src[i] * 7; break;
            case 7: dst[i] = src[i] * 8; break;
            default: dst[i] = src[i] * 9; break;  /* Could trigger default */
        }
    }
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = dynamic_size;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i * seed_factor + (i % 17);
        dst[i] = 0;
    }
    
    printf("Starting OpenMP offload partitioning tests...\n");
    
    /* Execute all test patterns to cover switch cases */
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
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
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
