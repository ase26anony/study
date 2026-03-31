#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent optimization and constant folding */
volatile int g_volatile_size = SIZE;
volatile int g_seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,noipa))
void test_gang_redundant(int *src, int *dst) {
    int factor = g_seed % 10 + 1;  /* Runtime-dependent */
    int offset = g_seed % 5;
    
    #pragma omp target teams map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                             num_teams(4) thread_limit(32)
    {
        int local_factor = factor;  /* gang redundant - case 0 */
        int local_offset = offset;  /* gang redundant - case 0 */
        
        #pragma omp distribute
        for (int i = 0; i < g_volatile_size; i++) {
            dst[i] = src[i] * local_factor + local_offset;
        }
    }
}

__attribute__((noinline,noipa))
void test_gang_partitioned(int *src, int *dst) {
    int factor = g_seed % 7 + 2;
    
    /* gang partitioned - case 1 */
    #pragma omp target teams map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                             num_teams(8)
    {
        #pragma omp distribute
        for (int i = 0; i < g_volatile_size; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline,noipa))
void test_worker_partitioned(int *src, int *dst) {
    /* worker partitioned - case 2 */
    #pragma omp target teams distribute parallel for \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                num_teams(2) num_threads(16)
    for (int i = 0; i < g_volatile_size; i++) {
        int worker_local = i % 8;  /* worker partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* gang+worker partitioned - case 3 */
    #pragma omp target teams distribute parallel for \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                num_teams(4) num_threads(8)
    for (int i = 0; i < g_volatile_size; i++) {
        int gang_worker_local = (i + omp_get_team_num()) % 16;
        dst[i] = src[i] * gang_worker_local;
    }
}

__attribute__((noinline,noipa))
void test_vector_partitioned(int *src, int *dst) {
    /* vector partitioned - case 4 */
    #pragma omp target teams distribute simd \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                num_teams(2) simdlen(8)
    for (int i = 0; i < g_volatile_size; i++) {
        int vector_local = i % 4;  /* vector partitioned */
        dst[i] = src[i] - vector_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* gang+vector partitioned - case 5 */
    #pragma omp target teams distribute simd \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                num_teams(4) simdlen(16)
    for (int i = 0; i < g_volatile_size; i++) {
        int gang_vector_local = (i * omp_get_team_num()) % 32;
        dst[i] = src[i] + gang_vector_local;
    }
}

__attribute__((noinline,noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* worker+vector partitioned - case 6 */
    #pragma omp target teams distribute parallel for simd \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                num_teams(2) num_threads(4) simdlen(8)
    for (int i = 0; i < g_volatile_size; i++) {
        int worker_vector_local = (i + omp_get_thread_num()) % 64;
        dst[i] = src[i] * worker_vector_local;
    }
}

__attribute__((noinline,noipa))
void test_fully_partitioned(int *src, int *dst) {
    /* fully partitioned - case 7 */
    #pragma omp target teams distribute parallel for simd \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                num_teams(8) num_threads(4) simdlen(4)
    for (int i = 0; i < g_volatile_size; i++) {
        int fully_local = (i * omp_get_team_num() + omp_get_thread_num()) % 128;
        dst[i] = src[i] + fully_local;
    }
}

__attribute__((noinline,noipa))
void test_mixed_clauses(int *src, int *dst) {
    /* Mixed clauses to trigger various partitioning combinations */
    int factor = g_seed % 9 + 1;
    int offset = g_seed % 13;
    
    #pragma omp target teams distribute parallel for simd \
                map(to:src[0:SIZE]) map(from:dst[0:SIZE]) \
                firstprivate(factor) private(offset) \
                num_teams(4) num_threads(8) simdlen(8)
    for (int i = 0; i < g_volatile_size; i++) {
        int thread_local = omp_get_thread_num();
        int simd_local = i % 8;
        dst[i] = src[i] * factor + offset + thread_local + simd_local;
    }
}

__attribute__((noinline,noipa))
void test_nowait_regions(int *src, int *dst) {
    /* Use nowait to create multiple concurrent offload regions */
    int factor1 = g_seed % 5 + 1;
    int factor2 = g_seed % 7 + 2;
    
    #pragma omp target teams map(to:src[0:SIZE/2]) map(from:dst[0:SIZE/2]) \
                             nowait num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < g_volatile_size/2; i++) {
            dst[i] = src[i] * factor1;
        }
    }
    
    #pragma omp target teams map(to:src[SIZE/2:SIZE/2]) map(from:dst[SIZE/2:SIZE/2]) \
                             nowait num_teams(2)
    {
        #pragma omp distribute
        for (int i = SIZE/2; i < g_volatile_size; i++) {
            dst[i] = src[i] * factor2;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    int *src = (int*)malloc(SIZE * sizeof(int));
    int *dst = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = (i * 13 + 7) % 97;  /* Non-trivial pattern */
    }
    
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
    
    /* Final reduction to compute checksum and prevent optimization */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(to:dst[0:SIZE]) num_teams(4) num_threads(8)
    for (int i = 0; i < g_volatile_size; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
