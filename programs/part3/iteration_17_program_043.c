/* test_omp_partitioning.c
 * 
 * This program exercises various OpenMP offload data partitioning patterns
 * to trigger the runtime string mapping function for all switch cases in
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -o test_omp test_omp_partitioning.c
 * Or for host-only testing: gcc -O2 -fopenmp -foffload=disable -o test_omp_host test_omp_partitioning.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = ARRAY_SIZE;
volatile int seed_factor = 0;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int N = volatile_N;
    int factor = seed_factor + 1;  /* Make runtime-dependent */
    
    /* Case 0: gang redundant - scalar replicated across all gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        int private_scalar = factor;  /* firstprivate by default in teams region */
        
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + private_scalar;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams distribute map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                         num_teams(4) thread_limit(32)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] * 2;
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int offset = seed_factor + 2;
    
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                                    num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = offset + omp_get_thread_num();  /* worker-partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 3: gang+worker partitioned - two-level nesting */
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                                    num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        /* Both gang (team) and worker (thread) partitioning occur here */
        dst[i] = src[i] + omp_get_team_num() * 100 + omp_get_thread_num();
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    int vector_factor = seed_factor + 3;
    
    /* Case 4: vector partitioned - SIMD region */
    #pragma omp target teams distribute simd map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                             num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* vector-private computation */
        dst[i] = src[i] * vector_factor;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 5: gang+vector partitioned - teams distribute with SIMD */
    #pragma omp target teams distribute simd map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                             num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Both gang and vector partitioning */
        dst[i] = src[i] + omp_get_team_num() * 10;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 6: worker+vector partitioned - parallel for SIMD */
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                                          num_teams(2) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Worker and vector partitioning */
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * (worker_id + 1);
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 7: fully partitioned - nested teams+parallel+SIMD */
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                                          num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* All three levels: gang (team), worker (thread), and vector */
        dst[i] = src[i] + omp_get_team_num() * 1000 + omp_get_thread_num() * 10;
    }
}

/* Additional test with nowait to vary runtime behavior */
__attribute__((noinline))
void test_with_nowait(int *src, int *dst) {
    int N = volatile_N;
    
    /* Mix of partitioning with nowait clause */
    #pragma omp target teams distribute nowait map(tofrom: dst[0:N/2]) map(to: src[0:N/2]) \
                                               num_teams(2)
    for (int i = 0; i < N/2; i++) {
        dst[i] = src[i] * 3;
    }
    
    #pragma omp target teams distribute parallel for nowait \
               map(tofrom: dst[N/2:N/2]) map(to: src[N/2:N/2]) \
               num_teams(2) num_threads(4)
    for (int i = N/2; i < N; i++) {
        dst[i] = src[i] + omp_get_thread_num();
    }
    
    #pragma omp taskwait
}

int main() {
    int N = volatile_N;
    
    /* Initialize arrays with runtime-dependent pattern */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Use rand() to make initialization runtime-dependent */
    srand(42);
    seed_factor = rand() % 10;
    
    for (int i = 0; i < N; i++) {
        src[i] = i + seed_factor;
        dst[i] = 0;
    }
    
    printf("Testing OpenMP offload data partitioning patterns...\n");
    
    /* Execute all test patterns to trigger various runtime partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_with_nowait(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                         map(tofrom: checksum) map(to: dst[0:N]) \
                         num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify some results to ensure computations actually happened */
    int errors = 0;
    for (int i = 0; i < 10 && i < N; i++) {
        if (dst[i] != 0) {
            printf("dst[%d] = %d\n", i, dst[i]);
        } else {
            errors++;
        }
    }
    
    if (errors == 10) {
        printf("Warning: All sampled dst values are zero!\n");
    }
    
    free(src);
    free(dst);
    
    return 0;
}
