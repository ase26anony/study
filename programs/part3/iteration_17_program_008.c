#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent optimization and ensure runtime execution */
volatile int dynamic_size = SIZE;

/* Test functions with noinline to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    int offset = 1;
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) \
                             map(to: src[0:dynamic_size]) \
                             num_teams(4) thread_limit(128)
    {
        /* factor is firstprivate by default in teams region */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int local_data[256];  /* Will be gang-partitioned */
    
    /* Initialize local data */
    for (int i = 0; i < 256; i++) {
        local_data[i] = i;
    }
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) \
                             map(to: src[0:dynamic_size], local_data[0:256]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            int idx = i % 256;
            dst[i] = src[i] + local_data[idx];
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < dynamic_size; i++) {
        /* Each worker gets its own private computation */
        int worker_local = omp_get_thread_num();
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int shared_var = 100;  /* Will be gang+worker partitioned */
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                firstprivate(shared_var) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < dynamic_size; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * team_id + thread_id + shared_var;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int vector_factor = 3;  /* Will be vector partitioned */
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                simdlen(8) num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * vector_factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        int team_id = omp_get_team_num();
        dst[i] = src[i] * team_id + i;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Use nested parallelism to trigger worker+vector partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            dst[i] += src[i] * j;
        }
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Complex nested construct to trigger full partitioning */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                private(src, dst) \
                num_teams(8) num_threads(16) simdlen(8)
    for (int i = 0; i < dynamic_size; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        int lane_id = i % 8;  /* Simulates vector lane */
        dst[i] = src[i] * team_id + thread_id * lane_id;
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mix different data clauses to trigger various partitioning types */
    int gang_private = 10;
    int worker_private = 20;
    int vector_private = 30;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                firstprivate(gang_private) \
                private(worker_private) \
                linear(vector_private:1) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] + gang_private + worker_private + vector_private;
        vector_private++;
    }
}

int main() {
    int *src = (int*)malloc(SIZE * sizeof(int));
    int *dst = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * (rand() % 10 + 1);  /* Prevent constant folding */
    }
    
    printf("Starting OpenMP offload tests...\n");
    
    /* Execute all test patterns */
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
    
    test_mixed_partitioning(src, dst);
    printf("Completed mixed_partitioning test\n");
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:dynamic_size]) \
                reduction(+:checksum) num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
