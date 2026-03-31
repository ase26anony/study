/* test_omp_partitioning.c - Exercise GCC OpenMP data partitioning types */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent optimization */
volatile int N = ARRAY_SIZE;
volatile int seed = 42;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline,noipa))
void test_gang_redundant(int *src, int *dst) {
    /* Case 0: gang redundant - scalar replicated across gangs */
    volatile int factor = 2;
    #pragma omp target teams map(tofrom:dst[0:N]) map(to:src[0:N]) \
                             num_teams(4) thread_limit(64)
    {
        int local_factor = factor;  /* firstprivate by default in teams */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline,noipa))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    volatile int offset = 10;
    #pragma omp target teams distribute map(tofrom:dst[0:N]) map(to:src[0:N]) \
                                         num_teams(4)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + offset;
    }
}

__attribute__((noinline,noipa))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within parallel region */
    volatile int scale = 3;
    #pragma omp target teams distribute parallel for map(tofrom:dst[0:N]) \
                                                    map(to:src[0:N]) \
                                                    num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_private = scale;  /* private to each worker */
        dst[i] = src[i] * worker_private + omp_get_thread_num();
    }
}

__attribute__((noinline,noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    volatile int base = 5;
    #pragma omp target teams distribute parallel for map(tofrom:dst[0:N]) \
                                                    map(to:src[0:N]) \
                                                    num_teams(3) num_threads(4)
    for (int i = 0; i < N; i++) {
        /* Shared across workers within a gang, partitioned across gangs */
        dst[i] = src[i] * base + omp_get_team_num() * 100 + omp_get_thread_num();
    }
}

__attribute__((noinline,noipa))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vectorization */
    volatile int vec_factor = 4;
    #pragma omp target teams distribute simd map(tofrom:dst[0:N]) \
                                             map(to:src[0:N]) \
                                             num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* Vector-private computation */
        dst[i] = src[i] * vec_factor + (i % 8);
    }
}

__attribute__((noinline,noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned - teams with SIMD */
    volatile int gv_factor = 6;
    #pragma omp target teams distribute simd map(tofrom:dst[0:N]) \
                                             map(to:src[0:N]) \
                                             num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Partitioned across gangs and vector lanes */
        dst[i] = src[i] * gv_factor + omp_get_team_num() * 10 + (i % 4);
    }
}

__attribute__((noinline,noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    volatile int wv_factor = 7;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom:dst[0:N]) map(to:src[0:N]) \
        num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Partitioned across workers and vector lanes */
        int lane_id = i % 4;  /* Simulated vector lane */
        dst[i] = src[i] * wv_factor + omp_get_thread_num() * 100 + lane_id;
    }
}

__attribute__((noinline,noipa))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned - three-level nesting */
    volatile int full_factor = 8;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom:dst[0:N]) map(to:src[0:N]) \
        num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        /* Fully partitioned across gangs, workers, and vectors */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int vector_id = i % 2;
        dst[i] = src[i] * full_factor + gang_id * 1000 + worker_id * 100 + vector_id;
    }
}

__attribute__((noinline,noipa))
void test_mixed_clauses(int *src, int *dst) {
    /* Additional test with mixed data clauses */
    volatile int private_var = 20;
    volatile int firstprivate_var = 30;
    
    #pragma omp target teams distribute parallel for \
        map(tofrom:dst[0:N]) map(to:src[0:N]) \
        firstprivate(firstprivate_var) private(private_var) \
        num_teams(3) num_threads(6)
    for (int i = 0; i < N; i++) {
        private_var = omp_get_thread_num();  /* Private to each thread */
        dst[i] = src[i] + firstprivate_var + private_var;
    }
}

__attribute__((noinline,noipa))
void test_nowait_depend(int *src, int *dst, int *tmp) {
    /* Test with nowait and depend clauses for async execution */
    #pragma omp target teams distribute nowait \
        map(to:src[0:N]) map(from:tmp[0:N]) \
        depend(out:tmp) num_teams(2)
    for (int i = 0; i < N; i++) {
        tmp[i] = src[i] * 2;
    }
    
    #pragma omp target teams distribute \
        map(to:tmp[0:N]) map(from:dst[0:N]) \
        depend(in:tmp) num_teams(2)
    for (int i = 0; i < N; i++) {
        dst[i] = tmp[i] + 1;
    }
    
    #pragma omp taskwait
}

int main() {
    /* Dynamic size to prevent constant folding */
    int size = N;
    int *src = (int*)malloc(size * sizeof(int));
    int *dst = (int*)malloc(size * sizeof(int));
    int *tmp = (int*)malloc(size * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    srand(seed);
    for (int i = 0; i < size; i++) {
        src[i] = rand() % 100;
        dst[i] = 0;
        tmp[i] = 0;
    }
    
    printf("Starting OpenMP partitioning tests...\n");
    
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
    
    /* Final reduction to compute checksum and prevent elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
        map(tofrom:checksum) map(to:dst[0:size]) num_teams(2)
    for (int i = 0; i < size; i++) {
        checksum += dst[i];
    }
    
    /* Also compute on host to ensure data consistency */
    int host_checksum = 0;
    for (int i = 0; i < size; i++) {
        host_checksum += dst[i];
    }
    
    printf("Device checksum: %d\n", checksum);
    printf("Host checksum: %d\n", host_checksum);
    printf("Difference: %d\n", checksum - host_checksum);
    
    /* Additional verification */
    int errors = 0;
    #pragma omp target teams distribute parallel for reduction(+:errors) \
        map(tofrom:errors) map(to:dst[0:size]) num_teams(1)
    for (int i = 0; i < size; i++) {
        if (dst[i] < 0) errors++;
    }
    
    printf("Negative values in dst: %d\n", errors);
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
