#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent constant folding and ensure runtime execution */
volatile int volatile_N = N;
volatile int seed_factor = 1;

/* Non-inlined functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    int factor = 3;
    int offset = seed_factor * 2;
    
    #pragma omp target teams map(to: src[0:N]) map(from: dst[0:N]) \
                             num_teams(4) thread_limit(64)
    {
        int private_var = factor + offset; /* Gang redundant */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + private_var;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    int factor = seed_factor;
    
    #pragma omp target teams map(to: src[0:N]) map(from: dst[0:N]) \
                             num_teams(8) thread_limit(32)
    {
        /* Array partitioned across gangs */
        int gang_part[128];
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            int gang_id = omp_get_team_num();
            gang_part[gang_id % 128] = factor * gang_id;
            dst[i] = src[i] + gang_part[gang_id % 128];
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    int factor = seed_factor + 1;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        /* Worker-partitioned variable */
        int worker_private = factor * omp_get_thread_num();
        dst[i] = src[i] * worker_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    int base = seed_factor * 10;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        /* Both gang and worker partitioned */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + base + (gang_id * 100) + worker_id;
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    int factor = seed_factor * 3;
    
    #pragma omp target teams distribute simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* Vector partitioned computation */
        int lane = i % 8; /* Simulated lane ID */
        dst[i] = src[i] * factor + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    int base = seed_factor * 5;
    
    #pragma omp target teams distribute simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Gang + vector partitioned */
        int gang_id = omp_get_team_num();
        int lane = i % 4;
        dst[i] = src[i] + base + (gang_id * 1000) + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    int factor = seed_factor * 7;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Worker + vector partitioned */
        int worker_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * factor + (worker_id * 100) + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    int base = seed_factor * 11;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:N]) map(from: dst[0:N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        /* Fully partitioned across gang, worker, and vector */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int lane = i % 2;
        dst[i] = src[i] + base + (gang_id * 10000) + (worker_id * 100) + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_clauses(int *src, int *dst) {
    /* Test with various data clauses to trigger different partitioning */
    int private_scalar = seed_factor;
    int firstprivate_var = private_scalar * 2;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                firstprivate(firstprivate_var) private(private_scalar) \
                num_teams(3) num_threads(6)
    for (int i = 0; i < N; i++) {
        private_scalar = omp_get_thread_num();
        dst[i] = src[i] + firstprivate_var + private_scalar;
    }
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int actual_N = volatile_N;
    int size = actual_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(size * sizeof(int));
    int *dst = (int*)malloc(size * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < size; i++) {
        src[i] = i + seed_factor;
        dst[i] = 0;
    }
    
    /* Execute all test patterns to trigger different partitioning types */
    test_gang_redundant(src, dst);
    
    /* Add dependency between tests */
    #pragma omp taskwait
    
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_clauses(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(to: dst[0:size]) map(tofrom: checksum) \
                num_teams(2)
    for (int i = 0; i < size; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    
    return 0;
}
