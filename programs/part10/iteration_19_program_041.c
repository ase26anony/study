/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our benchmarks */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to exercise various cache descriptor cases */
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
static void benchmark_cache(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than expected cache to ensure thrashing */
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
    
    COMPILER_BARRIER();
    
    /* Cache-thrashing benchmark */
    for (j = 0; j < iterations; j++) {
        /* Access pattern designed to exercise different cache associativities */
        int stride = 17; /* Prime number to avoid power-of-two strides */
        for (i = 0; i < elements; i++) {
            int idx = (i * stride) % elements;
            buffer[idx] = buffer[(idx + 1) % elements] + 1;
        }
        
        /* Another pattern with different stride */
        stride = 13;
        for (i = elements - 1; i >= 0; i--) {
            int idx = (i * stride) % elements;
            result ^= buffer[idx];
        }
    }
    
    COMPILER_BARRIER();
    
    /* Force use of result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free(buffer);
}

/* Multi-versioned function using target_clones attribute */
#if defined(USE_MULTIVERSION) && __GNUC__ >= 6
__attribute__((target_clones("default,arch=pentium3,arch=pentium4,arch=nocona,arch=k8,arch=core2")))
#endif
static void multiarch_benchmark(void) {
    /* Different buffer sizes to potentially trigger different cache detection paths */
    benchmark_cache(100, 128);   /* Likely fits in L1 for some CPUs */
    benchmark_cache(50, 1024);   /* Likely fits in L2 for some CPUs */
    benchmark_cache(20, 8192);   /* Larger than L2 for most uncovered cases */
}

/* Specialized benchmarks for specific cache descriptor cases */
#ifdef COVER_CASE_0x49
/* Need to target a CPU that returns descriptor 0x49 but is not Xeon MP
   Intel Prescott or later Pentium 4 with 2MB L2 cache */
__attribute__((target("arch=prescott")))
static void benchmark_case_0x49(void) {
    /* Use array sizes that match 4MB L2 cache (case 0x49) */
    int buffer_size = 4 * 1024 * 1024; /* 4MB */
    benchmark_cache(100, buffer_size / 1024);
}
#endif

#ifdef COVER_CASE_0x0A_0x0C
/* Early Pentium III variants with 8KB/16KB L1 cache */
__attribute__((target("arch=pentium3")))
static void benchmark_early_p3(void) {
    /* Small arrays to fit in L1 */
    benchmark_cache(1000, 8);
    benchmark_cache(1000, 16);
}
#endif

#ifdef COVER_CASE_0x78_0x87
/* AMD K8 family cache configurations */
__attribute__((target("arch=k8")))
static void benchmark_amd_k8(void) {
    /* Various sizes matching K8 cache hierarchy */
    benchmark_cache(200, 64);    /* L1 size for some K8 */
    benchmark_cache(100, 512);   /* L2 size for some K8 */
    benchmark_cache(50, 1024);   /* L2 size for other K8 */
}
#endif

int main(int argc, char *argv[]) {
    printf("Cache detection coverage test\n");
    printf("=============================\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Main benchmark - will use appropriate version based on compilation flags */
#if defined(USE_MULTIVERSION)
    printf("Running multi-architecture benchmark...\n");
    multiarch_benchmark();
#else
    /* Run architecture-specific benchmarks based on compile-time defines */
    #ifdef COVER_CASE_0x49
    printf("Testing case 0x49 (4MB L2 cache, non-Xeon-MP)...\n");
    benchmark_case_0x49();
    #endif
    
    #ifdef COVER_CASE_0x0A_0x0C
    printf("Testing cases 0x0A-0x0C (Pentium III L1 cache)...\n");
    benchmark_early_p3();
    #endif
    
    #ifdef COVER_CASE_0x78_0x87
    printf("Testing cases 0x78-0x87 (AMD K8 cache)...\n");
    benchmark_amd_k8();
    #endif
    
    /* Default benchmark for current architecture */
    printf("Running generic benchmark...\n");
    benchmark_cache(100, 1024);
#endif
    
    /* Additional memory-intensive operation to ensure cache detection is used */
    {
        volatile int sink = 0;
        const int huge_size = 16 * 1024 * 1024; /* 16MB */
        int *huge_buffer = malloc(huge_size);
        
        if (huge_buffer) {
            /* Initialize with pattern */
            for (int i = 0; i < huge_size / sizeof(int); i++) {
                huge_buffer[i] = i;
            }
            
            /* Complex access pattern */
            for (int i = 0; i < 10000; i++) {
                int idx = (i * 997) % (huge_size / sizeof(int));
                sink += huge_buffer[idx];
                huge_buffer[idx] = sink;
            }
            
            free(huge_buffer);
            printf("Final sink value: %d\n", sink);
        }
    }
    
    return 0;
}
