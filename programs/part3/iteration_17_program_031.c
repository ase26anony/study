#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int volatile_N = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline, noipa))
void test_gang_redundant(int *src, int *dst) {
    int N = volatile_N;
    int factor = 2;  /* Will be gang-redundant */
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        /* Case 0: gang redundant - firstprivate scalar */
        int local_factor = factor;  /* Implicitly firstprivate */
        
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline, noipa))
void test_gang_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 1: gang partitioned - mapped array partitioned across gangs */
    #pragma omp target teams distribute map(tofrom: dst[0:N]) map(to: src[0:N]) \
                                        num_teams(4)
    for (int i = 0; i < N; i++) {
        dst[i] = src[i] + i;
    }
}

__attribute__((noinline, noipa))
void test_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 2: worker partitioned - variable in parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = i % 16;  /* Worker-partitioned */
        dst[i] = src[i] * worker_local;
    }
}

__attribute__((noinline, noipa))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        int gang_id = omp_get_team_num();
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] + gang_id * 1000 + worker_id;
    }
}

__attribute__((noinline, noipa))
void test_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 4: vector partitioned - SIMD with vector-private variable */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        int vector_private = i & 0xF;  /* Vector-partitioned */
        dst[i] = src[i] ^ vector_private;
    }
}

__attribute__((noinline, noipa))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int gang_vector = (i + omp_get_team_num()) & 0xFF;
        dst[i] = src[i] * gang_vector;
    }
}

__attribute__((noinline, noipa))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 6: worker+vector partitioned - parallel for with SIMD */
    #pragma omp target parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        int worker_vector = (i + omp_get_thread_num()) % 32;
        dst[i] = src[i] + worker_vector;
    }
}

__attribute__((noinline, noipa))
void test_fully_partitioned(int *src, int *dst) {
    int N = volatile_N;
    
    /* Case 7: fully partitioned - teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(2)
    for (int i = 0; i < N; i++) {
        int fully_private = i + omp_get_team_num() * 1000 + 
                           omp_get_thread_num() * 100 + 
                           (i % 4);  /* SIMD lane variation */
        dst[i] = src[i] * fully_private;
    }
}

/* Additional test with depend clause to vary runtime behavior */
__attribute__((noinline, noipa))
void test_with_depend(int *src, int *dst, int *tmp) {
    int N = volatile_N;
    
    /* First kernel with output dependency */
    #pragma omp target teams distribute parallel for \
                map(tofrom: tmp[0:N]) map(to: src[0:N]) \
                depend(out: tmp) nowait
    for (int i = 0; i < N; i++) {
        tmp[i] = src[i] * 3;
    }
    
    /* Second kernel with input dependency */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: tmp[0:N]) \
                depend(in: tmp)
    for (int i = 0; i < N; i++) {
        dst[i] = tmp[i] + 7;
    }
    
    #pragma omp taskwait
}

int main() {
    int N = volatile_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (i * 13 + 7) % 97;  /* Non-trivial pattern */
        dst[i] = 0;
        tmp[i] = 0;
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
    test_with_depend(src, dst, tmp);
    
    /* Final reduction to compute checksum and prevent elimination */
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N])
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
