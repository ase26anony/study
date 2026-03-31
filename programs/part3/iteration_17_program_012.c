#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int global_N = SIZE;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline, noipa))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(factor) num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * factor;
        }
    }
}

__attribute__((noinline, noipa))
void test_gang_partitioned(int *src, int *dst) {
    int offset = 3;  /* Will be gang-partitioned through map */
    #pragma omp target teams map(tofrom: dst[0:global_N], offset) map(to: src[0:global_N]) \
                             num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] + offset;
        }
    }
}

__attribute__((noinline, noipa))
void test_worker_partitioned(int *src, int *dst) {
    volatile int local_N = global_N;
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                                                   num_teams(2) num_threads(8)
    for (int i = 0; i < local_N; i++) {
        int worker_local = i % 10;  /* Worker-partitioned variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline, noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    int shared_var[2] = {5, 10};  /* Will be gang+worker partitioned */
    #pragma omp target teams map(tofrom: dst[0:global_N], shared_var) map(to: src[0:global_N]) \
                             num_teams(2)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < global_N; i++) {
            int team_id = omp_get_team_num();
            dst[i] = src[i] * shared_var[team_id % 2];
        }
    }
}

__attribute__((noinline, noipa))
void test_vector_partitioned(int *src, int *dst) {
    volatile int N = global_N;
    #pragma omp target teams distribute simd map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                                            num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        int vector_private = i & 0xF;  /* Vector-partitioned */
        dst[i] = src[i] ^ vector_private;
    }
}

__attribute__((noinline, noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    int gang_vector_var[4] = {1, 2, 3, 4};  /* Gang+vector partitioned */
    #pragma omp target teams distribute simd map(tofrom: dst[0:global_N], gang_vector_var) \
                                            map(to: src[0:global_N]) \
                                            num_teams(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        int team_id = omp_get_team_num();
        dst[i] = src[i] + gang_vector_var[team_id % 4] + (i & 0x3);
    }
}

__attribute__((noinline, noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    volatile int N = global_N;
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:global_N]) \
                                                         map(to: src[0:global_N]) \
                                                         num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int thread_id = omp_get_thread_num();
        int lane = i & 0x3;  /* Simd lane */
        dst[i] = src[i] + (thread_id * 10) + lane;
    }
}

__attribute__((noinline, noipa))
void test_fully_partitioned(int *src, int *dst) {
    int fully_partitioned[8][4][2];  /* 3D array for full partitioning */
    
    /* Initialize on host */
    for (int g = 0; g < 8; g++)
        for (int w = 0; w < 4; w++)
            for (int v = 0; v < 2; v++)
                fully_partitioned[g][w][v] = g * 100 + w * 10 + v;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N], fully_partitioned) \
                map(to: src[0:global_N]) \
                num_teams(8) num_threads(4) simdlen(2)
    for (int i = 0; i < global_N; i++) {
        int gang = omp_get_team_num();
        int worker = omp_get_thread_num();
        int vector = i & 0x1;
        dst[i] = src[i] * fully_partitioned[gang % 8][worker % 4][vector];
    }
}

__attribute__((noinline, noipa))
void test_mixed_clauses(int *src, int *dst) {
    /* Test with mixed data clauses to trigger various partitioning */
    int private_var = 7;
    int firstprivate_var = 13;
    int shared_arr[16] = {0};
    
    for (int i = 0; i < 16; i++) shared_arr[i] = i;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N], shared_arr) \
                map(to: src[0:global_N]) \
                firstprivate(firstprivate_var) private(private_var) \
                num_teams(4) num_threads(8) simdlen(4) nowait
    for (int i = 0; i < global_N; i++) {
        private_var = omp_get_thread_num();
        dst[i] = src[i] + shared_arr[i % 16] + firstprivate_var + private_var;
    }
    #pragma omp taskwait
}

int main() {
    int N = global_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (i * 3 + 7) % 97;  /* Arbitrary but deterministic */
        dst[i] = 0;
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
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
