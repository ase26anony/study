#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int global_N = ARRAY_SIZE;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int N = global_N;
    int factor = 2;  /* Will be firstprivate */
    int offset = 10; /* Will be private */
    
    /* Case 0: gang redundant - firstprivate scalar replicated across gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             firstprivate(factor) private(offset) num_teams(4)
    {
        offset = omp_get_team_num() * 100; /* Different per team */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 1: gang partitioned - array mapped across gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + omp_get_team_num();
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 2: worker partitioned - variable in parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * worker_id;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 3: gang+worker partitioned - shared array with two-level parallelism */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + gang_id * 100 + worker_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 4: vector partitioned - SIMD with vector-private variables */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* Vector operations - each SIMD lane gets its own instance */
        dst[i] = src[i] * (i % 8); /* i%8 varies per SIMD lane */
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        dst[i] = src[i] + gang_id * (i % 4); /* Combined gang and vector */
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    #pragma omp target parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * worker_id + (i % 4); /* Combined worker and vector */
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = global_N;
    
    /* Case 7: fully partitioned - teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int vector_id = i % 2; /* SIMD lane */
        dst[i] = src[i] + gang_id * 1000 + worker_id * 100 + vector_id;
    }
}

/* Additional test with depend clause to vary runtime behavior */
__attribute__((noinline))
void test_with_depend(int *src, int *dst, int *tmp) {
    int N = global_N;
    
    /* First task with output dependence */
    #pragma omp target teams map(tofrom: tmp[0:N]) map(to: src[0:N]) \
                             nowait depend(out: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            tmp[i] = src[i] * 2;
        }
    }
    
    /* Second task with input dependence */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: tmp[0:N]) \
                             nowait depend(in: tmp) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = tmp[i] + 1;
        }
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
    test_with_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams map(tofrom: checksum) map(to: dst[0:N]) \
                             reduction(+:checksum) num_teams(2)
    {
        #pragma omp distribute parallel for reduction(+:checksum)
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
