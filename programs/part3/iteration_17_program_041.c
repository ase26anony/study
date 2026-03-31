/* test_omp_partitioning.c - Exercise OpenMP data partitioning types */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent optimization */
volatile int use_rand = 1;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst, int n) {
    int factor = use_rand ? rand() % 10 + 1 : 3;
    
    #pragma omp target teams map(to: src[0:n]) map(from: dst[0:n]) \
                             num_teams(4) thread_limit(32)
    {
        int local_factor = factor;  /* gang redundant - case 0 */
        
        #pragma omp distribute
        for (int i = 0; i < n; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst, int n) {
    int offset = use_rand ? rand() % 100 : 50;
    
    #pragma omp target teams map(to: src[0:n]) map(from: dst[0:n]) \
                             num_teams(8)
    {
        /* gang partitioned - case 1 */
        #pragma omp distribute
        for (int i = 0; i < n; i++) {
            dst[i] = src[i] + offset;
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst, int n) {
    volatile int chunk = 64;  /* Prevent constant propagation */
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:n]) map(from: dst[0:n]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < n; i++) {
        /* worker partitioned - case 2 */
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + worker_id * chunk;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst, int n) {
    int scale = use_rand ? rand() % 5 + 1 : 2;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:n]) map(from: dst[0:n]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < n; i++) {
        /* gang+worker partitioned - case 3 */
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * scale + team_id * 1000 + thread_id;
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst, int n) {
    int vector_factor = use_rand ? rand() % 8 + 1 : 4;
    
    #pragma omp target teams distribute simd \
                map(to: src[0:n]) map(from: dst[0:n]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* vector partitioned - case 4 */
        dst[i] = src[i] * vector_factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst, int n) {
    int base = use_rand ? rand() % 1000 : 100;
    
    #pragma omp target teams distribute simd \
                map(to: src[0:n]) map(from: dst[0:n]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < n; i++) {
        /* gang+vector partitioned - case 5 */
        int team_id = omp_get_team_num();
        dst[i] = src[i] + base + team_id * 100;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst, int n) {
    volatile int stride = 16;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:n]) map(from: dst[0:n]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < n; i++) {
        /* worker+vector partitioned - case 6 */
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] + thread_id * stride;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst, int n) {
    volatile int multiplier = 3;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:n]) map(from: dst[0:n]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < n; i++) {
        /* fully partitioned - case 7 */
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        int lane = i % 2;  /* Simulated vector lane */
        dst[i] = src[i] * multiplier + team_id * 1000 + thread_id * 100 + lane;
    }
}

/* Additional test with depend clause */
__attribute__((noinline))
void test_with_depend(int *src, int *dst, int *tmp, int n) {
    /* First kernel */
    #pragma omp target teams map(to: src[0:n]) map(from: tmp[0:n]) \
                             depend(out: tmp[0:n]) nowait
    {
        #pragma omp distribute
        for (int i = 0; i < n; i++) {
            tmp[i] = src[i] * 2;
        }
    }
    
    /* Second kernel with dependency */
    #pragma omp target teams distribute parallel for \
                map(to: tmp[0:n]) map(from: dst[0:n]) \
                depend(in: tmp[0:n]) nowait
    for (int i = 0; i < n; i++) {
        dst[i] = tmp[i] + 1;
    }
    
    #pragma omp taskwait
}

int main() {
    volatile int N = ARRAY_SIZE;  /* Prevent constant folding */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i;
        dst[i] = 0;
        tmp[i] = 0;
    }
    
    /* Seed random for variability */
    srand(42);
    
    /* Execute all partitioning tests */
    test_gang_redundant(src, dst, N);
    test_gang_partitioned(src, dst, N);
    test_worker_partitioned(src, dst, N);
    test_gang_worker_partitioned(src, dst, N);
    test_vector_partitioned(src, dst, N);
    test_gang_vector_partitioned(src, dst, N);
    test_worker_vector_partitioned(src, dst, N);
    test_fully_partitioned(src, dst, N);
    test_with_depend(src, dst, tmp, N);
    
    /* Final reduction to compute checksum */
    int checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(to: dst[0:N]) reduction(+:checksum) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
