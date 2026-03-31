/* test_cache_coverage.c - Comprehensive cache detection test for GCC i386 driver */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our memory accesses */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger various cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets cases: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets cases: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
void benchmark_cache(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than L2 cache to ensure cache misses */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    for (i = 0; i < elements; i++) {
        buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Cache-thrashing benchmark */
    for (j = 0; j < iterations; j++) {
        /* Access pattern designed to stress different cache levels */
        int stride = 17; /* Prime number to avoid power-of-two strides */
        for (i = 0; i < elements; i += stride) {
            buffer[i] = buffer[(i + stride) % elements] + j;
        }
        
        /* Another pattern with different stride */
        stride = 13;
        for (i = 0; i < elements; i += stride) {
            buffer[(i + 7) % elements] = buffer[i] * 3;
        }
        
        COMPILER_BARRIER();
    }
    
    /* Compute final result to prevent dead code elimination */
    for (i = 0; i < elements; i += 256) { /* Sample every 256th element */
        result ^= buffer[i];
    }
    
    /* Use result to prevent optimization */
    if (result == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    free(buffer);
}

/* Multi-versioned function using target_clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
void multiarch_cache_test() {
    /* Test different buffer sizes to stress different cache levels */
    benchmark_cache(100, 128);   /* L1 cache size */
    benchmark_cache(50, 1024);   /* L2 cache size */
    benchmark_cache(20, 8192);   /* Larger than L2 */
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache detection coverage test...\n");
    
    start = clock();
    
    /* Call architecture-specific versions if compiled with multi-versioning */
#ifdef USE_MULTIVERSIONING
    multiarch_cache_test();
#else
    /* Otherwise use the version compiled for specific architecture */
    benchmark_cache(100, 128);
    benchmark_cache(50, 1024);
    benchmark_cache(20, 8192);
#endif
    
    /* Additional memory-intensive operations */
    {
        volatile int temp = 0;
        int *large_buffer = (int*)malloc(16 * 1024 * 1024); /* 16MB */
        if (large_buffer) {
            /* Fill with pattern */
            for (int i = 0; i < (16 * 1024 * 1024 / sizeof(int)); i++) {
                large_buffer[i] = i;
            }
            
            /* Access in pseudo-random order */
            uint32_t rng = 123456789;
            for (int i = 0; i < 1000000; i++) {
                rng = rng * 1103515245 + 12345;
                int idx = rng % (16 * 1024 * 1024 / sizeof(int));
                temp += large_buffer[idx];
                large_buffer[idx] = temp;
            }
            
            free(large_buffer);
            
            /* Use temp to prevent optimization */
            if (temp == 0x87654321) {
                printf("Another impossible condition\n");
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    
    return 0;
}
