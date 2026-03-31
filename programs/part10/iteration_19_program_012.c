/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic by targeting specific x86 microarchitectures through compiler flags.
 * Each architecture section targets specific cache descriptor values.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Prevent compiler from optimizing away memory accesses */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function */
static void cache_thrash(size_t size_kb, int iterations) {
    /* Allocate buffer slightly larger than specified cache size */
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    volatile int result = 0;
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random values */
    for (size_t i = 0; i < elements; i++) {
        buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Perform cache-thrashing accesses */
    for (int iter = 0; iter < iterations; iter++) {
        /* Access pattern designed to stress cache associativity */
        for (size_t i = 0; i < elements; i++) {
            /* Use prime stride to avoid simple patterns */
            size_t idx = (i * 97) % elements;
            buffer[idx] = buffer[idx] * 3 + 1;
        }
        MEMORY_BARRIER();
    }
    
    /* Compute final result to prevent dead code elimination */
    for (size_t i = 0; i < elements; i++) {
        result ^= buffer[i];
    }
    
    /* Use result to prevent optimization */
    if (result == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    free(buffer);
}

/* Architecture-specific benchmark functions with target attributes */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    printf("Testing Pentium III cache configuration...\n");
    /* Exercise L1 and L2 caches */
    cache_thrash(16, 1000);   /* L1 size */
    cache_thrash(256, 500);   /* L2 size */
    cache_thrash(1024, 100);  /* Larger than L2 */
}
#endif

#ifdef TEST_PENTIUM4
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x39-0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    printf("Testing Pentium 4 cache configuration...\n");
    cache_thrash(32, 1000);   /* L1 data cache */
    cache_thrash(128, 500);   /* L2 typical */
    cache_thrash(512, 200);
    cache_thrash(2048, 100);
}
#endif

#ifdef TEST_XEON_MP
/* Special case for 0x49 with xeon_mp = true */
__attribute__((target("arch=nocona")))
static void benchmark_xeon_mp(void) {
    printf("Testing Xeon MP cache configuration (case 0x49 with xeon_mp=true)...\n");
    /* Large L2 cache typical for Xeon MP */
    cache_thrash(4096, 100);
    cache_thrash(8192, 50);
}
#endif

#ifdef TEST_XEON_DP
/* Targets case 0x49 with xeon_mp = false */
__attribute__((target("arch=nocona")))
static void benchmark_xeon_dp(void) {
    printf("Testing Xeon DP cache configuration (case 0x49 with xeon_mp=false)...\n");
    cache_thrash(4096, 100);
    cache_thrash(6144, 50);
}
#endif

#ifdef TEST_ATHLON64
/* Targets cases: 0x40, 0x78-0x87 */
__attribute__((target("arch=k8")))
static void benchmark_athlon64(void) {
    printf("Testing Athlon 64 cache configuration...\n");
    cache_thrash(64, 1000);   /* L1 */
    cache_thrash(512, 500);   /* L2 */
    cache_thrash(1024, 200);
}
#endif

#ifdef TEST_CORE2
/* Targets cases: 0x66, 0x67, 0x68, 0x78-0x87 */
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    printf("Testing Core 2 cache configuration...\n");
    cache_thrash(32, 1000);   /* L1 */
    cache_thrash(2048, 200);  /* L2 */
    cache_thrash(4096, 100);
}
#endif

#ifdef TEST_NEHALEM
/* Targets cases with larger caches */
__attribute__((target("arch=nehalem")))
static void benchmark_nehalem(void) {
    printf("Testing Nehalem cache configuration...\n");
    cache_thrash(32, 1000);   /* L1 */
    cache_thrash(256, 500);   /* L2 */
    cache_thrash(8192, 50);   /* L3 */
}
#endif

/* Main benchmark driver */
int main(void) {
    printf("Cache Detection Test Program\n");
    printf("============================\n\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
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
    
    /* Generic cache thrashing to ensure driver runs detection */
    printf("\nRunning generic cache benchmark...\n");
    cache_thrash(1024, 100);
    cache_thrash(4096, 50);
    cache_thrash(16384, 20);
    
    printf("\nTest completed.\n");
    return 0;
}
