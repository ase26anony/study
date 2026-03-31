/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the switch statement in omp-oacc-neuter-broadcast.cc that maps
 * integer codes to human-readable partitioning type strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int dynamic_size = ARRAY_SIZE;

/* Non-inline functions to ensure separate compilation units in the call graph */

__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    /* Case 0: gang redundant - scalar variable replicated across all gangs */
    int factor = 2;  /* Will be firstprivate in teams region */
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                             firstprivate(factor) num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - array partitioned across gangs */
    int offset = 5;
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                             firstprivate(offset) num_teams(8)
    {
        /* Each team (gang) works on its partition of the array */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] + offset + omp_get_team_num();
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within each gang, workers get partitions */
    int worker_factor = 3;
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(worker_factor) num_teams(4) num_threads(32)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * worker_factor + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    int base = 10;
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(base) num_teams(4)
    for (int i = 0; i < dynamic_size; i++) {
        /* Combined gang and worker partitioning */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + base + gang_id * 100 + worker_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vector lanes get partitions */
    int vec_scale = 7;
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(vec_scale) num_teams(2) simdlen(8)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * vec_scale;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned - gangs with SIMD vectorization */
    int gv_factor = 4;
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(gv_factor) num_teams(8) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * gv_factor + omp_get_team_num();
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned - workers with SIMD vectorization */
    int wv_factor = 6;
    #pragma omp target parallel for simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(wv_factor) num_threads(16) simdlen(8)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * wv_factor + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned - gang, worker, and vector levels */
    int full_factor = 9;
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(full_factor) num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * full_factor + 
                 omp_get_team_num() * 1000 + 
                 omp_get_thread_num() * 100;
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mixed test with multiple clauses to trigger various runtime paths */
    int private_var = 42;
    int shared_arr[ARRAY_SIZE];
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size], shared_arr[0:dynamic_size]) \
                map(to: src[0:dynamic_size]) \
                private(private_var) num_teams(4) num_threads(16)
    for (int i = 0; i < dynamic_size; i++) {
        private_var = omp_get_team_num() * 100 + omp_get_thread_num();
        dst[i] = src[i] + private_var;
        shared_arr[i] = private_var;
    }
    
    /* Nowait clause to vary runtime behavior */
    int nowait_factor = 3;
    #pragma omp target teams nowait \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(nowait_factor) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] += src[i] * nowait_factor;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    int N = dynamic_size;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + (rand() % 10);  /* Prevent compile-time optimization */
        dst[i] = 0;
    }
    
    printf("Testing various OpenMP offload partitioning patterns...\n");
    
    /* Exercise all partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_partitioning(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) num_teams(4)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
