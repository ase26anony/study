#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and ensure runtime execution */
volatile int dynamic_size = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    int offset = rand() % 10; /* Prevent optimization */
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                              firstprivate(factor, offset) num_teams(4)
    {
        /* factor and offset are gang-redundant (case 0) */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int local_data[SIZE];
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size], local_data[0:dynamic_size]) \
                              map(to: src[0:dynamic_size]) num_teams(4)
    {
        /* local_data is gang-partitioned (case 1) */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            local_data[i] = src[i];
            dst[i] = local_data[i] * 3;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        int worker_local = i * 2;  /* Worker-partitioned (case 2) */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int shared_var = 5;
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                              firstprivate(shared_var) num_teams(2)
    {
        /* Two-level partitioning: gang+worker (case 3) */
        #pragma omp distribute parallel for num_threads(4)
        for (int i = 0; i < dynamic_size; i++) {
            int thread_local = omp_get_thread_num();
            dst[i] = src[i] * shared_var + thread_local;
        }
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < dynamic_size; i++) {
        int vector_private = i % 16;  /* Vector-partitioned (case 4) */
        dst[i] = src[i] + vector_private;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                simdlen(8) num_teams(4)
    for (int i = 0; i < dynamic_size; i++) {
        int gang_vector_local = (i + omp_get_team_num()) % 32;  /* Gang+vector partitioned (case 5) */
        dst[i] = src[i] * gang_vector_local;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                simdlen(4) num_teams(1) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        int worker_vector_local = (i + omp_get_thread_num()) % 16;  /* Worker+vector partitioned (case 6) */
        dst[i] = src[i] + worker_vector_local;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                simdlen(4) num_teams(4) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        /* Fully partitioned across gang, worker, and vector (case 7) */
        int fully_partitioned = (i + omp_get_team_num() * 100 + omp_get_thread_num() * 10) % 64;
        dst[i] = src[i] * fully_partitioned;
    }
}

/* Test with depend clause for additional runtime paths */
__attribute__((noinline))
void test_with_depend(int *src, int *dst, int *tmp) {
    #pragma omp target teams map(tofrom: tmp[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                              nowait depend(out: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            tmp[i] = src[i] * 2;
        }
    }
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) map(to: tmp[0:dynamic_size]) \
                              depend(in: tmp) num_teams(2)
    {
        #pragma omp distribute parallel for num_threads(2)
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = tmp[i] + i;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = dynamic_size;
    
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
    
    /* Execute all test patterns to trigger different partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_with_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams map(tofrom: checksum) map(to: dst[0:N]) \
                              reduction(+:checksum) num_teams(2)
    {
        #pragma omp distribute parallel for reduction(+:checksum) num_threads(2)
        for (int i = 0; i < N; i++) {
            checksum += dst[i];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
