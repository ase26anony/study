#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent inlining to ensure separate runtime calls */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    volatile int factor = 2;  /* Volatile to prevent constant folding */
    int offset = 1;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        int local_factor = factor;  /* Firstprivate/gang redundant */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor + offset;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    volatile int chunk = N/4;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        /* Gang partitioned array section */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + chunk;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    volatile int offset = 10;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = offset + omp_get_thread_num();  /* Worker partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    volatile int base = 100;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        /* Both gang and worker partitioning occurs */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + base + gang_id * 1000 + worker_id;
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    volatile int factor = 3;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < N; i++) {
        /* Vector partitioned computation */
        dst[i] = src[i] * factor;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    volatile int base_val = 50;
    
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Gang + vector partitioning */
        int gang_mod = omp_get_team_num() % 2;
        dst[i] = src[i] + base_val + gang_mod;
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    volatile int multiplier = 2;
    
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Worker + vector partitioning */
        int thread_bias = omp_get_thread_num() * 10;
        dst[i] = src[i] * multiplier + thread_bias;
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    volatile int seed = 42;
    
    /* Complex construct to trigger full partitioning */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(2) \
                private(seed)
    for (int i = 0; i < N; i++) {
        /* Fully partitioned across all dimensions */
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        int lane_id = i % 2;  /* Simulates vector lane */
        dst[i] = src[i] + gang_id * 100 + worker_id * 10 + lane_id;
    }
}

__attribute__((noinline,optimize("O0")))
void test_mixed_clauses(int *src, int *dst) {
    volatile int shared_var = 5;
    int private_var = 10;
    
    /* Mix of clauses to trigger various runtime paths */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                firstprivate(private_var) shared(shared_var) \
                num_teams(3) num_threads(6)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + private_var + shared_var;
    }
}

__attribute__((noinline,optimize("O0")))
void test_nowait_regions(int *src, int *dst, int *dst2) {
    volatile int offset1 = 1, offset2 = 2;
    
    /* Multiple regions with nowait to test async execution */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             nowait depend(out: dst) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + offset1;
        }
    }
    
    #pragma omp target teams map(tofrom: dst2[0:N]) map(to: src[0:N]) \
                             nowait depend(out: dst2) num_teams(2)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst2[i] = src[i] * offset2;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    /* Volatile to prevent compile-time optimization */
    volatile int array_size = N;
    int *src = (int*)malloc(array_size * sizeof(int));
    int *dst = (int*)malloc(array_size * sizeof(int));
    int *dst2 = (int*)malloc(array_size * sizeof(int));
    
    if (!src || !dst || !dst2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = i + (rand() % 10);  /* Add some randomness */
        dst[i] = 0;
        dst2[i] = 0;
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
    test_nowait_regions(src, dst, dst2);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:array_size]) \
                num_teams(2)
    for (int i = 0; i < array_size; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional check with dst2 array */
    int checksum2 = 0;
    #pragma omp target teams distribute simd reduction(+:checksum2) \
                map(tofrom: checksum2) map(to: dst2[0:array_size])
    for (int i = 0; i < array_size; i++) {
        checksum2 += dst2[i];
    }
    
    printf("Secondary checksum: %d\n", checksum2);
    
    free(src);
    free(dst);
    free(dst2);
    
    return 0;
}
