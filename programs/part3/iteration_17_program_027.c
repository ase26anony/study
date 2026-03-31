#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent optimization and constant folding */
volatile int global_N = SIZE;
volatile int seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,noipa))
void test_gang_redundant(int *src, int *dst) {
    int N = global_N;
    int factor = seed % 10 + 1;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        /* Case 0: gang redundant - firstprivate scalar */
        int gang_local = factor;
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + gang_local;
        }
    }
}

__attribute__((noinline,noipa))
void test_gang_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 1: gang partitioned - mapped array */
    #pragma omp target teams distribute map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                        num_teams(8)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * 2;
    }
}

__attribute__((noinline,noipa))
void test_worker_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 2: worker partitioned - parallel for private */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = i % 16;  /* Worker-partitioned computation */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 3: gang+worker partitioned - nested parallelism */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        /* Shared across workers within a gang, partitioned across gangs */
        dst[i] = src[i] * (omp_get_team_num() + 1) + omp_get_thread_num();
    }
}

__attribute__((noinline,noipa))
void test_vector_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 4: vector partitioned - SIMD with vector-private */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        int vector_local = i & 0xF;  /* Vector-partitioned */
        dst[i] = src[i] ^ vector_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 5: gang+vector partitioned - teams distribute simd */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Partitioned across gangs and vector lanes */
        dst[i] = src[i] + (omp_get_team_num() * 16) + (i % 4);
    }
}

__attribute__((noinline,noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 6: worker+vector partitioned - parallel for simd */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Partitioned across workers and vector lanes */
        int worker_id = omp_get_thread_num();
        int lane_id = i % 4;
        dst[i] = src[i] * worker_id + lane_id;
    }
}

__attribute__((noinline,noipa))
void test_fully_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 7: fully partitioned - complex nested construct */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(2) \
                private(src)  /* Force full partitioning analysis */
    for (int i = 0; i < N; i++) {
        /* Fully partitioned across gangs, workers, and vectors */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int lane_id = i % 2;
        dst[i] = src[i] + (gang_id * 1000) + (worker_id * 100) + lane_id;
    }
}

__attribute__((noinline,noipa))
void test_mixed_clauses(int *src, int *dst) {
    int N = global_N;
    int factor = seed;
    
    /* Mixed clauses to trigger various partitioning analyses */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                firstprivate(factor) \
                num_teams(3) num_threads(5) simdlen(4)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * factor + (i % 256);
    }
}

__attribute__((noinline,noipa))
void test_nowait_depend(int *src, int *dst, int *tmp) {
    int N = global_N;
    
    /* Use nowait and depend to create complex runtime scheduling */
    #pragma omp target teams distribute nowait \
                depend(out: tmp[0]) \
                map(tofrom: tmp[0:N]) map(to: src[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        tmp[i] = src[i] * 3;
    }
    
    #pragma omp target teams distribute parallel for nowait \
                depend(in: tmp[0]) \
                map(tofrom: dst[0:N]) map(tofrom: tmp[0:N]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        dst[i] = tmp[i] + i;
    }
    
    #pragma omp taskwait
}

int main() {
    int N = global_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (i * 17 + seed) % 1000;
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
    
    /* Final reduction to compute checksum and prevent optimization */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify some results */
    int host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    
    if (checksum == host_checksum) {
        printf("Checksum verification passed\n");
    } else {
        printf("Checksum mismatch: device=%d, host=%d\n", checksum, host_checksum);
    }
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
