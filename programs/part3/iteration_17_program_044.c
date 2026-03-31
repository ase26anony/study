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
                             firstprivate(factor, offset) num_teams(4) thread_limit(128)
    {
        int gang_local = factor * 2;  /* This becomes gang redundant */
        #pragma omp distribute
        for (int i = 0; i < dynamic_N; i++) {
            dst[i] = src[i] * factor + offset + gang_local;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                             num_teams(4) thread_limit(128)
    {
        /* dst is gang partitioned */
        #pragma omp distribute
        for (int i = 0; i < dynamic_N; i++) {
            dst[i] = src[i] * 2;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < dynamic_N; i++) {
        int worker_local = i % 8;  /* Worker partitioned variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < dynamic_N; i++) {
        /* Both gang and worker partitioning occur here */
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * team_id + thread_id;
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < dynamic_N; i++) {
        int vector_private = i % 16;  /* Vector partitioned */
        dst[i] = src[i] + vector_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) simdlen(8)
    for (int i = 0; i < dynamic_N; i++) {
        int team_id = omp_get_team_num();
        int lane = i % 8;  /* Simd lane - vector partitioned */
        dst[i] = src[i] * team_id + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        int thread_id = omp_get_thread_num();
        int lane = i % 4;  /* Vector partitioned */
        dst[i] = src[i] + thread_id * 10 + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * team_id + thread_id * 100 + lane;
    }
}

/* Additional test with depend clause to vary runtime behavior */
__attribute__((noinline,optimize("O0")))
void test_with_depend(int *src, int *dst, int *tmp) {
    /* First kernel with output dependency */
    #pragma omp target teams distribute parallel for \
                map(tofrom: tmp[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                depend(out: tmp) nowait
    for (int i = 0; i < dynamic_N; i++) {
        tmp[i] = src[i] * 3;
    }
    
    /* Second kernel with input dependency */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: tmp[0:dynamic_N]) \
                depend(in: tmp)
    for (int i = 0; i < dynamic_N; i++) {
        dst[i] = tmp[i] + i;
    }
}

int main() {
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    srand(seed);
    for (int i = 0; i < N; i++) {
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
    test_with_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent optimization */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:dynamic_N])
    for (int i = 0; i < dynamic_N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
