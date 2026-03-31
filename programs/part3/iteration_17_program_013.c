#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent optimization and ensure runtime execution */
volatile int global_N = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;
    int offset = rand() % 10;  /* Prevent constant folding */
    
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                              num_teams(4) thread_limit(32)
    {
        int private_var = factor;  /* Gang redundant - replicated across all gangs */
        
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * private_var + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int factor = 3;
    
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                              num_teams(8) thread_limit(64)
    {
        /* Gang partitioned array - each gang gets a portion */
        int gang_array[128];
        
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            gang_array[i % 128] = src[i];
            dst[i] = gang_array[i % 128] * factor;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int factor = 4;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        /* Worker partitioned - each worker thread gets its own instance */
        int worker_local = factor + omp_get_thread_num();
        dst[i] = src[i] * worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int factor = 5;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < global_N; i++) {
        /* Shared within team, partitioned across teams */
        int team_shared = factor * omp_get_team_num();
        dst[i] = src[i] + team_shared + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int factor = 6;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        /* Vector partitioned - each SIMD lane gets its own instance */
        int vector_private = factor + (i % 8);
        dst[i] = src[i] * vector_private;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int factor = 7;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        /* Both gang and vector partitioned */
        int gang_part = factor * omp_get_team_num();
        int vec_part = (i % 4);
        dst[i] = src[i] + gang_part + vec_part;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int factor = 8;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < global_N; i++) {
        /* Both worker and vector partitioned */
        int worker_part = factor + omp_get_thread_num();
        int vec_part = (i % 2);
        dst[i] = src[i] * worker_part + vec_part;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int factor = 9;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        /* Fully partitioned across gang, worker, and vector */
        int gang_part = omp_get_team_num();
        int worker_part = omp_get_thread_num();
        int vec_part = (i % 4);
        dst[i] = src[i] + gang_part * 100 + worker_part * 10 + vec_part;
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    /* Test with various data clauses to trigger different partitioning */
    int private_scalar = rand() % 100;
    int firstprivate_var = private_scalar;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(firstprivate_var) private(private_scalar) \
                num_teams(3) num_threads(5)
    for (int i = 0; i < global_N; i++) {
        int local_private = private_scalar + omp_get_thread_num();
        dst[i] = src[i] * firstprivate_var + local_private;
    }
}

__attribute__((noinline))
void test_nowait_regions(int *src, int *dst) {
    /* Test with nowait to create multiple concurrent regions */
    int factor1 = 10;
    int factor2 = 11;
    
    #pragma omp target teams map(tofrom: dst[0:global_N/2]) map(to: src[0:global_N/2]) \
                              num_teams(2) nowait
    {
        #pragma omp distribute
        for (int i = 0; i < global_N/2; i++) {
            dst[i] = src[i] * factor1;
        }
    }
    
    #pragma omp target teams map(tofrom: dst[global_N/2:global_N/2]) \
                              map(to: src[global_N/2:global_N/2]) \
                              num_teams(2) nowait
    {
        #pragma omp distribute
        for (int i = global_N/2; i < global_N; i++) {
            dst[i] = src[i] * factor2;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int N = global_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    srand(42);
    for (int i = 0; i < N; i++) {
        src[i] = i + (rand() % 10);
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
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
