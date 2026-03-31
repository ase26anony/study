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
    int offset = rand() % 10;  /* Make runtime-dependent */
    
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(factor, offset) num_teams(4)
    {
        /* Case 0: gang redundant - factor is replicated across all gangs */
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - arrays are partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] + i;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within each gang, partitioned across workers */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        int worker_local = omp_get_thread_num();  /* Worker-specific */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(4)
    for (int i = 0; i < global_N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * gang_id + worker_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD lanes get different partitions */
    int factor = rand() % 5 + 1;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(factor) num_teams(2) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        /* Each SIMD lane gets its own i value */
        dst[i] = src[i] * factor + (i % 8);  /* i%8 simulates lane id */
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned */
    int base = rand() % 100;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(base) num_teams(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        int gang_id = omp_get_team_num();
        dst[i] = src[i] + base + gang_id * 10 + (i % 4);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * worker_id + (i % 4);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                num_teams(4) num_threads(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + gang_id * 100 + worker_id * 10 + (i % 4);
    }
}

/* Additional test with private clauses */
__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    /* Mix different data clauses to trigger various partitioning */
    int shared_var = 42;
    int private_var = rand() % 20;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(shared_var) private(private_var) \
                num_teams(4) num_threads(4)
    for (int i = 0; i < global_N; i++) {
        private_var = omp_get_thread_num();
        dst[i] = src[i] + shared_var + private_var;
    }
}

/* Test with nowait to vary runtime behavior */
__attribute__((noinline))
void test_async_partitioning(int *src, int *dst, int *dst2) {
    /* Use depend clauses to create dependencies */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                depend(out: dst[0:global_N]) nowait \
                num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        dst[i] = src[i] * 2;
    }
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst2[0:global_N]) map(to: dst[0:global_N]) \
                depend(in: dst[0:global_N]) nowait \
                num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        dst2[i] = dst[i] + 1;
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = global_N;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *dst2 = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !dst2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i;
        dst[i] = 0;
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
    test_mixed_clauses(src, dst);
    test_async_partitioning(src, dst, dst2);
    
    /* Final checksum computation using reduction */
    int final_sum = 0;
    #pragma omp target teams distribute parallel for reduction(+:final_sum) \
                map(tofrom: final_sum) map(to: dst[0:N]) \
                num_teams(4)
    for (int i = 0; i < N; i++) {
        final_sum += dst[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    
    /* Additional host-side computation to ensure all code paths are used */
    int host_sum = 0;
    #pragma omp parallel for reduction(+:host_sum)
    for (int i = 0; i < N; i++) {
        host_sum += dst2[i];
    }
    printf("Host-side checksum: %d\n", host_sum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(dst2);
    
    return 0;
}
