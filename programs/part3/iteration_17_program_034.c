#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int global_N = SIZE;

/* Non-inline functions to ensure separate compilation units */
__attribute__((noinline))
void test_gang_redundant(int *src, int *dst) {
    int factor = 2;  // Will be firstprivate
    int offset = rand() % 10;  // Make it runtime-dependent
    
    #pragma omp target teams map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                             num_teams(4) thread_limit(32)
    {
        int local_factor = factor;  // gang redundant (case 0)
        #pragma omp distribute
        for (int i = 0; i < global_N; i++) {
            dst[i] = src[i] * local_factor + offset;
        }
    }
}

__attribute__((noinline))
void test_gang_partitioned(int *src, int *dst) {
    volatile int chunk = global_N / 4;
    
    #pragma omp target teams map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                             num_teams(4) thread_limit(32)
    {
        // gang partitioned (case 1) - each team gets its portion
        #pragma omp distribute
        for (int i = 0; i < global_N; i += chunk) {
            for (int j = i; j < i + chunk && j < global_N; j++) {
                dst[j] = src[j] * 3;
            }
        }
    }
}

__attribute__((noinline))
void test_worker_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                num_teams(2) num_threads(8)
    for (int i = 0; i < global_N; i++) {
        int worker_local = i % 8;  // worker partitioned (case 2)
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline))
void test_gang_worker_partitioned(int *src, int *dst) {
    int tile_size = 32;
    
    #pragma omp target teams map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                             num_teams(4)
    {
        #pragma omp distribute
        for (int team_start = 0; team_start < global_N; team_start += global_N/4) {
            #pragma omp parallel for
            for (int i = team_start; i < team_start + global_N/4 && i < global_N; i++) {
                // gang+worker partitioned (case 3)
                int team_id = omp_get_team_num();
                int thread_id = omp_get_thread_num();
                dst[i] = src[i] * team_id + thread_id;
            }
        }
    }
}

__attribute__((noinline))
void test_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                simdlen(8)
    for (int i = 0; i < global_N; i++) {
        // vector partitioned (case 4)
        int lane = i % 8;
        dst[i] = src[i] * lane;
    }
}

__attribute__((noinline))
void test_gang_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                num_teams(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        // gang+vector partitioned (case 5)
        int team_id = omp_get_team_num();
        int lane = i % 4;
        dst[i] = src[i] * team_id + lane;
    }
}

__attribute__((noinline))
void test_worker_vector_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        // worker+vector partitioned (case 6)
        int thread_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * thread_id + lane;
    }
}

__attribute__((noinline))
void test_fully_partitioned(int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(to: src[0:global_N]) map(from: dst[0:global_N]) \
                num_teams(4) num_threads(4) simdlen(4)
    for (int i = 0; i < global_N; i++) {
        // fully partitioned (case 7)
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * team_id + thread_id + lane;
    }
}

__attribute__((noinline))
void test_mixed_partitions(int *src, int *dst) {
    // Test with depend clause to vary runtime behavior
    int *tmp = (int*)malloc(global_N * sizeof(int));
    
    #pragma omp target data map(to: src[0:global_N]) map(alloc: tmp[0:global_N]) \
                            map(from: dst[0:global_N])
    {
        // First region with nowait
        #pragma omp target teams distribute nowait \
                    depend(out: tmp[0:global_N]) num_teams(2)
        for (int i = 0; i < global_N; i++) {
            tmp[i] = src[i] * 2;
        }
        
        // Second region with depend clause
        #pragma omp target teams distribute parallel for simd \
                    depend(in: tmp[0:global_N]) num_teams(2) num_threads(2) simdlen(2)
        for (int i = 0; i < global_N; i++) {
            dst[i] = tmp[i] + i % 16;
        }
    }
    
    free(tmp);
}

int main() {
    int N = global_N;
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    // Initialize with pattern
    for (int i = 0; i < N; i++) {
        src[i] = i;
        dst[i] = 0;
    }
    
    printf("Testing all data partitioning types...\n");
    
    // Execute all test patterns
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_partitions(src, dst);
    
    // Final reduction to compute checksum
    int checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
                map(to: dst[0:N]) map(tofrom: checksum) num_teams(2)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    // Additional test with private/firstprivate clauses
    int private_var = 42;
    int firstprivate_var = 100;
    
    #pragma omp target teams distribute parallel for \
                map(to: src[0:N]) map(from: dst[0:N]) \
                private(private_var) firstprivate(firstprivate_var) \
                num_teams(3) num_threads(5)
    for (int i = 0; i < N; i++) {
        private_var = omp_get_thread_num();
        dst[i] = src[i] + private_var + firstprivate_var;
    }
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < 10; i++) {
        verify_sum += dst[i];
    }
    printf("Verification sum (first 10 elements): %d\n", verify_sum);
    
    free(src);
    free(dst);
    
    return 0;
}
