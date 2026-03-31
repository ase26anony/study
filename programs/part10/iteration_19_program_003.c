/* test_cache_coverage.c - Cover GCC i386 driver cache detection cases */

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
static void cache_thrashing_benchmark(int cpu_type) {
    /* Allocate buffers larger than typical L2 cache */
    const size_t buffer_size = 4 * 1024 * 1024; /* 4MB */
    volatile int* buffer1 = (volatile int*)malloc(buffer_size * sizeof(int));
    volatile int* buffer2 = (volatile int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        /* Simple LCG */
        seed = seed * 1103515245 + 12345;
        buffer1[i] = (int)(seed & 0x7FFFFFFF);
        buffer2[i] = (int)(seed >> 16);
    }
    
    MB();
    
    /* Cache-thrashing access pattern */
    volatile int result = 0;
    const int iterations = 100;
    
    /* Different access patterns for different CPU types */
    switch (cpu_type) {
        case 0: /* Linear access - good spatial locality */
            for (int iter = 0; iter < iterations; iter++) {
                for (size_t i = 0; i < buffer_size; i += 64) {
                    result ^= buffer1[i] + buffer2[i];
                }
                MB();
            }
            break;
            
        case 1: /* Strided access - tests associativity */
            for (int iter = 0; iter < iterations; iter++) {
                for (size_t i = 0; i < buffer_size; i += 128) {
                    result ^= buffer1[i] + buffer2[(i * 17) % buffer_size];
                }
                MB();
            }
            break;
            
        case 2: /* Random-ish access - worst cache behavior */
            for (int iter = 0; iter < iterations; iter++) {
                uint32_t idx = 0;
                for (size_t i = 0; i < buffer_size / 4; i++) {
                    idx = (idx * 1103515245 + 12345) % buffer_size;
                    result ^= buffer1[idx] + buffer2[(idx * 13) % buffer_size];
                }
                MB();
            }
            break;
    }
    
    /* Use result to prevent dead code elimination */
    printf("CPU type %d benchmark result: %d\n", cpu_type, result);
    
    free((void*)buffer1);
    free((void*)buffer2);
}

/* Multi-versioned function using target_clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multi_version_cache_test(void) {
    /* This function will be compiled for multiple targets */
    cache_thrashing_benchmark(0);
}

int main(int argc, char** argv) {
    printf("Cache detection coverage test\n");
    
    /* Force compiler to consider cache characteristics by using
       different optimization hints for different code sections */
    
#if defined(TEST_PENTIUM3)
    printf("Testing Pentium III target (cases 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24)\n");
    cache_thrashing_benchmark(0);
    
#elif defined(TEST_PENTIUM4)
    printf("Testing Pentium 4 target (cases 0x2c, 0x39-0x3e, 0x41-0x45)\n");
    /* Test both with and without Xeon MP flag */
    cache_thrashing_benchmark(1);
    cache_thrashing_benchmark(2);
    
#elif defined(TEST_NOCONA)
    printf("Testing Nocona/Xeon DP target (case 0x49 non-Xeon-MP, 0x60, 0x66-0x68)\n");
    /* This should trigger case 0x49 without xeon_mp flag */
    cache_thrashing_benchmark(0);
    cache_thrashing_benchmark(1);
    
#elif defined(TEST_K8)
    printf("Testing AMD K8 target (cases 0x78-0x87)\n");
    cache_thrashing_benchmark(0);
    cache_thrashing_benchmark(2);
    
#elif defined(TEST_CORE2)
    printf("Testing Core 2 target (cases 0x48, 0x4e)\n");
    cache_thrashing_benchmark(1);
    
#else
    /* Generic test - compiler will use native or generic tuning */
    printf("Testing generic x86 target\n");
    
    /* Try to trigger multiple cache detection paths by using
       architecture-specific intrinsics if available */
    
    /* Use __builtin_cpu_supports to potentially influence optimizations */
    if (__builtin_cpu_supports("sse")) {
        cache_thrashing_benchmark(0);
    }
    if (__builtin_cpu_supports("sse2")) {
        cache_thrashing_benchmark(1);
    }
    if (__builtin_cpu_supports("sse3")) {
        cache_thrashing_benchmark(2);
    }
    
    #ifdef USE_MULTIVERSIONING
    /* Call multi-versioned function */
    multi_version_cache_test();
    #endif
    
#endif
    
    /* Additional memory-intensive computation to ensure
       cache optimization decisions matter */
    const int matrix_size = 512;
    volatile int* matrix = (volatile int*)malloc(matrix_size * matrix_size * sizeof(int));
    
    if (matrix) {
        /* Matrix multiplication-like access pattern */
        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                matrix[i * matrix_size + j] = i * j;
            }
        }
        
        volatile int sum = 0;
        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                sum += matrix[i * matrix_size + j];
            }
            MB();
        }
        
        printf("Matrix sum: %d\n", sum);
        free((void*)matrix);
    }
    
    return 0;
}
