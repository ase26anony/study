#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent optimization and ensure runtime execution */
volatile int dynamic_N = N;
volatile int seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    int factor = 3;
    int offset = 7;
    
    /* Case 0: gang redundant - scalar replicated across all gangs */
    #pragma omp target teams map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                              firstprivate(factor, offset) num_teams(4)
    {
        int gang_local = factor + offset;  /* Will be gang redundant */
        #pragma omp distribute
        for (int i = 0; i < dynamic_N; i++) {
            dst[i] = src[i] * gang_local;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                              num_teams(8) thread_limit(32)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_N; i++) {
            dst[i] = src[i] * 2;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(2) num_threads(16)
    for (int i = 0; i < dynamic_N; i++) {
        int worker_local = i % 8;  /* Worker partitioned variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4)
    for (int i = 0; i < dynamic_N; i++) {
        /* Both gang and worker partitioning occur here */
        int tid = omp_get_thread_num();
        int team = omp_get_team_num();
        dst[i] = src[i] * (tid + 1) * (team + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vectorization */
    int factor = 5;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                simdlen(8) num_teams(2)
    for (int i = 0; i < dynamic_N; i++) {
        int vector_private = factor + (i % 4);  /* Vector partitioned */
        dst[i] = src[i] * vector_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        /* Both gang and vector partitioning */
        int team = omp_get_team_num();
        dst[i] = src[i] * (team + 1) * ((i % 8) + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        /* Worker and vector partitioning */
        int tid = omp_get_thread_num();
        dst[i] = src[i] * (tid + 1) * ((i % 4) + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        /* All three levels of partitioning */
        int team = omp_get_team_num();
        int tid = omp_get_thread_num();
        int lane = i % 4;  /* Simulates vector lane */
        dst[i] = src[i] * (team + 1) * (tid + 1) * (lane + 1);
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_clauses(int *src, int *dst) {
    /* Additional test with mixed data clauses */
    int scalar_private = 10;
    int scalar_firstprivate = 20;
    int arr_partial[N/2];
    
    /* Initialize partial array */
    for (int i = 0; i < N/2; i++) {
        arr_partial[i] = i * 2;
    }
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N], arr_partial[0:dynamic_N/2]) \
                map(to: src[0:dynamic_N]) \
                firstprivate(scalar_firstprivate) private(scalar_private) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < dynamic_N; i++) {
        scalar_private = i % 16;
        if (i < dynamic_N/2) {
            dst[i] = src[i] * scalar_private + scalar_firstprivate + arr_partial[i];
        } else {
            dst[i] = src[i] * scalar_private + scalar_firstprivate;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_nowait_depend(int *src, int *dst, int *tmp) {
    /* Test with nowait and depend clauses for async execution */
    #pragma omp target teams distribute parallel for \
                map(tofrom: tmp[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                depend(out: tmp) nowait \
                num_teams(2)
    for (int i = 0; i < dynamic_N; i++) {
        tmp[i] = src[i] * 3;
    }
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(tofrom: tmp[0:dynamic_N]) \
                depend(in: tmp) \
                num_teams(2)
    for (int i = 0; i < dynamic_N; i++) {
        dst[i] = tmp[i] + 7;
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent constant folding */
    volatile int array_size = dynamic_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(array_size * sizeof(int));
    int *dst = (int*)malloc(array_size * sizeof(int));
    int *tmp = (int*)malloc(array_size * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = i + 1;
        dst[i] = 0;
        tmp[i] = 0;
    }
    
    printf("Starting OpenMP offload tests...\n");
    
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
    test_nowait_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:array_size]) \
                num_teams(4)
    for (int i = 0; i < array_size; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional host-side verification */
    long long host_checksum = 0;
    for (int i = 0; i < array_size; i++) {
        host_checksum += dst[i];
    }
    printf("Host verification checksum: %lld\n", host_checksum);
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
