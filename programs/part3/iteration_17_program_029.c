/* Test program to exercise GCC's OpenMP data partitioning string mapping function
 * Specifically targets the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1024

/* Prevent constant folding and dead code elimination */
volatile int dynamic_size = ARRAY_SIZE;

/* Non-inline functions to ensure separate compilation units in optimizer */
__attribute__((noinline, cold))
void test_gang_redundant(int *src, int *dst) {
    int N = dynamic_size;
    int factor = 2;  /* Will be firstprivate */
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
        firstprivate(factor) num_teams(4) thread_limit(32)
    {
        int local_factor = factor;  /* Gang redundant */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline, cold))
void test_gang_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    #pragma omp target teams map(tofrom: dst[0:N]) map(to: src[0:N]) \
        num_teams(8) thread_limit(64)
    {
        /* dst is gang partitioned */
        #pragma omp distribute
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] + i;
        }
    }
}

__attribute__((noinline, cold))
void test_worker_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: dst[0:N]) map(to: src[0:N]) \
        num_teams(2) num_threads(8)
    for (int i = 0; i < N; i++) {
        /* Each worker thread gets its own partition */
        int worker_id = omp_get_thread_num();
        dst[i] = src[i] * worker_id + i;
    }
}

__attribute__((noinline, cold))
void test_gang_worker_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    int shared_offset = 10;  /* Will be gang+worker partitioned */
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: dst[0:N], shared_offset) map(to: src[0:N]) \
        num_teams(4) num_threads(16)
    for (int i = 0; i < N; i++) {
        /* Two-level partitioning: across teams and threads */
        dst[i] = src[i] + shared_offset + omp_get_team_num() * 100 + omp_get_thread_num();
    }
}

__attribute__((noinline, cold))
void test_vector_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    int vector_factor = 3;
    
    #pragma omp target teams distribute simd \
        map(tofrom: dst[0:N]) map(to: src[0:N]) \
        firstprivate(vector_factor) num_teams(1) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* Vector partitioned within SIMD lanes */
        dst[i] = src[i] * vector_factor + (i % 8);
    }
}

__attribute__((noinline, cold))
void test_gang_vector_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    #pragma omp target teams distribute simd \
        map(tofrom: dst[0:N]) map(to: src[0:N]) \
        num_teams(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Partitioned across gangs and vector lanes */
        int gang_id = omp_get_team_num();
        int lane = i % 4;
        dst[i] = src[i] + gang_id * 1000 + lane * 100;
    }
}

__attribute__((noinline, cold))
void test_worker_vector_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Use nested parallelism to get worker+vector partitioning */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: dst[0:N]) map(to: src[0:N]) \
        num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < N; i++) {
        /* Partitioned across worker threads and vector lanes */
        int worker = omp_get_thread_num();
        int lane = i % 4;
        dst[i] = src[i] * worker + lane;
    }
}

__attribute__((noinline, cold))
void test_fully_partitioned(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Complex construct to trigger full partitioning */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: dst[0:N]) map(to: src[0:N]) \
        num_teams(4) num_threads(8) simdlen(2) \
        private(src, dst)  /* Makes everything partitioned */
    for (int i = 0; i < N; i++) {
        /* Fully partitioned: gang, worker, and vector */
        int gang = omp_get_team_num();
        int worker = omp_get_thread_num();
        int lane = i % 2;
        dst[i] = src[i] + gang * 10000 + worker * 1000 + lane * 100;
    }
}

__attribute__((noinline, cold))
void test_mixed_partitioning(int *src, int *dst) {
    int N = dynamic_size;
    
    /* Mix different data clauses in same region */
    int gang_private = 5;      /* Case 0: gang redundant */
    int gang_partitioned = 10; /* Case 1: gang partitioned */
    
    #pragma omp target teams map(tofrom: dst[0:N], gang_partitioned) \
        map(to: src[0:N]) firstprivate(gang_private) \
        num_teams(4) thread_limit(32)
    {
        int worker_private = 20; /* Case 2: worker partitioned */
        
        #pragma omp distribute parallel for private(worker_private)
        for (int i = 0; i < N; i++) {
            int vector_private = 30; /* Case 4: vector partitioned */
            #pragma omp simd private(vector_private)
            for (int j = 0; j < 4; j++) {
                /* This accesses should trigger multiple partitioning types */
                int idx = i * 4 + j;
                if (idx < N) {
                    dst[idx] = src[idx] + gang_private + gang_partitioned + 
                              worker_private + vector_private + 
                              omp_get_team_num() + omp_get_thread_num() + j;
                }
            }
        }
    }
}

int main() {
    int N = dynamic_size;
    
    /* Allocate and initialize arrays */
    int *src = (int*)malloc(N * sizeof(int));
    int *dst = (int*)malloc(N * sizeof(int));
    
    if (!src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < N; i++) {
        src[i] = i + (rand() % 10);  /* Prevent optimization */
        dst[i] = 0;
    }
    
    printf("Starting OpenMP offload partitioning tests...\n");
    
    /* Execute all test patterns to trigger different partitioning types */
    test_gang_redundant(src, dst);
    test_gang_partitioned(src, dst);
    test_worker_partitioned(src, dst);
    test_gang_worker_partitioned(src, dst);
    test_vector_partitioned(src, dst);
    test_gang_vector_partitioned(src, dst);
    test_worker_vector_partitioned(src, dst);
    test_fully_partitioned(src, dst);
    test_mixed_partitioning(src, dst);
    
    /* Final reduction to compute checksum and prevent dead code elimination */
    long long checksum = 0;
    #pragma omp target teams distribute parallel for reduction(+:checksum) \
        map(tofrom: checksum) map(to: dst[0:N]) \
        num_teams(2) num_threads(4)
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional test with nowait to vary runtime behavior */
    #pragma omp target map(to: src[0:N]) map(from: dst[0:N]) nowait
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            dst[i] = src[i] * 2;
        }
    }
    
    #pragma omp taskwait
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (dst[i] != src[i] * 2) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: expected %d, got %d\n", 
                       i, src[i] * 2, dst[i]);
            }
        }
    }
    
    if (errors > 0) {
        printf("Found %d errors in final verification\n", errors);
    } else {
        printf("All tests completed successfully\n");
    }
    
    free(src);
    free(dst);
    
    return 0;
}
