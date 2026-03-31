/* test_cache_descriptors.c
 * 
 * This test program aims to cover CPUID leaf 2 cache descriptor cases
 * in GCC's i386 driver by targeting specific x86 microarchitectures.
 * Compile with different -march flags to trigger different cache configurations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Function attributes for targeting specific architectures */
#ifdef TARGET_PENTIUM3
__attribute__((target("arch=pentium3")))
#elif defined(TARGET_PENTIUM4)
__attribute__((target("arch=pentium4")))
#elif defined(TARGET_NOCONA)
__attribute__((target("arch=nocona")))
#elif defined(TARGET_K8)
__attribute__((target("arch=k8")))
#elif defined(TARGET_CORE2)
__attribute__((target("arch=core2")))
#endif
static void cache_thrash_benchmark(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than expected cache */
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
    
    /* Cache thrashing pattern */
    for (j = 0; j < iterations; j++) {
        /* Linear access with stride to test associativity */
        for (i = 0; i < elements; i += 64) {  /* 64-byte cache line stride */
            buffer[i] = buffer[i] * 3 + 1;
        }
        
        /* Reverse access pattern */
        for (i = elements - 1; i >= 0; i -= 128) {
            buffer[i] = buffer[i] / 2;
        }
        
        MB();  /* Prevent reordering */
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < elements; i += 256) {
        result ^= buffer[i];
    }
    
    /* Volatile use of result */
    if (result == 0x12345678) {  /* Unlikely condition */
        printf("Impossible result\n");
    }
    
    free(buffer);
}

/* Different benchmark variations for different cache sizes */
#ifdef TEST_CASE_0x0A
/* Targets: 8KB L1 cache (e.g., early Pentium III) */
__attribute__((target("arch=pentium3")))
static void test_case_0x0a(void) {
    cache_thrash_benchmark(1000, 16);  /* Use 16KB buffer to exceed L1 */
}
#endif

#ifdef TEST_CASE_0x0C
/* Targets: 16KB L1 cache */
__attribute__((target("arch=pentium3")))
static void test_case_0x0c(void) {
    cache_thrash_benchmark(1000, 32);
}
#endif

#ifdef TEST_CASE_0x21
/* Targets: 256KB L2 cache (e.g., Pentium III Coppermine) */
__attribute__((target("arch=pentium3")))
static void test_case_0x21(void) {
    cache_thrash_benchmark(500, 512);  /* 512KB buffer to exceed L2 */
}
#endif

#ifdef TEST_CASE_0x24
/* Targets: 1MB L2 cache (e.g., Pentium III Tualatin) */
__attribute__((target("arch=pentium3")))
static void test_case_0x24(void) {
    cache_thrash_benchmark(500, 2048);  /* 2MB buffer */
}
#endif

#ifdef TEST_CASE_0x2C
/* Targets: 32KB L1 cache (e.g., Pentium 4 Northwood) */
__attribute__((target("arch=pentium4")))
static void test_case_0x2c(void) {
    cache_thrash_benchmark(1000, 64);
}
#endif

#ifdef TEST_CASE_0x39
/* Targets: 128KB L2 cache */
__attribute__((target("arch=pentium4")))
static void test_case_0x39(void) {
    cache_thrash_benchmark(500, 256);
}
#endif

#ifdef TEST_CASE_0x41
/* Targets: 128KB L2 cache with 32-byte lines */
__attribute__((target("arch=pentium4")))
static void test_case_0x41(void) {
    cache_thrash_benchmark(500, 256);
}
#endif

#ifdef TEST_CASE_0x49
/* Targets: 4MB L2 cache (Xeon DP, not MP) 
 * Note: Need to ensure xeon_mp flag is false
 * Targeting nocona (Xeon DP) should work */
__attribute__((target("arch=nocona")))
static void test_case_0x49(void) {
    cache_thrash_benchmark(200, 8192);  /* 8MB buffer */
}
#endif

#ifdef TEST_CASE_0x60
/* Targets: 16KB L1 cache, 8-way (e.g., AMD K8) */
__attribute__((target("arch=k8")))
static void test_case_0x60(void) {
    cache_thrash_benchmark(1000, 32);
}
#endif

#ifdef TEST_CASE_0x78
/* Targets: 1MB L2 cache, 4-way */
__attribute__((target("arch=k8")))
static void test_case_0x78(void) {
    cache_thrash_benchmark(500, 2048);
}
#endif

#ifdef TEST_CASE_0x7A
/* Targets: 256KB L2 cache, 8-way */
__attribute__((target("arch=k8")))
static void test_case_0x7a(void) {
    cache_thrash_benchmark(500, 512);
}
#endif

#ifdef TEST_CASE_0x82
/* Targets: 256KB L2 cache, 8-way, 32-byte lines */
__attribute__((target("arch=pentium4")))
static void test_case_0x82(void) {
    cache_thrash_benchmark(500, 512);
}
#endif

#ifdef TEST_CASE_0x86
/* Targets: 512KB L2 cache, 4-way */
__attribute__((target("arch=core2")))
static void test_case_0x86(void) {
    cache_thrash_benchmark(500, 1024);
}
#endif

/* Main function with architecture-specific compilation */
int main(int argc, char **argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    printf("Cache descriptor test - iterations: %d\n", iterations);
    
    /* Call architecture-specific tests based on compile-time defines */
#ifdef TEST_CASE_0x0A
    test_case_0x0a();
#endif
    
#ifdef TEST_CASE_0x0C
    test_case_0x0c();
#endif
    
#ifdef TEST_CASE_0x21
    test_case_0x21();
#endif
    
#ifdef TEST_CASE_0x24
    test_case_0x24();
#endif
    
#ifdef TEST_CASE_0x2C
    test_case_0x2c();
#endif
    
#ifdef TEST_CASE_0x39
    test_case_0x39();
#endif
    
#ifdef TEST_CASE_0x41
    test_case_0x41();
#endif
    
#ifdef TEST_CASE_0x49
    test_case_0x49();
#endif
    
#ifdef TEST_CASE_0x60
    test_case_0x60();
#endif
    
#ifdef TEST_CASE_0x78
    test_case_0x78();
#endif
    
#ifdef TEST_CASE_0x7A
    test_case_0x7a();
#endif
    
#ifdef TEST_CASE_0x82
    test_case_0x82();
#endif
    
#ifdef TEST_CASE_0x86
    test_case_0x86();
#endif
    
    /* Generic cache test that should work on any architecture */
    cache_thrash_benchmark(iterations, 4096);  /* 4MB buffer */
    
    printf("Test completed\n");
    return 0;
}
