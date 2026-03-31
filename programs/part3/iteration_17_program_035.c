#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int dynamic_size = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline, noipa))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) \
                             map(to: src[0:dynamic_size]) \
                             firstprivate(factor) num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline, noipa))
void test_gang_partitioned(int *src, int *dst) {
    int offset = 3;
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) \
                             map(to: src[0:dynamic_size]) \
                             private(offset) num_teams(4)
    {
        offset = omp_get_team_num() + 1;  /* Different per gang */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] + offset;
        }
    }
}

__attribute__((noinline, noipa))
void test_worker_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        int worker_local = omp_get_thread_num();  /* Worker-partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline, noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < dynamic_size; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * gang_id + worker_id;
    }
}

__attribute__((noinline, noipa))
void test_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < dynamic_size; i++) {
        /* Vector-private computation */
        int lane = i % 8;  /* Simulating vector lane */
        dst[i] = src[i] * lane;
    }
}

__attribute__((noinline, noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                simdlen(4) num_teams(4)
    for (int i = 0; i < dynamic_size; i++) {
        int gang_id = omp_get_team_num();
        int lane = i % 4;
        dst[i] = src[i] * gang_id + lane;
    }
}

__attribute__((noinline, noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        int worker_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * worker_id + lane;
    }
}

__attribute__((noinline, noipa))
void test_fully_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * gang_id + worker_id + lane;
    }
}

/* Additional test with nowait to vary runtime behavior */
__attribute__((noinline, noipa))
void test_with_nowait(int *src, int *dst, int *dst2) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                nowait num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * 2;
    }
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst2[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                nowait num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        dst2[i] = src[i] + 1;
    }
    
    #pragma omp taskwait
}

/* Test with depend clauses */
__attribute__((noinline, noipa))
void test_with_depend(int *src, int *dst, int *tmp) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: tmp[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                depend(out: tmp) num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        tmp[i] = src[i] * 3;
    }
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: tmp[0:dynamic_size]) \
                depend(in: tmp) num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = tmp[i] + 5;
    }
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = dynamic_size;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    int *dst2 = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !tmp || !dst2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i % 100;  /* Prevent large numbers */
        dst[i] = 0;
        tmp[i] = 0;
        dst2[i] = 0;
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
    test_with_nowait(src, dst, dst2);
    test_with_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:N]) \
                reduction(+:checksum) num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(tmp);
    free(dst2);
    
    return 0;
}
