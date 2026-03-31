#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent optimization */
volatile int vol_N = N;
volatile int vol_seed = 42;

/* Non-inlined test functions to ensure separate compilation units */
__attribute__((noinline,optimize("no-tree-vectorize")))
void test_gang_redundant(int *src, int *dst) {
    int factor = vol_seed % 10 + 1;  /* Runtime-dependent */
    
    #pragma omp target teams map(to: src[0:vol_N]) map(from: dst[0:vol_N]) \
                             num_teams(4) thread_limit(64)
    {
        int local_factor = factor;  /* gang redundant - case 0 */
        
        #pragma omp distribute
        for (int i = 0; i < vol_N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_gang_partitioned(int *src, int *dst) {
    int offset = vol_seed % 100;
    
    #pragma omp target teams map(to: src[0:vol_N]) map(tofrom: dst[0:vol_N]) \
                             num_teams(8)
    {
        /* gang partitioned array - case 1 */
        int gang_array[vol_N];
        
        #pragma omp distribute
        for (int i = 0; i < vol_N; i++) {
            gang_array[i] = src[i] + offset;
            dst[i] = gang_array[i] * 2;
        }
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_worker_partitioned(int *src, int *dst) {
    int scale = vol_seed % 5 + 1;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:vol_N]) map(from: dst[0:vol_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < vol_N; i++) {
        /* worker partitioned variable - case 2 */
        int worker_local = scale * (omp_get_thread_num() + 1);
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_gang_worker_partitioned(int *src, int *dst) {
    volatile int chunk = vol_N / 4;
    
    #pragma omp target teams map(to: src[0:vol_N]) map(tofrom: dst[0:vol_N]) \
                             num_teams(4)
    {
        /* gang+worker partitioned - case 3 */
        int shared_array[vol_N];
        
        #pragma omp distribute
        for (int g = 0; g < 4; g++) {
            #pragma omp parallel for
            for (int i = g * chunk; i < (g + 1) * chunk && i < vol_N; i++) {
                shared_array[i] = src[i] * 2;
                dst[i] = shared_array[i] + omp_get_team_num();
            }
        }
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_vector_partitioned(int *src, int *dst) {
    int vector_factor = 3;
    
    #pragma omp target teams distribute simd \
                map(to: src[0:vol_N]) map(from: dst[0:vol_N]) \
                simdlen(8) num_teams(2)
    for (int i = 0; i < vol_N; i++) {
        /* vector partitioned - case 4 */
        int vector_private = vector_factor * (i % 8);
        dst[i] = src[i] + vector_private;
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_gang_vector_partitioned(int *src, int *dst) {
    int base = vol_seed % 20;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:vol_N]) map(from: dst[0:vol_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < vol_N; i++) {
        /* gang+vector partitioned - case 5 */
        int gang_vector_var = base + (i % 16);
        dst[i] = src[i] * gang_vector_var;
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_worker_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:vol_N]) map(from: dst[0:vol_N]) \
                num_teams(2) num_threads(4) simdlen(8) \
                private(src, dst)  /* Force worker+vector partitioning - case 6 */
    for (int i = 0; i < vol_N; i++) {
        int lane = i % 8;
        int thread_id = omp_get_thread_num();
        dst[i] = src[i] + lane + thread_id * 100;
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_fully_partitioned(int *src, int *dst) {
    volatile int teams = 4;
    volatile int threads = 8;
    volatile int vectors = 4;
    
    #pragma omp target teams num_teams(teams) thread_limit(threads) \
                map(to: src[0:vol_N]) map(tofrom: dst[0:vol_N])
    {
        /* fully partitioned - case 7 */
        int fully_partitioned[vol_N];
        
        #pragma omp distribute parallel for simd collapse(2) \
                    simdlen(vectors) num_threads(threads)
        for (int i = 0; i < vol_N; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < vol_N) {
                    fully_partitioned[idx] = src[idx] * 
                                           (omp_get_team_num() + 1) *
                                           (omp_get_thread_num() + 1) *
                                           ((idx % vectors) + 1);
                    dst[idx] = fully_partitioned[idx];
                }
            }
        }
    }
}

__attribute__((noinline,optimize("no-tree-vectorize")))
void test_mixed_partitioning(int *src, int *dst) {
    /* Mix different partition types in one region */
    int scalar = vol_seed;          /* gang redundant */
    int gang_array[vol_N];          /* gang partitioned */
    
    #pragma omp target teams map(to: src[0:vol_N]) map(from: dst[0:vol_N]) \
                             map(alloc: gang_array[0:vol_N]) \
                             num_teams(4)
    {
        #pragma omp distribute parallel for simd simdlen(4) num_threads(8)
        for (int i = 0; i < vol_N; i++) {
            /* This triggers multiple partition type queries */
            int worker_vec_private = scalar + (i % 4);  /* worker+vector partitioned */
            gang_array[i] = src[i] * 2;                 /* gang partitioned */
            dst[i] = gang_array[i] + worker_vec_private;
        }
    }
}

int main() {
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + vol_seed;
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
    
    /* Final reduction to compute checksum */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(to: dst[0:N]) reduction(+:checksum) \
                num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional test with nowait to vary runtime behavior */
    int *tmp = (int*)malloc(N * sizeof(int));
    #pragma omp target map(to: src[0:N]) map(from: tmp[0:N]) nowait
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            tmp[i] = src[i] * 3;
        }
    }
    #pragma omp taskwait
    
    free(src);
    free(dst);
    free(tmp);
    
    return 0;
}
