#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int dynamic_size = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  /* Will be gang-redundant */
    int offset = rand() % 100;  /* Prevent optimization */
    
    #pragma omp target teams map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                              firstprivate(factor, offset) num_teams(4)
    {
        /* Case 0: gang redundant - factor is replicated across all gangs */
        #pragma omp distribute
        for (int i = 0; i < dynamic_size; i++) {
            dst[i] = src[i] * factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    /* Case 1: gang partitioned - arrays are partitioned across gangs */
    #pragma omp target teams distribute map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                                         num_teams(4)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * 3;
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    /* Case 2: worker partitioned - within parallel region */
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:dynamic_size]) \
                                                    map(to: src[0:dynamic_size]) \
                                                    num_teams(2) num_threads(4)
    for (int i = 0; i < dynamic_size; i++) {
        int local_var = i;  /* Worker partitioned */
        dst[i] = src[i] + local_var;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    /* Case 3: gang+worker partitioned - two-level partitioning */
    #pragma omp target teams distribute parallel for map(tofrom: dst[0:dynamic_size]) \
                                                    map(to: src[0:dynamic_size]) \
                                                    num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        /* Shared across workers within a gang, partitioned across gangs */
        dst[i] = src[i] * (omp_get_team_num() + 1);
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    /* Case 4: vector partitioned - SIMD vectorization */
    #pragma omp target teams distribute simd map(tofrom: dst[0:dynamic_size]) \
                                            map(to: src[0:dynamic_size]) \
                                            num_teams(1) simdlen(8)
    for (int i = 0; i < dynamic_size; i++) {
        /* Each SIMD lane gets its own instance */
        dst[i] = src[i] * 5;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    /* Case 5: gang+vector partitioned */
    #pragma omp target teams distribute simd map(tofrom: dst[0:dynamic_size]) \
                                            map(to: src[0:dynamic_size]) \
                                            num_teams(4) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * (omp_get_team_num() + 2);
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    /* Case 6: worker+vector partitioned */
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:dynamic_size]) \
                                                         map(to: src[0:dynamic_size]) \
                                                         num_teams(1) num_threads(4) simdlen(4)
    for (int i = 0; i < dynamic_size; i++) {
        int thread_factor = omp_get_thread_num() + 1;
        dst[i] = src[i] * thread_factor;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma omp target teams distribute parallel for simd map(tofrom: dst[0:dynamic_size]) \
                                                         map(to: src[0:dynamic_size]) \
                                                         num_teams(2) num_threads(2) simdlen(2)
    for (int i = 0; i < dynamic_size; i++) {
        /* All three levels of partitioning are active */
        int team = omp_get_team_num();
        int thread = omp_get_thread_num();
        dst[i] = src[i] * (team * 10 + thread + 1);
    }
}

__attribute__((noinline))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mixed clauses to trigger various runtime paths */
    int gang_private = rand() % 100;
    int worker_private;
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                firstprivate(gang_private) private(worker_private) \
                num_teams(3) num_threads(3)
    for (int i = 0; i < dynamic_size; i++) {
        worker_private = omp_get_thread_num();
        dst[i] = src[i] + gang_private + worker_private;
    }
}

__attribute__((noinline))
void test_nowait_regions(int *src, int *dst, int *dst2) {
    /* Use nowait to create multiple concurrent regions */
    #pragma omp target teams distribute nowait \
                map(tofrom: dst[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        dst[i] = src[i] * 2;
    }
    
    #pragma omp target teams distribute nowait \
                map(tofrom: dst2[0:dynamic_size]) map(to: src[0:dynamic_size]) \
                num_teams(2)
    for (int i = 0; i < dynamic_size; i++) {
        dst2[i] = src[i] * 3;
    }
    
    #pragma omp taskwait
}

int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = dynamic_size;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    int *dst2 = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst || !dst2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i;
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
    test_mixed_partitioning(src, dst);
    test_nowait_regions(src, dst, dst2);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(tofrom: checksum) map(to: dst[0:N]) num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional reduction on second array */
    long long checksum2 = 0;
    #pragma omp target teams distribute parallel for simd reduction(+:checksum2) \
                map(tofrom: checksum2) map(to: dst2[0:N]) \
                num_teams(2) simdlen(4)
    for (int i = 0; i < N; i++) {
        checksum2 += dst2[i];
    }
    
    printf("Second checksum: %lld\n", checksum2);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(dst2);
    
    return 0;
}
