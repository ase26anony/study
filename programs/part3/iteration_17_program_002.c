#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int global_N = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    int offset = rand() % 100;  /* Make runtime-dependent */
    
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(factor, offset) num_teams(4) thread_limit(128)
    {
        /* factor and offset are gang-redundant (case 0) */
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Array partitioned across gangs (case 1) */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             num_teams(8)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] + i;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Worker-partitioned computation (case 2) */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        int worker_local = i * 2;  /* Worker-partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Two-level partitioning: gang + worker (case 3) */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < global_N; i++) {
        int gang_worker_local = omp_get_team_num() * 100 + omp_get_thread_num();
        dst[i] = src[i] + gang_worker_local;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Vector-partitioned computation (case 4) */
    int factor = 3;
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(factor) num_teams(2) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        /* Vector operations - each SIMD lane gets its own i */
        dst[i] = src[i] * factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Gang + vector partitioning (case 5) */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) simdlen(16)
    for (int i = 0; i < global_N; i++) {
        int gang_vector_val = omp_get_team_num() * 10 + (i % 16);
        dst[i] = src[i] + gang_vector_val;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Worker + vector partitioning (case 6) */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        int worker_vector_val = omp_get_thread_num() * 100 + (i % 4);
        dst[i] = src[i] + worker_vector_val;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Fully partitioned: gang + worker + vector (case 7) */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(8) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        int fully_partitioned = omp_get_team_num() * 1000 + 
                               omp_get_thread_num() * 100 + 
                               (i % 8);
        dst[i] = src[i] + fully_partitioned;
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    /* Test with various data clauses to trigger different partitioning */
    int private_var = rand() % 100;
    int firstprivate_var = rand() % 100;
    int shared_array[10];
    
    for (int k = 0; k < 10; k++) shared_array[k] = k;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N], shared_array) map(to: src[0:global_N]) \
                firstprivate(firstprivate_var) private(private_var) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < global_N; i++) {
        private_var = i % 10;
        dst[i] = src[i] + firstprivate_var + private_var + shared_array[i % 10];
    }
}

__attribute__((noinline))
void test_nowait_depend(int *src, int *dst, int *tmp) {
    /* Test with nowait and depend clauses */
    #pragma omp target teams map(to: src[0:global_N]) map(from: tmp[0:global_N]) \
                             nowait depend(out: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            tmp[i] = src[i] * 2;
        }
    }
    
    #pragma omp target teams map(to: tmp[0:global_N]) map(from: dst[0:global_N]) \
                             nowait depend(in: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = tmp[i] + 1;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = global_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i;
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
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) num_teams(4)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
