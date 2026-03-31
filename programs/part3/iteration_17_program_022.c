/* Test program to exercise GCC's OpenMP data partitioning string mapping function.
   Designed to trigger all cases in the switch statement returning strings like
   "gang redundant", "gang partitioned", etc. from omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Use volatile to prevent constant folding and dead code elimination */
volatile int global_N = ARRAY_SIZE;

/* Non-inline functions to ensure separate compilation units/regions */

__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    /* Case 0: gang redundant - scalar firstprivate in teams region */
    int factor = 2;  /* Will be made firstprivate */
    #pragma omp target teams map(to:src[0:global_N]) map(from:dst[0:global_N]) \
        num_teams(4) thread_limit(128)
    {
        int local_factor = factor;  /* firstprivate behavior */
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - mapped array in teams region */
    int offset = rand() % 100;  /* Prevent optimization */
    #pragma omp target teams map(to:src[0:global_N]) map(tofrom:dst[0:global_N]) \
        num_teams(8)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] += src[i] + offset;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - variable in parallel region */
    volatile int chunk = global_N / 16;
    #pragma omp target teams distribute parallel for \
        map(to:src[0:global_N]) map(from:dst[0:global_N]) \
        num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        int worker_local = i % 32;  /* Worker-partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - shared array with two-level parallelism */
    int tile_size = 64;
    #pragma omp target teams distribute parallel for \
        map(to:src[0:global_N]) map(tofrom:dst[0:global_N]) \
        num_teams(4) num_threads(16)
    for (int i = 0; i < global_N; i++) {
        /* Both gang and worker dimensions active */
        dst[i] = src[i] * (1 + (i / tile_size) % 4);
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - simd construct */
    int vector_factor = 4;
    #pragma omp target teams distribute simd \
        map(to:src[0:global_N]) map(from:dst[0:global_N]) \
        simdlen(8) num_teams(1)
    for (int i = 0; i < global_N; i++) {
        /* Vector-private computation */
        dst[i] = src[i] / vector_factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned - teams distribute simd */
    volatile int mod_val = 7;
    #pragma omp target teams distribute simd \
        map(to:src[0:global_N]) map(tofrom:dst[0:global_N]) \
        num_teams(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        dst[i] = src[i] % mod_val;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned - parallel for simd */
    int vec_private_var = 3;
    #pragma omp target teams distribute parallel for simd \
        map(to:src[0:global_N]) map(from:dst[0:global_N]) \
        num_teams(1) num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        /* Both worker and vector partitioning */
        dst[i] = src[i] * vec_private_var + (i % 16);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned - teams distribute parallel for simd */
    volatile int fully_private = 2;
    #pragma omp target teams distribute parallel for simd \
        map(to:src[0:global_N]) map(tofrom:dst[0:global_N]) \
        num_teams(4) num_threads(8) simdlen(4) \
        private(fully_private)
    for (int i = 0; i < global_N; i++) {
        /* All three levels: gang, worker, and vector */
        fully_private = 1 + (i % 8);
        dst[i] = src[i] * fully_private + omp_get_team_num() + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    /* Additional test with mixed data clauses to trigger various mappings */
    int gang_private = 10;
    int worker_private = 20;
    int vector_private = 30;
    
    #pragma omp target teams distribute parallel for simd \
        map(to:src[0:global_N]) map(from:dst[0:global_N]) \
        firstprivate(gang_private) private(worker_private) \
        linear(vector_private:1) \
        num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < global_N; i++) {
        worker_private = omp_get_thread_num();
        dst[i] = src[i] + gang_private + worker_private + vector_private++;
    }
}

int main() {
    int N = global_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i % 100;
        dst[i] = 0;
    }
    
    /* Call all test functions to exercise different partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_clauses(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
        map(tofrom:checksum) map(to:dst[0:N]) \
        num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    /* Also compute on host for verification */
    int host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    
    printf("Device checksum: %d\n", checksum);
    printf("Host checksum: %d\n", host_checksum);
    printf("Difference: %d\n", checksum - host_checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
