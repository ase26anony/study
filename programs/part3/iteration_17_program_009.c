/* test_omp_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent optimization and constant folding */
volatile int dynamic_size = ARRAY_SIZE;
volatile int seed_factor = 0;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline,optimize("O0")))
void test_gang_redundant(int *src, int *dst) {
    int N = dynamic_size;
    int factor = 3;
    int offset = seed_factor + 1;
    
    /* Case 0: gang redundant - scalar replicated across gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             firstprivate(factor, offset) num_teams(4)
    {
        int gang_private = factor * 2;  /* Will be gang redundant */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * gang_private + offset;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Case 1: gang partitioned - array partitioned across gangs */
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
                             num_teams(4) thread_limit(32)
    {
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * 2;
        }
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    int local_factor = seed_factor + 2;
    
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                firstprivate(local_factor) num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        int worker_local = local_factor + i % 16;  /* Worker partitioned */
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Case 3: gang+worker partitioned - two-level nesting */
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] * team_id + thread_id;  /* Both gang and worker dependent */
    }
}

__attribute__((noinline,optimize("O0")))
void test_vector_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    int base = seed_factor + 3;
    
    /* Case 4: vector partitioned - SIMD region */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                simdlen(8) num_teams(1)
    for (int i = 0; i < N; i++) {
        int lane = i % 8;  /* Simulates vector lane */
        dst[i] = src[i] * base + lane;
    }
}

__attribute__((noinline,optimize("O0")))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Case 5: gang+vector partitioned - teams with SIMD */
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        int team = omp_get_team_num();
        int lane = i % 4;
        dst[i] = src[i] * team + lane;  /* Gang and vector partitioned */
    }
}

__attribute__((noinline,optimize("O0")))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Case 6: worker+vector partitioned - parallel for SIMD */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(2) num_threads(4) simdlen(2)
    for (int i = 0; i < N; i++) {
        int thread = omp_get_thread_num();
        int lane = i % 2;
        dst[i] = src[i] + thread * 10 + lane;  /* Worker and vector partitioned */
    }
}

__attribute__((noinline,optimize("O0")))
void test_fully_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Case 7: fully partitioned - three-level nesting */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:N]) map(to: src[0:N]) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < N; i++) {
        int team = omp_get_team_num();
        int thread = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * 100 + team * 10 + thread + lane;  /* All three levels */
    }
}

/* Additional test with data dependencies */
__attribute__((noinline,optimize("O0")))
void test_with_depend(int *src, int *dst, int *tmp) {
    int N = dynamic_size;
    
    /* Create task dependencies to exercise runtime */
    #pragma omp target map(tofrom: tmp[0:N]) nowait depend(out: tmp)
    {
        #pragma omp teams distribute parallel for num_teams(2)
        for (int i = 0; i < N; i++) {
            tmp[i] = src[i] * 2;
        }
    }
    
    #pragma omp target map(tofrom: dst[0:N]) map(to: tmp[0:N]) \
                     nowait depend(in: tmp)
    {
        #pragma omp teams distribute parallel for simd num_teams(4) simdlen(2)
        for (int i = 0; i < N; i++) {
            dst[i] = tmp[i] + i;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    int N = dynamic_size;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !tmp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    seed_factor = rand() % 100;
    for (int i = 0; i < N; i++) {
        src[i] = (i + seed_factor) % 1000;
        dst[i] = 0;
        tmp[i] = 0;
    }
    
    printf("Testing OpenMP data partitioning patterns...\n");
    
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
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:N]) \
                reduction(+:checksum) num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with host computation */
    int host_checksum = 0;
    for (int i = 0; i < N; i++) {
        host_checksum += dst[i];
    }
    printf("Host verification checksum: %d\n", host_checksum);
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
