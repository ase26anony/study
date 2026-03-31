#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent constant folding and dead code elimination */
volatile int global_N = N;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;
    int offset = 1;
    
    #pragma omp target teams map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                             firstprivate(factor, offset) num_teams(4) thread_limit(128)
    {
        int gang_local = factor + offset;  /* gang redundant */
        
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * gang_local;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int factor = 3;
    
    #pragma omp target teams map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                             map(tofrom: factor) num_teams(8)
    {
        /* factor is gang partitioned due to map(tofrom:) */
        
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] + factor;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int factor = 4;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                firstprivate(factor) num_teams(2) num_threads(32)
    for (int i = 0; i < global_N; i++) {
        int worker_local = factor + i % 10;  /* worker partitioned */
        dst[i] = src[i] * worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int shared_factor = 5;
    int private_offset = 2;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                map(tofrom: shared_factor) firstprivate(private_offset) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < global_N; i++) {
        /* shared_factor is gang+worker partitioned */
        dst[i] = src[i] * shared_factor + private_offset + (i % 100);
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int factor = 6;
    
    #pragma omp target teams distribute simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                firstprivate(factor) num_teams(1) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        int vector_local = factor + (i & 0xF);  /* vector partitioned */
        dst[i] = src[i] + vector_local;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int gang_factor = 7;
    int vector_factor = 3;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                map(tofrom: gang_factor) firstprivate(vector_factor) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        /* gang_factor is gang partitioned, vector_factor is vector partitioned */
        dst[i] = src[i] * gang_factor + vector_factor * (i & 0x7);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int worker_factor = 8;
    int vector_factor = 2;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                private(worker_factor) firstprivate(vector_factor) \
                num_teams(2) num_threads(16) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        worker_factor = 8 + (i % 20);  /* worker partitioned */
        /* vector_factor is vector partitioned */
        dst[i] = src[i] + worker_factor * vector_factor;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int gang_var = 9;
    int worker_var = 4;
    int vector_var = 1;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                map(tofrom: gang_var) private(worker_var) firstprivate(vector_var) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        worker_var = 4 + (i % 30);  /* worker partitioned */
        /* gang_var is gang partitioned, vector_var is vector partitioned */
        dst[i] = src[i] * gang_var + worker_var * vector_var;
    }
}

__attribute__((noinline))
void test_mixed_partitioning_with_nowait(int *src, int *dst1, int *dst2) {
    int factor1 = 10, factor2 = 5;
    
    /* First region with nowait */
    #pragma omp target teams map(to: src[0:global_N]) map(from: dst1[0:global_N]) \
                             map(tofrom: factor1) num_teams(2) nowait
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst1[i] = src[i] * factor1;
        }
    }
    
    /* Second region with depend clause */
    #pragma omp target teams distribute parallel for \
                map(to: src[0:global_N]) map(from: dst2[0:global_N]) \
                firstprivate(factor2) depend(inout: dst1) num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        dst2[i] = dst1[i] + factor2 + (i % 50);
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int array_size = global_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(array_size * sizeof(int));
    int *dst = (int*)malloc(array_size * sizeof(int));
    int *dst2 = (int*)malloc(array_size * sizeof(int));
    
    if (!src || !dst || !dst2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = i;
        dst[i] = 0;
        dst2[i] = 0;
    }
    
    printf("Starting OpenMP offload tests...\n");
    
    /* Test all partitioning types */
    test_gang_redundant(src, dst);
    printf("Completed gang_redundant test\n");
    
    test_gang_partitioned(src, dst);
    printf("Completed gang_partitioned test\n");
    
    test_worker_partitioned(src, dst);
    printf("Completed worker_partitioned test\n");
    
    test_gang_worker_partitioned(src, dst);
    printf("Completed gang_worker_partitioned test\n");
    
    test_vector_partitioned(src, dst);
    printf("Completed vector_partitioned test\n");
    
    test_gang_vector_partitioned(src, dst);
    printf("Completed gang_vector_partitioned test\n");
    
    test_worker_vector_partitioned(src, dst);
    printf("Completed worker_vector_partitioned test\n");
    
    test_fully_partitioned(src, dst);
    printf("Completed fully_partitioned test\n");
    
    test_mixed_partitioning_with_nowait(src, dst, dst2);
    printf("Completed mixed_partitioning_with_nowait test\n");
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(to: dst[0:array_size]) map(tofrom: checksum) \
                num_teams(2) num_threads(16)
    for (int i = 0; i < array_size; i++) {
        checksum += dst[i];
    }
    
    /* Also checksum the second destination array */
    int checksum2 = 0;
    #pragma omp target teams distribute simd reduction(+:checksum2) \
                map(to: dst2[0:array_size]) map(tofrom: checksum2) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < array_size; i++) {
        checksum2 += dst2[i];
    }
    
    printf("Final checksums: %d, %d\n", checksum, checksum2);
    printf("Total: %d\n", checksum + checksum2);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(dst2);
    
    return 0;
}
