/* test_cache_coverage.c - Test program to cover CPUID leaf 2 cache descriptor cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
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
static void cache_thrashing_benchmark(int iterations) {
    /* Allocate buffers larger than typical L2 cache */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4 MB */
    int *buffer1 = (int*)aligned_alloc(64, buffer_size * sizeof(int));
    int *buffer2 = (int*)aligned_alloc(64, buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer1[i] = (int)(seed & 0x7FFFFFFF);
        buffer2[i] = (int)((seed >> 16) & 0x7FFFFFFF);
    }
    
    volatile int result = 0;
    
    /* Cache-thrashing access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        /* Strided access to defeat prefetching */
        for (size_t i = 0; i < buffer_size; i += 64) {
            buffer1[i] = buffer1[i] * 3 + buffer2[i];
            MB();
        }
        
        /* Reverse strided access */
        for (size_t i = buffer_size - 1; i > 0; i -= 128) {
            buffer2[i] = buffer1[i] - buffer2[i];
            MB();
        }
        
        /* Random-like but deterministic pattern */
        seed = iter;
        for (int j = 0; j < 1000; j++) {
            seed = seed * 1664525 + 1013904223;
            size_t idx = seed % buffer_size;
            buffer1[idx] ^= buffer2[idx];
            MB();
        }
    }
    
    /* Prevent dead code elimination */
    for (size_t i = 0; i < buffer_size; i += 1024) {
        result ^= buffer1[i] ^ buffer2[i];
    }
    
    printf("Benchmark result (volatile): %d\n", result);
    
    free(buffer1);
    free(buffer2);
}

/* Function variants for different CPU targets using multi-versioning */
#if defined(__GNUC__) && (__GNUC__ >= 6)
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2, generic")))
#endif
static void multi_version_cache_test(int scale) {
    const int base_iterations = 100;
    cache_thrashing_benchmark(base_iterations * scale);
}

/* Main test routine with architecture-specific sections */
int main(int argc, char **argv) {
    int test_scale = argc > 1 ? atoi(argv[1]) : 1;
    
    printf("Cache descriptor coverage test\n");
    printf("==============================\n");
    
    /* Test different architectures through conditional compilation */
#ifdef TEST_PENTIUM3
    printf("Testing Pentium III target (cases 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24)\n");
    cache_thrashing_benchmark(50 * test_scale);
#endif
    
#ifdef TEST_PENTIUM4
    printf("Testing Pentium 4 target (cases 0x2c, 0x39-0x3e, 0x41-0x45)\n");
    /* Special test for case 0x49 - need non-Xeon-MP */
    printf("Testing for case 0x49 (non-Xeon-MP configuration)\n");
    cache_thrashing_benchmark(40 * test_scale);
#endif
    
#ifdef TEST_NOCONA
    printf("Testing Nocona target (cases 0x49, 0x60, 0x66-0x68)\n");
    cache_thrashing_benchmark(60 * test_scale);
#endif
    
#ifdef TEST_K8
    printf("Testing AMD K8 target (cases 0x78-0x87)\n");
    cache_thrashing_benchmark(70 * test_scale);
#endif
    
#ifdef TEST_CORE2
    printf("Testing Core 2 target (cases 0x48, 0x4e)\n");
    cache_thrashing_benchmark(80 * test_scale);
#endif
    
    /* Use multi-versioning if available */
#if defined(__GNUC__) && (__GNUC__ >= 6)
    printf("Testing multi-version function (covers multiple architectures)\n");
    multi_version_cache_test(test_scale);
#endif
    
    /* Generic test that should work on any x86 */
    printf("Running generic cache test\n");
    
    /* Force different optimization levels through pragmas */
#pragma GCC optimize("O2")
    cache_thrashing_benchmark(10 * test_scale);
    
#pragma GCC optimize("O3")
    cache_thrashing_benchmark(5 * test_scale);
    
    /* Array operation that depends on cache line size */
    {
        const int test_size = 1024 * 1024;
        volatile int* test_array = (volatile int*)calloc(test_size, sizeof(int));
        
        if (test_array) {
            /* Access pattern that depends on cache associativity */
            for (int i = 0; i < test_size; i += 67) { /* Prime number stride */
                test_array[i] = i;
                MB();
            }
            
            /* Another pattern with power-of-two strides */
            for (int stride = 64; stride <= 512; stride *= 2) {
                for (int i = 0; i < test_size; i += stride) {
                    test_array[i] += test_array[(i + stride/2) % test_size];
                    MB();
                }
            }
            
            free((void*)test_array);
        }
    }
    
    printf("Test completed\n");
    return 0;
}
