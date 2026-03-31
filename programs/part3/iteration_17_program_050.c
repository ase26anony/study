#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent constant folding and ensure runtime execution */
volatile int vol_N = N;
volatile int vol_factor = 2;
volatile int vol_offset = 1;

/* Non-inlined functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = vol_factor;
    int offset = vol_offset;
    
    #pragma omp target teams map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                             num_teams(4) thread_limit(64)
    {
        int private_var = factor;  /* gang redundant */
        #pragma omp distribute
        for (int i = 0; i < vol_N; i++) {
            dst[i] = src[i] * private_var + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                             num_teams(4) thread_limit(64)
    {
        /* dst is gang partitioned */
        #pragma omp distribute
        for (int i = 0; i < vol_N; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < vol_N; i++) {
        /* Each worker gets its own partition */
        int worker_val = factor + omp_get_thread_num();
        dst[i] = src[i] * worker_val;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < vol_N; i++) {
        /* Two-level partitioning: gang and worker */
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * factor + team_id * 100 + thread_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < vol_N; i++) {
        /* Vector partitioning */
        dst[i] = src[i] * factor + (i % 8);
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(4) simdlen(8)
    for (int i = 0; i < vol_N; i++) {
        /* Combined gang and vector partitioning */
        int team_id = omp_get_team_num();
        dst[i] = src[i] * factor + team_id * 10 + (i % 8);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < vol_N; i++) {
        /* Combined worker and vector partitioning */
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * factor + thread_id * 5 + (i % 4);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int factor = vol_factor;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(4) num_threads(16) simdlen(8)
    for (int i = 0; i < vol_N; i++) {
        /* Fully partitioned across gang, worker, and vector */
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        int lane = i % 8;
        dst[i] = src[i] * factor + team_id * 1000 + thread_id * 10 + lane;
    }
}

__attribute__((noinline))
void test_mixed_clauses(int *src, int *dst) {
    int factor = vol_factor;
    int shared_var = 42;
    int private_var = 7;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                map(to: shared_var) firstprivate(private_var) \
                num_teams(3) num_threads(12)
    for (int i = 0; i < vol_N; i++) {
        /* Mix of shared, firstprivate, and mapped variables */
        dst[i] = src[i] * factor + shared_var + private_var + 
                 omp_get_team_num() + omp_get_thread_num();
    }
}

int main() {
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + (rand() % 10);
        dst[i] = 0;
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
    test_mixed_clauses(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:vol_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < vol_N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
