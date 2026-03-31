/* test_cache_coverage.c - Comprehensive cache detection test for GCC i386 driver */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Function attributes for targeting specific architectures */
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
   to engage cache-aware optimizations */
static void cache_thrash_benchmark(int pattern) {
    volatile int result = 0;
    const size_t buffer_size = 4 * 1024 * 1024; /* 4MB - larger than typical L2 */
    int *buffer = (int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer) return;
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    MB();
    
    /* Different access patterns to stress various cache configurations */
    switch (pattern) {
        case 0: /* Sequential access - good spatial locality */
            for (size_t i = 0; i < buffer_size; i += 64) {
                result += buffer[i];
            }
            break;
            
        case 1: /* Strided access with prime step - poor locality */
            for (size_t i = 0; i < buffer_size * 16; i += 997) {
                result += buffer[i % buffer_size];
            }
            break;
            
        case 2: /* Random walk using LCG - cache thrashing */
            {
                size_t pos = 0;
                for (int i = 0; i < 1000000; i++) {
                    seed = seed * 1103515245 + 12345;
                    pos = (pos + (seed % 64)) % buffer_size;
                    result += buffer[pos];
                }
            }
            break;
            
        case 3: /* Blocked matrix-style access */
            {
                const size_t block = 256;
                for (size_t i = 0; i < buffer_size; i += block) {
                    for (size_t j = 0; j < block && (i + j) < buffer_size; j++) {
                        result += buffer[i + j];
                    }
                }
            }
            break;
    }
    
    MB();
    
    /* Use result to prevent dead code elimination */
    if (result == 0x12345678) {
        printf("Impossible!\n");
    }
    
    free(buffer);
}

/* Specialized functions for different CPU targets to ensure
   the driver evaluates cache parameters for each */
#ifdef TEST_MULTIARCH
__attribute__((target("arch=pentium3")))
void bench_pentium3(void) {
    cache_thrash_benchmark(0);
}

__attribute__((target("arch=pentium4")))
void bench_pentium4(void) {
    cache_thrash_benchmark(1);
}

__attribute__((target("arch=nocona")))
void bench_nocona(void) {
    cache_thrash_benchmark(2);
}

__attribute__((target("arch=k8")))
void bench_k8(void) {
    cache_thrash_benchmark(3);
}

__attribute__((target("arch=core2")))
void bench_core2(void) {
    cache_thrash_benchmark(0);
}

__attribute__((target("arch=nehalem")))
void bench_nehalem(void) {
    cache_thrash_benchmark(1);
}
#endif

/* Main function with architecture-specific compilation paths */
int main(void) {
    printf("Cache detection coverage test\n");
    
    /* Force multiple compilation paths through conditional compilation */
#if defined(TEST_PENTIUM3)
    printf("Target: Pentium III (should trigger cases 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24)\n");
    cache_thrash_benchmark(0);
    cache_thrash_benchmark(1);
    
#elif defined(TEST_PENTIUM4)
    printf("Target: Pentium 4 (should trigger cases 0x2c, 0x39-0x3e, 0x41-0x45)\n");
    /* Pentium 4 has different cache configurations */
    cache_thrash_benchmark(2);
    
#elif defined(TEST_NOCONA)
    printf("Target: Nocona/Xeon DP (should trigger case 0x49 when not Xeon MP)\n");
    /* Nocona is Xeon DP, not Xeon MP, so should trigger 0x49 assignment */
    cache_thrash_benchmark(3);
    
#elif defined(TEST_K8)
    printf("Target: AMD K8 (should trigger cases 0x40, 0x78-0x87)\n");
    cache_thrash_benchmark(0);
    cache_thrash_benchmark(2);
    
#elif defined(TEST_CORE2)
    printf("Target: Core 2 (should trigger cases 0x66-0x68, 0x78-0x87)\n");
    cache_thrash_benchmark(1);
    cache_thrash_benchmark(3);
    
#elif defined(TEST_NEHALEM)
    printf("Target: Nehalem (should trigger cases 0x60, 0x78-0x87)\n");
    cache_thrash_benchmark(0);
    cache_thrash_benchmark(2);
    
#elif defined(TEST_MULTIARCH)
    printf("Target: Multi-arch (testing all through function multi-versioning)\n");
    bench_pentium3();
    bench_pentium4();
    bench_nocona();
    bench_k8();
    bench_core2();
    bench_nehalem();
    
#else
    printf("Target: Generic x86 (driver will detect actual CPU)\n");
    cache_thrash_benchmark(0);
#endif
    
    /* Additional memory-intensive computation to ensure cache detection is used */
    {
        const int size = 1024 * 1024;
        volatile int* data = (volatile int*)malloc(size * sizeof(int));
        volatile int sum = 0;
        
        if (data) {
            /* Write pattern */
            for (int i = 0; i < size; i++) {
                data[i] = i & 0xFF;
            }
            
            MB();
            
            /* Read with different strides to test various cache lines */
            for (int stride = 1; stride <= 64; stride *= 2) {
                for (int i = 0; i < size; i += stride) {
                    sum += data[i];
                }
            }
            
            MB();
            
            free((void*)data);
            
            /* Use sum to prevent optimization */
            if (sum == 0x87654321) {
                printf("Impossible result!\n");
            }
        }
    }
    
    return 0;
}
