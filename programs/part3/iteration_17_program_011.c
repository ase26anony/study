#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent optimization and constant folding */
volatile int N = SIZE;
volatile int seed = 42;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,noipa))
void test_gang_redundant(int *src, int *dst) {
    int factor = seed;  /* Will be gang-redundant */
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(128)
    {
        int local_factor = factor;  /* firstprivate/gang-redundant */
        
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + local_factor;
        }
    }
}

__attribute__((noinline,noipa))
void test_gang_partitioned(int *src, int *dst) {
    int offset = seed;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(8) thread_limit(64)
    {
        /* dst is gang-partitioned through map clause */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * 2 + offset;
        }
    }
}

__attribute__((noinline,noipa))
void test_worker_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = i % 16;  /* worker-partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    int base = seed;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4)
    {
        /* Shared across teams but partitioned within teams */
        #pragma omp distribute
        for (int gang = 0; gang < N/256; gang++) {
            #pragma omp parallel for
            for (int i = gang*256; i < (gang+1)*256 && i < N; i++) {
                int worker_id = omp_get_thread_num();
                dst[i] = src[i] * 3 + base + worker_id;
            }
        }
    }
}

__attribute__((noinline,noipa))
void test_vector_partitioned(int *src, int *dst) {
    int scale = seed % 10;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        int lane = i % 8;  /* vector-partitioned */
        dst[i] = src[i] * scale + lane;
    }
}

__attribute__((noinline,noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int gang_factor = (i / (N/4)) * 100;
        int lane = i % 4;
        dst[i] = src[i] + gang_factor + lane;
    }
}

__attribute__((noinline,noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    int multiplier = seed;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        int worker_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * multiplier + worker_id * 10 + lane;
    }
}

__attribute__((noinline,noipa))
void test_fully_partitioned(int *src, int *dst) {
    /* Complex nested construct to trigger full partitioning */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int gang = 0; gang < N/128; gang++) {
            #pragma omp parallel for simd num_threads(4) simdlen(4)
            for (int i = gang*128; i < (gang+1)*128 && i < N; i++) {
                int gang_id = omp_get_team_num();
                int worker_id = omp_get_thread_num();
                int lane = i % 4;
                dst[i] = src[i] + gang_id * 1000 + worker_id * 100 + lane;
            }
        }
    }
}

__attribute__((noinline,noipa))
void test_mixed_clauses(int *src, int *dst) {
    /* Test with various data clauses to trigger different partitioning */
    int a = seed;
    int b = seed * 2;
    int c = seed + 5;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                firstprivate(a) private(b) lastprivate(c) \
                num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < N; i++) {
        b = i % 8;  /* private variable */
        dst[i] = src[i] + a + b;
        c = dst[i];  /* lastprivate update */
    }
    
    /* Use c to prevent dead code elimination */
    dst[0] += c;
}

int main() {
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (i * 17 + seed) % 1000;
        dst[i] = 0;
    }
    
    printf("Testing various OpenMP offload partitioning patterns...\n");
    
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
    
    /* Final reduction to compute checksum and prevent optimization */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional host-side verification */
    int host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    printf("Host verification checksum: %d\n", host_checksum);
    
    free(src);
    free(dst);
    
    return 0;
}
