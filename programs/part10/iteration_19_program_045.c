/* test_cache_coverage.c - Cover GCC i386 driver cache detection cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Target-specific function attributes */
#ifdef TEST_PENTIUM3
__attribute__((target("arch=pentium3")))
#elif defined(TEST_PENTIUM4)
__attribute__((target("arch=pentium4")))
#elif defined(TEST_NOCONA)
__attribute__((target("arch=nocona")))
#elif defined(TEST_K8)
__attribute__((target("arch=k8")))
#elif defined(TEST_CORE2)
__attribute__((target("arch=core2")))
#elif defined(TEST_NEHALEM)
__attribute__((target("arch=nehalem")))
#endif

/* Cache-thrashing benchmark that uses different access patterns
   to trigger cache-aware optimizations */
static void cache_thrash_benchmark(int pattern_id) {
    volatile int result = 0;
    const size_t buffer_size = 4 * 1024 * 1024; /* 4MB - larger than typical L2 */
    int *buffer = (int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer) return;
    
    /* Initialize with pseudo-random values */
    unsigned int seed = pattern_id * 1234567;
    for (size_t i = 0; i < buffer_size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFF);
    }
    
    /* Different access patterns to stress different cache aspects */
    switch (pattern_id % 4) {
        case 0: /* Sequential access - good spatial locality */
            for (size_t i = 0; i < buffer_size; i += 64) {
                result += buffer[i];
            }
            break;
            
        case 1: /* Strided access - tests associativity */
            for (size_t i = 0; i < buffer_size; i += 257) {
                result += buffer[i % buffer_size];
            }
            break;
            
        case 2: /* Random-ish access - poor locality */
            seed = pattern_id;
            for (size_t i = 0; i < 10000; i++) {
                seed = seed * 1664525 + 1013904223;
                size_t idx = seed % buffer_size;
                result += buffer[idx];
            }
            break;
            
        case 3: /* Block access - tests cache line utilization */
            for (size_t block = 0; block < buffer_size; block += 1024) {
                for (size_t j = 0; j < 256 && (block + j) < buffer_size; j++) {
                    result += buffer[block + j];
                }
            }
            break;
    }
    
    MB(); /* Ensure memory operations complete */
    
    /* Use result to prevent dead code elimination */
    if (result == 0x1234) { /* Never true, but compiler doesn't know */
        printf("Impossible result: %d\n", result);
    }
    
    free(buffer);
}

/* Multi-versioned benchmark function using target clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2, nehalem")))
#endif
void run_comprehensive_benchmark() {
    volatile int final_result = 0;
    
    /* Allocate buffers of different sizes to stress all cache levels */
    const size_t l1_size = 32 * 1024;      /* Typical L1 size */
    const size_t l2_size = 256 * 1024;     /* Typical L2 size */
    const size_t l3_size = 2 * 1024 * 1024; /* Typical L3 size */
    
    int *l1_buffer = (int*)malloc(l1_size);
    int *l2_buffer = (int*)malloc(l2_size);
    int *l3_buffer = (int*)malloc(l3_size);
    
    if (!l1_buffer || !l2_buffer || !l3_buffer) {
        free(l1_buffer); free(l2_buffer); free(l3_buffer);
        return;
    }
    
    /* Initialize with deterministic pattern */
    for (size_t i = 0; i < l1_size / sizeof(int); i++) {
        l1_buffer[i] = (int)(i * 3);
    }
    for (size_t i = 0; i < l2_size / sizeof(int); i++) {
        l2_buffer[i] = (int)(i * 5);
    }
    for (size_t i = 0; i < l3_size / sizeof(int); i++) {
        l3_buffer[i] = (int)(i * 7);
    }
    
    MB();
    
    /* Matrix multiplication-like pattern - heavily cache dependent */
    const int dim = 128;
    int *A = (int*)malloc(dim * dim * sizeof(int));
    int *B = (int*)malloc(dim * dim * sizeof(int));
    int *C = (int*)malloc(dim * dim * sizeof(int));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                A[i*dim + j] = (i + j) % 7;
                B[i*dim + j] = (i - j + dim) % 11;
                C[i*dim + j] = 0;
            }
        }
        
        /* Classic triple loop - cache behavior depends on loop order */
        for (int i = 0; i < dim; i++) {
            for (int k = 0; k < dim; k++) {
                int aik = A[i*dim + k];
                for (int j = 0; j < dim; j++) {
                    C[i*dim + j] += aik * B[k*dim + j];
                }
            }
        }
        
        /* Sum results to prevent elimination */
        for (int i = 0; i < dim * dim; i++) {
            final_result ^= C[i];
        }
        
        free(A); free(B); free(C);
    }
    
    /* Combine all results */
    for (size_t i = 0; i < l1_size / sizeof(int); i += 8) {
        final_result += l1_buffer[i];
    }
    for (size_t i = 0; i < l2_size / sizeof(int); i += 16) {
        final_result += l2_buffer[i];
    }
    for (size_t i = 0; i < l3_size / sizeof(int); i += 32) {
        final_result += l3_buffer[i];
    }
    
    MB();
    
    /* Print something to ensure execution */
    if (final_result != 0) {
        printf("Benchmark completed. Checksum: %d\n", final_result);
    }
    
    free(l1_buffer); free(l2_buffer); free(l3_buffer);
}

/* Main function with architecture-specific sections */
int main() {
    printf("Cache detection coverage test\n");
    
    /* Run benchmarks multiple times to ensure cache detection is triggered
       during compilation for optimization decisions */
    
#ifdef TEST_PENTIUM3
    /* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, etc. */
    printf("Testing Pentium III target...\n");
    cache_thrash_benchmark(0);
    cache_thrash_benchmark(1);
#elif defined(TEST_PENTIUM4)
    /* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
    printf("Testing Pentium 4 target...\n");
    cache_thrash_benchmark(2);
    cache_thrash_benchmark(3);
#elif defined(TEST_NOCONA)
    /* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68, 0x78-0x87 */
    printf("Testing Nocona/Xeon DP target...\n");
    cache_thrash_benchmark(4);
    cache_thrash_benchmark(5);
#elif defined(TEST_K8)
    /* Targets cases: 0x40 series, 0x78-0x87 for AMD K8 */
    printf("Testing AMD K8 target...\n");
    cache_thrash_benchmark(6);
    cache_thrash_benchmark(7);
#elif defined(TEST_CORE2)
    /* Targets cases: 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x87 */
    printf("Testing Core 2 target...\n");
    cache_thrash_benchmark(8);
    cache_thrash_benchmark(9);
#elif defined(TEST_NEHALEM)
    /* Targets newer cache configurations */
    printf("Testing Nehalem target...\n");
    cache_thrash_benchmark(10);
    cache_thrash_benchmark(11);
#else
    /* Generic test - compiler will use native or generic detection */
    printf("Testing generic x86 target...\n");
    cache_thrash_benchmark(12);
#endif
    
    /* Run comprehensive benchmark if multi-versioning is enabled */
#ifdef USE_MULTIVERSIONING
    run_comprehensive_benchmark();
#endif
    
    /* Additional memory-intensive operation to ensure cache matters */
    volatile int sum = 0;
    const int iter_count = 1000000;
    int *data = (int*)malloc(iter_count * sizeof(int));
    
    if (data) {
        for (int i = 0; i < iter_count; i++) {
            data[i] = i;
        }
        
        /* Access in reverse order to break prefetching */
        for (int i = iter_count - 1; i >= 0; i--) {
            sum += data[i];
        }
        
        /* Scramble access pattern */
        unsigned int rng = 123456789;
        for (int i = 0; i < iter_count; i++) {
            rng = rng * 1103515245 + 12345;
            int idx = rng % iter_count;
            sum ^= data[idx];
        }
        
        free(data);
    }
    
    printf("Test completed. Final sum: %d\n", sum);
    return 0;
}
