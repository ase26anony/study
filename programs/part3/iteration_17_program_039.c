#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent optimization and ensure runtime execution */
volatile int global_N = N;
volatile int seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(factor) num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int offset = 3;  /* Will be gang-partitioned through map */
    #pragma omp target teams map(tofrom: dst[0:global_N], offset) map(to: src[0:global_N]) \
                             num_teams(8)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] + offset;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int worker_var = 5;  /* Will be worker-partitioned */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                private(worker_var) num_teams(2) num_threads(4)
    for (int i = 0; i < global_N; i++) {
        worker_var = i % 8;
        dst[i] = src[i] - worker_var;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int shared_var = 7;  /* Will be gang+worker partitioned */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N], shared_var) map(to: src[0:global_N]) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        dst[i] = src[i] * shared_var + (i % 16);
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int vector_var = 11;  /* Will be vector-partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                linear(vector_var:1) num_teams(2) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        vector_var = i;
        dst[i] = src[i] + vector_var;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int factor = 13;  /* Will be gang+vector partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N], factor) map(to: src[0:global_N]) \
                num_teams(4) num_threads(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        dst[i] = src[i] * factor - (i % 32);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Complex nested construct for worker+vector partitioning */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             num_teams(2)
    {
        #pragma omp distribute
        for (int g = 0; g < 2; g++) {
            #pragma omp parallel for simd private(g) simdlen(8)
            for (int i = g * (global_N/2); i < (g+1) * (global_N/2); i++) {
                int worker_vector_var = i % 64;
                dst[i] = src[i] + worker_vector_var * 2;
            }
        }
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Fully partitioned across all dimensions */
    int fully_var = 17;  /* Will be fully partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N], fully_var) map(to: src[0:global_N]) \
                num_teams(8) num_threads(16) simdlen(16) \
                private(fully_var)
    for (int i = 0; i < global_N; i++) {
        fully_var = i % 128;
        dst[i] = src[i] * 3 + fully_var;
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    /* Mix different data clauses to trigger various partitionings */
    int gang_private = 19;
    int worker_private = 23;
    int vector_private = 29;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(gang_private) private(worker_private) \
                linear(vector_private:1) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        worker_private = i % 16;
        vector_private = i % 8;
        dst[i] = src[i] + gang_private * worker_private + vector_private;
    }
}

__attribute__((noinline))
void test_nowait_regions(int *src, int *dst) {
    /* Use nowait to create multiple concurrent regions */
    int tmp1[N], tmp2[N];
    
    #pragma omp target teams map(tofrom: tmp1[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(global_N) nowait
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            tmp1[i] = src[i] * 2;
        }
    }
    
    #pragma omp target teams map(tofrom: tmp2[0:global_N]) map(to: src[0:global_N]) \
                             nowait
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            tmp2[i] = src[i] + 5;
        }
    }
    
    #pragma omp taskwait
    
    /* Combine results */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: tmp1[0:global_N], tmp2[0:global_N])
    for (int i = 0; i < global_N; i++) {
        dst[i] = tmp1[i] + tmp2[i];
    }
}

int main() {
    /* Use volatile to prevent constant folding */
    volatile int array_size = global_N;
    int *src = (int*)malloc(array_size * sizeof(int));
    int *dst = (int*)malloc(array_size * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    srand(seed);
    for (int i = 0; i < array_size; i++) {
        src[i] = rand() % 100;
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
    test_mixed_clauses(src, dst);
    test_nowait_regions(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:array_size])
    for (int i = 0; i < array_size; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
