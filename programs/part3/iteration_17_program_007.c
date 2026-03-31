#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent constant folding and ensure runtime execution */
volatile int vol_N = N;
volatile int vol_seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    int factor = 3;  /* Will be gang-redundant */
    #pragma omp target teams map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                             firstprivate(factor) num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (int i = 0; i < vol_N; i++) {
            dst[i] = src[i] * factor;  /* factor is gang-redundant */
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    int offset = 7;  /* Will be gang-partitioned through map */
    #pragma omp target teams map(tofrom: dst[0:vol_N], offset) map(to: src[0:vol_N]) \
                             num_teams(8)
    {
        #pragma omp distribute
        for (int i = 0; i < vol_N; i++) {
            dst[i] = src[i] + offset;  /* offset is gang-partitioned */
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    int worker_local;  /* Will be worker-partitioned */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                private(worker_local) num_teams(2)
    for (int i = 0; i < vol_N; i++) {
        worker_local = i % 16;  /* Worker-private variable */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    int shared_var = 5;  /* Will be gang+worker partitioned */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:vol_N], shared_var) map(to: src[0:vol_N]) \
                num_teams(4)
    for (int i = 0; i < vol_N; i++) {
        dst[i] = src[i] * shared_var + (i % 8);
        shared_var = (shared_var + 1) % 10;  /* Modified in parallel */
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    int vector_private;  /* Will be vector-partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                private(vector_private) num_teams(1)
    for (int i = 0; i < vol_N; i++) {
        vector_private = i & 0xF;  /* Vector-private computation */
        dst[i] = src[i] ^ vector_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    int gang_vector_var = 2;  /* Will be gang+vector partitioned */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:vol_N], gang_vector_var) map(to: src[0:vol_N]) \
                num_teams(4)
    for (int i = 0; i < vol_N; i++) {
        dst[i] = src[i] * gang_vector_var + (i % 32);
        /* gang_vector_var is both gang and vector partitioned */
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Worker+vector partitioned through nested parallelism */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                num_teams(2)
    for (int i = 0; i < vol_N; i++) {
        int worker_vector_private = (i % 64) + omp_get_thread_num();
        dst[i] = src[i] + worker_vector_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    int fully_partitioned_var = 1;  /* Will be fully partitioned */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:vol_N], fully_partitioned_var) \
                map(to: src[0:vol_N]) \
                num_teams(8)
    for (int i = 0; i < vol_N; i++) {
        /* fully_partitioned_var is gang+worker+vector partitioned */
        dst[i] = src[i] * fully_partitioned_var + 
                 (i % 128) + omp_get_thread_num();
        fully_partitioned_var = (fully_partitioned_var * 3) % 17;
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mix different data clauses to trigger various partitioning types */
    int gang_private = 10;
    int worker_private = 20;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:vol_N]) map(to: src[0:vol_N]) \
                firstprivate(gang_private) private(worker_private) \
                num_teams(4)
    for (int i = 0; i < vol_N; i++) {
        worker_private = i % 50;
        dst[i] = src[i] + gang_private + worker_private;
    }
}

__attribute__((noinline,optimize("O0")))
void test_nowait_regions(int *src, int *dst) {
    /* Use nowait to create multiple concurrent offload regions */
    int section1[N], section2[N];
    
    #pragma omp target teams map(to: src[0:vol_N/2]) \
                map(from: section1[0:vol_N/2]) nowait \
                num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < vol_N/2; i++) {
            section1[i] = src[i] * 2;
        }
    }
    
    #pragma omp target teams map(to: src[vol_N/2:vol_N/2]) \
                map(from: section2[0:vol_N/2]) nowait \
                num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < vol_N/2; i++) {
            section2[i] = src[vol_N/2 + i] + 100;
        }
    }
    
    #pragma omp taskwait
    
    /* Combine results */
    #pragma omp target teams distribute parallel for \
                map(to: section1[0:vol_N/2], section2[0:vol_N/2]) \
                map(from: dst[0:vol_N]) num_teams(4)
    for (int i = 0; i < vol_N; i++) {
        if (i < vol_N/2) {
            dst[i] = section1[i];
        } else {
            dst[i] = section2[i - vol_N/2];
        }
    }
}

int main() {
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    srand(vol_seed);
    for (int i = 0; i < N; i++) {
        src[i] = rand() % 1000;
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
    test_mixed_partitioning(src, dst);
    test_nowait_regions(src, dst);
    
    /* Final reduction to compute checksum and prevent optimization */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:vol_N]) \
                reduction(+:checksum) num_teams(2)
    for (int i = 0; i < vol_N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
