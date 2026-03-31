#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = N;
volatile int seed = 42;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;
    int offset = 10;
    
    #pragma omp target teams map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                             firstprivate(factor, offset) num_teams(4) thread_limit(128)
    {
        /* Case 0: gang redundant - firstprivate scalar replicated across gangs */
        int gang_local = factor * offset;
        
        #pragma omp distribute
        for (int i = 0; i < volatile_N; i++) {
            dst[i] = src[i] + gang_local;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - mapped array partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                             num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (int i = 0; i < volatile_N; i++) {
            dst[i] = src[i] * 3;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - variable in parallel region */
    int chunk_size = volatile_N / 4;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < volatile_N; i++) {
        int worker_local = i % 16;  /* Worker-partitioned variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < volatile_N; i++) {
        /* Both gang and worker partitioning occur here */
        dst[i] = src[i] * (omp_get_team_num() + 1) + omp_get_thread_num();
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vectorization */
    int factor = 5;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                simdlen(8) num_teams(2)
    for (int i = 0; i < volatile_N; i++) {
        /* Vector-private computation */
        int vec_tmp = src[i] * factor;
        dst[i] = vec_tmp + (i & 0xF);  /* Add some variation */
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < volatile_N; i++) {
        /* Both gang and vector partitioning */
        dst[i] = src[i] * (omp_get_team_num() + 2) + (i % 8);
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    #pragma omp target parallel for simd \
                map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                num_threads(8) simdlen(4)
    for (int i = 0; i < volatile_N; i++) {
        /* Worker and vector partitioning */
        int thread_factor = omp_get_thread_num() + 1;
        dst[i] = src[i] * thread_factor + (i & 0x7);
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned - teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:volatile_N]) map(to: src[0:volatile_N]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < volatile_N; i++) {
        /* Fully partitioned across gang, worker, and vector */
        int team = omp_get_team_num();
        int thread = omp_get_thread_num();
        int lane = i % 4;  /* Simulated vector lane */
        dst[i] = src[i] * (team + 1) + thread * 10 + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_clauses(int *src, int *dst) {
    /* Mixed data clauses to trigger various partitioning combinations */
    int scalar_private = 100;
    int scalar_firstprivate = 200;
    int arr_partial[64];
    
    /* Initialize partial array */
    for (int i = 0; i < 64; i++) {
        arr_partial[i] = i;
    }
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:volatile_N]) \
                map(to: src[0:volatile_N], arr_partial[0:64]) \
                firstprivate(scalar_firstprivate) \
                private(scalar_private) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < volatile_N; i++) {
        scalar_private = omp_get_thread_num();
        dst[i] = src[i] + scalar_firstprivate + scalar_private + arr_partial[i % 64];
    }
}

__attribute__((noinline,optimize("O0")))
void test_nowait_depend(int *src, int *dst, int *tmp) {
    /* Test with nowait and depend clauses for async execution */
    #pragma omp target teams map(to: src[0:volatile_N]) map(from: tmp[0:volatile_N]) \
                             nowait depend(out: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < volatile_N; i++) {
            tmp[i] = src[i] * 2;
        }
    }
    
    #pragma omp target teams map(to: tmp[0:volatile_N]) map(from: dst[0:volatile_N]) \
                             nowait depend(in: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < volatile_N; i++) {
            dst[i] = tmp[i] + 1;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int actual_N = volatile_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(actual_N * sizeof(int));
    int *dst = (int*)malloc(actual_N * sizeof(int));
    int *tmp = (int*)malloc(actual_N * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    srand(seed);
    for (int i = 0; i < actual_N; i++) {
        src[i] = rand() % 100;
        dst[i] = 0;
        tmp[i] = 0;
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
    test_mixed_clauses(src, dst);
    test_nowait_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:actual_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < actual_N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
