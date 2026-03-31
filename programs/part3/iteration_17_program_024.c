#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int dynamic_N = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,noipa))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    int offset = 10; /* Will be gang-redundant */
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                             num_teams(4) thread_limit(32)
    {
        /* Case 0: gang redundant - firstprivate scalar */
        int local_factor = factor;  /* Implicit firstprivate */
        int local_offset = offset; /* Implicit firstprivate */
        
        #pragma omp distribute
        for (int i = 0; i < dynamic_N; i++) {
            dst[i] = src[i] * local_factor + local_offset;
        }
    }
}

__attribute__((noinline,noipa))
void test_gang_partitioned(int *src, int *dst) {
    int factor = 3;
    
    /* Case 1: gang partitioned - mapped array partitioned across gangs */
    #pragma omp target teams distribute map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                                        num_teams(8)
    for (int i = 0; i < dynamic_N; i++) {
        dst[i] = src[i] * factor;
    }
}

__attribute__((noinline,noipa))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - variable in parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < dynamic_N; i++) {
        int worker_local = i % 100;  /* Worker-partitioned variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - shared array with nested parallelism */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) num_threads(4)
    for (int i = 0; i < dynamic_N; i++) {
        /* Both gang and worker partitioning occur here */
        dst[i] = src[i] * (i % 16);
    }
}

__attribute__((noinline,noipa))
void test_vector_partitioned(int *src, int *dst) {
    int factor = 5;
    
    /* Case 4: vector partitioned - SIMD with vector-private variable */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < dynamic_N; i++) {
        int vector_private = factor * (i & 3);  /* Vector-partitioned */
        dst[i] = src[i] + vector_private;
    }
}

__attribute__((noinline,noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned - teams distribute simd */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        dst[i] = src[i] * 2 + (i % 8);
    }
}

__attribute__((noinline,noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned - parallel for simd */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        int temp = (i % 16) * 3;
        dst[i] = src[i] + temp;
    }
}

__attribute__((noinline,noipa))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned - nested teams+parallel+simd */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < dynamic_N; i++) {
        /* Fully partitioned across gang, worker, and vector */
        int gang_part = i / (dynamic_N / 4);
        int worker_part = (i / 32) % 8;
        int vector_part = i % 2;
        dst[i] = src[i] + gang_part + worker_part + vector_part;
    }
}

__attribute__((noinline,noipa))
void test_mixed_clauses(int *src, int *dst) {
    /* Mixed data clauses to trigger various partitioning */
    int scalar = 7;
    int array[16];
    
    /* Initialize local array */
    for (int i = 0; i < 16; i++) {
        array[i] = i;
    }
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                map(to: array[0:16]) firstprivate(scalar) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < dynamic_N; i++) {
        int idx = i % 16;
        dst[i] = src[i] * scalar + array[idx];
    }
}

__attribute__((noinline,noipa))
void test_nowait_depend(int *src, int *dst, int *tmp) {
    /* Use nowait and depend clauses to vary runtime behavior */
    #pragma omp target teams distribute \
                map(tofrom: tmp[0:dynamic_N]) map(to: src[0:dynamic_N]) \
                depend(out: tmp) nowait \
                num_teams(2)
    for (int i = 0; i < dynamic_N; i++) {
        tmp[i] = src[i] * 2;
    }
    
    #pragma omp taskwait
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_N]) map(to: tmp[0:dynamic_N]) \
                depend(in: tmp) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < dynamic_N; i++) {
        dst[i] = tmp[i] + i;
    }
}

int main() {
    int N = dynamic_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = rand() % 100;  /* Use rand() to prevent constant folding */
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
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional verification */
    int host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    printf("Host verification checksum: %d\n", host_checksum);
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
