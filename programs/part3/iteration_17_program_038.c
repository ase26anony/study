#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

/* Prevent inlining to ensure separate compilation units */
__attribute__((noinline,noipa))
void test_gang_redundant(volatile int n, int *src, int *dst) {
    int factor = 2;  // Will be firstprivate
    #pragma omp target teams map(tofrom: dst[0:n]) map(to: src[0:n]) \
                             num_teams(4) thread_limit(32)
    {
        int local_factor = factor;  // Becomes gang redundant
        #pragma omp distribute
        for (int i = 0; i < n; i++) {
            dst[i] = src[i] * local_factor;
        }
    }
}

__attribute__((noinline,noipa))
void test_gang_partitioned(volatile int n, int *src, int *dst) {
    int offset = 10;
    #pragma omp target teams map(tofrom: dst[0:n]) map(to: src[0:n]) \
                             map(tofrom: offset) num_teams(8)
    {
        // offset is gang partitioned via map clause
        #pragma omp distribute
        for (int i = 0; i < n; i++) {
            dst[i] = src[i] + offset;
        }
    }
}

__attribute__((noinline,noipa))
void test_worker_partitioned(volatile int n, int *src, int *dst) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: dst[0:n]) map(to: src[0:n]) \
                num_teams(2) num_threads(16)
    for (int i = 0; i < n; i++) {
        int worker_local = i % 100;  // Worker partitioned
        dst[i] = src[i] + worker_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_worker_partitioned(volatile int n, int *src, int *dst) {
    int shared_var = 5;
    #pragma omp target teams map(tofrom: dst[0:n]) map(to: src[0:n]) \
                             map(tofrom: shared_var) num_teams(4)
    {
        // shared_var is gang partitioned
        #pragma omp distribute parallel for num_threads(8)
        for (int i = 0; i < n; i++) {
            int worker_local = omp_get_thread_num();  // Worker partitioned
            dst[i] = src[i] * shared_var + worker_local;
        }
    }
}

__attribute__((noinline,noipa))
void test_vector_partitioned(volatile int n, int *src, int *dst) {
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:n]) map(to: src[0:n]) \
                num_teams(1) simdlen(8)
    for (int i = 0; i < n; i++) {
        int vector_local = i & 0xF;  // Vector partitioned
        dst[i] = src[i] ^ vector_local;
    }
}

__attribute__((noinline,noipa))
void test_gang_vector_partitioned(volatile int n, int *src, int *dst) {
    int gang_var = 3;
    #pragma omp target teams distribute simd \
                map(tofrom: dst[0:n]) map(to: src[0:n]) \
                map(tofrom: gang_var) num_teams(4) simdlen(4)
    for (int i = 0; i < n; i++) {
        int vector_local = i % 8;  // Vector partitioned
        dst[i] = src[i] * gang_var + vector_local;
    }
}

__attribute__((noinline,noipa))
void test_worker_vector_partitioned(volatile int n, int *src, int *dst) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:n]) map(to: src[0:n]) \
                num_teams(2) num_threads(4) simdlen(4)
    for (int i = 0; i < n; i++) {
        int worker_vector_local = (i + omp_get_thread_num()) & 0x7;
        dst[i] = src[i] + worker_vector_local;
    }
}

__attribute__((noinline,noipa))
void test_fully_partitioned(volatile int n, int *src, int *dst) {
    int gang_var = 2;
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: dst[0:n]) map(to: src[0:n]) \
                map(tofrom: gang_var) \
                num_teams(4) num_threads(8) simdlen(4)
    for (int i = 0; i < n; i++) {
        int thread_local = omp_get_thread_num();  // Worker partitioned
        int vector_local = i & 0x3;               // Vector partitioned
        dst[i] = src[i] * gang_var + thread_local + vector_local;
    }
}

/* Additional test with nowait to vary runtime behavior */
__attribute__((noinline,noipa))
void test_with_nowait(volatile int n, int *src, int *dst, int *dst2) {
    #pragma omp target map(tofrom: dst[0:n]) map(to: src[0:n]) nowait
    {
        #pragma omp teams distribute parallel for num_teams(2)
        for (int i = 0; i < n; i++) {
            dst[i] = src[i] * 2;
        }
    }
    
    #pragma omp target map(tofrom: dst2[0:n]) map(to: src[0:n]) nowait
    {
        #pragma omp teams distribute parallel for num_teams(2)
        for (int i = 0; i < n; i++) {
            dst2[i] = src[i] + 100;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    volatile int n = N;  // Prevent constant folding
    int *src = (int*)malloc(n * sizeof(int));
    int *dst = (int*)malloc(n * sizeof(int));
    int *dst2 = (int*)malloc(n * sizeof(int));
    
    // Initialize with pattern
    for (int i = 0; i < n; i++) {
        src[i] = i + (rand() % 10);  // Add some randomness
        dst[i] = 0;
        dst2[i] = 0;
    }
    
    printf("Starting OpenMP offload tests...\n");
    
    // Test all partitioning types
    test_gang_redundant(n, src, dst);
    test_gang_partitioned(n, src, dst);
    test_worker_partitioned(n, src, dst);
    test_gang_worker_partitioned(n, src, dst);
    test_vector_partitioned(n, src, dst);
    test_gang_vector_partitioned(n, src, dst);
    test_worker_vector_partitioned(n, src, dst);
    test_fully_partitioned(n, src, dst);
    test_with_nowait(n, src, dst, dst2);
    
    // Final reduction to compute checksum and prevent dead code elimination
    long long checksum = 0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: checksum) map(to: dst[0:n]) \
                reduction(+:checksum) num_teams(2)
    for (int i = 0; i < n; i++) {
        checksum += dst[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    // Additional test with depend clause
    int *tmp = (int*)malloc(n * sizeof(int));
    #pragma omp target data map(to: src[0:n]) map(alloc: tmp[0:n])
    {
        #pragma omp target map(from: tmp[0:n]) depend(out: tmp)
        {
            #pragma omp teams distribute parallel for num_teams(2)
            for (int i = 0; i < n; i++) {
                tmp[i] = src[i] * 3;
            }
        }
        
        #pragma omp target map(tofrom: dst[0:n]) map(to: tmp[0:n]) depend(in: tmp)
        {
            #pragma omp teams distribute parallel for num_teams(2)
            for (int i = 0; i < n; i++) {
                dst[i] = tmp[i] + 7;
            }
        }
    }
    
    // Compute final checksum
    checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += dst[i];
    }
    printf("Final checksum after depend test: %lld\n", checksum);
    
    free(src);
    free(dst);
    free(dst2);
    free(tmp);
    
    return 0;
}
