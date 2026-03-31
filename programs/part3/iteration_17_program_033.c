#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int global_N = SIZE;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;
    int offset = 1;
    
    /* Case 0: gang redundant - scalar variable replicated across gangs */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(factor, offset) num_teams(4)
    {
        int gang_local = factor * 10;  /* This becomes gang redundant */
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * factor + offset + gang_local;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * 3;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < global_N; i++) {
        int worker_local = i % 10;  /* Worker partitioned variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        /* Both gang and worker partitioning occurs here */
        dst[i] = src[i] * (omp_get_team_num() + 1) + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD region */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        int vector_local = i & 0xF;  /* Vector partitioned */
        dst[i] = src[i] ^ vector_local;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        /* Gang and vector partitioning */
        dst[i] = src[i] + (omp_get_team_num() << 4) + (i & 0xF);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    #pragma omp target parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        /* Worker and vector partitioning */
        int lane = i & 0x3;  /* Vector lane */
        dst[i] = src[i] * omp_get_thread_num() + lane;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned - all three levels */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(4) simdlen(2)
    for (int i = 0; i < global_N; i++) {
        /* Fully partitioned across gang, worker, and vector */
        dst[i] = src[i] + (omp_get_team_num() << 8) + 
                 (omp_get_thread_num() << 4) + (i & 0x3);
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    /* Additional test with multiple data clauses */
    int shared_var = 100;
    int private_var = 50;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(shared_var) private(private_var) \
                num_teams(3) num_threads(6)
    for (int i = 0; i < global_N; i++) {
        private_var = omp_get_thread_num();
        dst[i] = src[i] + shared_var + private_var;
    }
}

__attribute__((noinline))
void test_nowait_regions(int *src, int *dst) {
    /* Test with nowait to create multiple concurrent regions */
    #pragma omp target teams nowait \
                map(tofrom: dst[0:global_N/2]) map(to: src[0:global_N/2]) \
                num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N/2; i++) {
            dst[i] = src[i] * 2;
        }
    }
    
    #pragma omp target teams nowait \
                map(tofrom: dst[global_N/2:global_N/2]) \
                map(to: src[global_N/2:global_N/2]) \
                num_teams(2)
    {
        #pragma omp distribute
        for (int i = global_N/2; i < global_N; i++) {
            dst[i] = src[i] * 3;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = global_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + (rand() % 10);  /* Add some randomness */
        dst[i] = 0;
    }
    
    printf("Starting OpenMP offload tests...\n");
    
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
    test_nowait_regions(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
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
    
    return 0;
}
