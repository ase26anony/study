#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent optimization and constant folding */
volatile int global_N = SIZE;
volatile int seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline)) 
void test_gang_redundant(int *src, int *dst) {
    int factor = 3;
    int offset = seed;
    
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             firstprivate(factor, offset) num_teams(4) thread_limit(128)
    {
        int local_factor = factor;  /* gang redundant */
        int local_offset = offset;  /* gang redundant */
        
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * local_factor + local_offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int factor = seed % 5 + 1;
    
    #pragma omp target teams map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                             map(tofrom: factor) num_teams(8)
    {
        /* factor is gang partitioned due to map(tofrom:) */
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * factor + i;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int local_N = global_N;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:local_N]) map(to: src[0:local_N]) \
                num_teams(2) num_threads(32)
    for (int i = 0; i < local_N; i++) {
        int worker_local = i * 2;  /* worker partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int chunk_size = global_N / 4;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                map(to: chunk_size) num_teams(4) num_threads(16)
    for (int i = 0; i < global_N; i++) {
        int gang_worker_val = (i / chunk_size) * 100 + (i % 10);
        dst[i] = src[i] * gang_worker_val;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int factor = seed;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                firstprivate(factor) num_teams(1) simdlen(8)
    for (int i = 0; i < global_N; i++) {
        int vector_private = i % 16;  /* vector partitioned */
        dst[i] = src[i] + factor + vector_private;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int base = seed;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:global_N]) map(to: src[0:global_N]) \
                map(to: base) num_teams(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        int gang_vector = (i / 256) * 10 + (i % 4);  /* gang+vector partitioned */
        dst[i] = src[i] * base + gang_vector;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    volatile int N = global_N;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        int worker_vector = (omp_get_thread_num() * 100) + (i % 4);  /* worker+vector partitioned */
        dst[i] = src[i] + worker_vector;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = global_N;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(4) \
                private(N)
    for (int i = 0; i < N; i++) {
        int fully_part = (i / 64) * 1000 + (omp_get_thread_num() * 100) + (i % 4);  /* fully partitioned */
        dst[i] = src[i] * fully_part;
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mixed clauses to trigger various runtime paths */
    int a = seed;
    int b = seed * 2;
    int c = seed * 3;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:global_N], a) map(to: src[0:global_N], b) \
                firstprivate(c) private(b) \
                num_teams(3) num_threads(4) simdlen(2) \
                depend(inout: dst[0:global_N]) nowait
    for (int i = 0; i < global_N; i++) {
        int mixed = a + b + c + i + omp_get_team_num() + omp_get_thread_num() + (i % 2);
        dst[i] = src[i] + mixed;
    }
    #pragma omp taskwait
}

int main() {
    int N = global_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + seed;
        dst[i] = 0;
    }
    
    printf("Starting OpenMP offload tests...\n");
    
    /* Test all partitioning types */
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
    
    /* Final reduction to prevent dead code elimination */
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
