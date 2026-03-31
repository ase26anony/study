/* test_cache_coverage.c - Comprehensive test to cover CPUID leaf 2 cache descriptor cases */
/* Compile with different -D flags and -march options to target specific CPUs */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function template */
static void cache_thrash(size_t buffer_size, int iterations) {
    volatile int* buffer = (volatile int*)malloc(buffer_size * sizeof(int));
    if (!buffer) return;
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    volatile int result = 0;
    
    /* Cache-thrashing access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        seed = iter;
        for (size_t i = 0; i < buffer_size; i++) {
            /* Linear congruential generator for access pattern */
            seed = seed * 1664525 + 1013904223;
            size_t idx = seed % buffer_size;
            
            /* Read-modify-write operation */
            int val = buffer[idx];
            MEMORY_BARRIER();
            buffer[idx] = val + 1;
            MEMORY_BARRIER();
            
            /* Accumulate to prevent dead code elimination */
            result ^= val;
        }
    }
    
    /* Use result to prevent optimization */
    if (result == 0x12345678) {
        printf("Impossible!\n");
    }
    
    free((void*)buffer);
}

/* Architecture-specific benchmark variants using GCC target attributes */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    printf("Testing Pentium III cache configuration...\n");
    /* L1: 16KB, L2: 256KB-512KB typical */
    cache_thrash(256 * 1024 / sizeof(int), 100);  /* > L1, < L2 */
    cache_thrash(1024 * 1024 / sizeof(int), 50);  /* > L2 */
}
#endif

#ifdef TEST_PENTIUM4
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x39-0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    printf("Testing Pentium 4 cache configuration...\n");
    /* L1: 8KB, L2: 256KB-2MB typical */
    cache_thrash(64 * 1024 / sizeof(int), 100);   /* > L1, < L2 */
    cache_thrash(2048 * 1024 / sizeof(int), 30);  /* > L2 */
}
#endif

#ifdef TEST_XEON_MP
/* Targets case 0x49 with xeon_mp=true (should skip assignment) */
__attribute__((target("arch=nocona")))  /* Nocona is Xeon DP, not MP */
static void benchmark_xeon_mp(void) {
    printf("Testing Xeon MP cache configuration...\n");
    /* Large L2 caches: 1MB-4MB */
    cache_thrash(512 * 1024 / sizeof(int), 80);
    cache_thrash(4096 * 1024 / sizeof(int), 20);
}
#endif

#ifdef TEST_XEON_DP
/* Targets case 0x49 with xeon_mp=false (should assign L2=4MB) */
__attribute__((target("arch=nocona")))
static void benchmark_xeon_dp(void) {
    printf("Testing Xeon DP (Nocona) cache configuration...\n");
    /* L2: 1MB-4MB */
    cache_thrash(1024 * 1024 / sizeof(int), 60);
    cache_thrash(8192 * 1024 / sizeof(int), 15);
}
#endif

#ifdef TEST_ATHLON64
/* Targets cases: 0x40, 0x48, 0x60, 0x66-0x68, 0x78-0x87 */
__attribute__((target("arch=k8")))
static void benchmark_athlon64(void) {
    printf("Testing Athlon 64 cache configuration...\n");
    /* L1: 64KB, L2: 512KB-1MB typical */
    cache_thrash(128 * 1024 / sizeof(int), 90);
    cache_thrash(2048 * 1024 / sizeof(int), 40);
}
#endif

#ifdef TEST_CORE2
/* Targets cases: 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x87 */
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    printf("Testing Core 2 cache configuration...\n");
    /* L1: 32KB, L2: 2MB-6MB typical */
    cache_thrash(256 * 1024 / sizeof(int), 80);
    cache_thrash(8192 * 1024 / sizeof(int), 20);
}
#endif

#ifdef TEST_NEHALEM
/* Targets cases with large shared L3 caches */
__attribute__((target("arch=nehalem")))
static void benchmark_nehalem(void) {
    printf("Testing Nehalem cache configuration...\n");
    /* L1: 32KB, L2: 256KB, L3: 4MB-8MB */
    cache_thrash(512 * 1024 / sizeof(int), 70);
    cache_thrash(16384 * 1024 / sizeof(int), 10);
}
#endif

/* Multi-versioned function that will generate code for multiple targets */
#if defined(USE_MULTIVERSIONING) && __GNUC__ >= 4
__attribute__((target_clones("pentium3, pentium4, k8, core2, nehalem")))
static void benchmark_multiversion(void) {
    printf("Multi-versioned benchmark running...\n");
    cache_thrash(1024 * 1024 / sizeof(int), 50);
}
#endif

/* Main test driver */
int main(int argc, char** argv) {
    printf("Cache descriptor coverage test\n");
    printf("==============================\n");
    
    /* Seed RNG for reproducible access patterns */
    srand(0x12345678);
    
    /* Run architecture-specific benchmarks based on compile-time defines */
    
#ifdef TEST_PENTIUM3
    benchmark_pentium3();
#endif
    
#ifdef TEST_PENTIUM4
    benchmark_pentium4();
#endif
    
#ifdef TEST_XEON_MP
    benchmark_xeon_mp();
#endif
    
#ifdef TEST_XEON_DP
    benchmark_xeon_dp();
#endif
    
#ifdef TEST_ATHLON64
    benchmark_athlon64();
#endif
    
#ifdef TEST_CORE2
    benchmark_core2();
#endif
    
#ifdef TEST_NEHALEM
    benchmark_nehalem();
#endif
    
#if defined(USE_MULTIVERSIONING) && __GNUC__ >= 4
    benchmark_multiversion();
#endif
    
    /* Default benchmark if no specific arch is targeted */
#if !defined(TEST_PENTIUM3) && !defined(TEST_PENTIUM4) && \
    !defined(TEST_XEON_MP) && !defined(TEST_XEON_DP) && \
    !defined(TEST_ATHLON64) && !defined(TEST_CORE2) && \
    !defined(TEST_NEHALEM) && !defined(USE_MULTIVERSIONING)
    printf("Running generic cache benchmark...\n");
    /* Test multiple cache sizes to potentially trigger various cases */
    cache_thrash(32 * 1024 / sizeof(int), 200);   /* Small L1 */
    cache_thrash(256 * 1024 / sizeof(int), 100);  /* Medium L2 */
    cache_thrash(2048 * 1024 / sizeof(int), 50);  /* Large L2 */
    cache_thrash(8192 * 1024 / sizeof(int), 20);  /* Very large */
#endif
    
    printf("Benchmark completed.\n");
    return 0;
}
