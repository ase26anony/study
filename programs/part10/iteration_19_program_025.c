/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
static void cache_thrash_benchmark(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than L2 cache to ensure thrashing */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    unsigned int seed = 0xDEADBEEF;
    for (i = 0; i < elements; i++) {
        buffer[i] = (int)(seed = seed * 1103515245 + 12345);
    }
    
    MB(); /* Memory barrier */
    
    /* Cache thrashing pattern: access elements with large stride */
    for (j = 0; j < iterations; j++) {
        /* Use different access patterns to stress different cache aspects */
        int stride = 17; /* Prime number to avoid cache line conflicts */
        
        /* Pattern 1: Sequential with stride */
        for (i = 0; i < elements; i += stride) {
            buffer[i] = buffer[i] * 3 + 1;
        }
        
        MB();
        
        /* Pattern 2: Reverse sequential */
        for (i = elements - 1; i >= 0; i -= stride) {
            buffer[i] = buffer[i] * 5 - 2;
        }
        
        MB();
        
        /* Pattern 3: Middle-out pattern */
        int mid = elements / 2;
        for (i = 0; i < mid; i++) {
            buffer[mid + i] = buffer[mid - i] + buffer[mid + i];
        }
        
        MB();
    }
    
    /* Compute final result to prevent dead code elimination */
    for (i = 0; i < elements; i += 64) { /* Cache line sized steps */
        result ^= buffer[i];
    }
    
    /* Volatile use to ensure computation isn't optimized away */
    volatile int final_result = result;
    (void)final_result; /* Suppress unused warning */
    
    free(buffer);
}

/* Multi-versioned function using target clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
static void multiarch_cache_test(void) {
    /* Test different buffer sizes to trigger different cache level detections */
    const int test_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        /* Fewer iterations for larger buffers */
        int iterations = 1000 / (test_sizes[i] / 64 + 1);
        if (iterations < 10) iterations = 10;
        
        cache_thrash_benchmark(iterations, test_sizes[i]);
    }
}

/* Individual test functions for specific cache descriptor cases */
#ifdef TEST_CASE_0x0A
__attribute__((target("arch=pentium3")))
void test_case_0x0a(void) {
    /* 8KB L1 cache - typical for early Pentium III */
    cache_thrash_benchmark(5000, 8);
    cache_thrash_benchmark(5000, 64); /* Exceed L1 */
}
#endif

#ifdef TEST_CASE_0x0C
__attribute__((target("arch=pentium3")))
void test_case_0x0c(void) {
    /* 16KB L1 cache */
    cache_thrash_benchmark(5000, 16);
    cache_thrash_benchmark(5000, 128);
}
#endif

#ifdef TEST_CASE_0x21
__attribute__((target("arch=pentium3")))
void test_case_0x21(void) {
    /* 256KB L2 cache */
    cache_thrash_benchmark(1000, 256);
    cache_thrash_benchmark(1000, 512); /* Exceed L2 */
}
#endif

#ifdef TEST_CASE_0x2C
__attribute__((target("arch=pentium4")))
void test_case_0x2c(void) {
    /* 32KB L1 cache - Pentium 4 */
    cache_thrash_benchmark(5000, 32);
    cache_thrash_benchmark(5000, 256);
}
#endif

#ifdef TEST_CASE_0x39
__attribute__((target("arch=pentium4")))
void test_case_0x39(void) {
    /* 128KB L2 cache */
    cache_thrash_benchmark(2000, 128);
    cache_thrash_benchmark(2000, 512);
}
#endif

#ifdef TEST_CASE_0x49
__attribute__((target("arch=nocona")))
void test_case_0x49(void) {
    /* 4096KB L2 cache - Xeon DP (not MP) */
    cache_thrash_benchmark(500, 4096);
    cache_thrash_benchmark(500, 8192); /* Exceed L2 */
}
#endif

#ifdef TEST_CASE_0x60
__attribute__((target("arch=nocona")))
void test_case_0x60(void) {
    /* 16KB L1 cache, 8-way */
    cache_thrash_benchmark(5000, 16);
    cache_thrash_benchmark(5000, 128);
}
#endif

#ifdef TEST_CASE_0x78
__attribute__((target("arch=k8")))
void test_case_0x78(void) {
    /* 1024KB L2 cache - AMD K8 */
    cache_thrash_benchmark(1000, 1024);
    cache_thrash_benchmark(1000, 2048);
}
#endif

#ifdef TEST_CASE_0x82
__attribute__((target("arch=k8")))
void test_case_0x82(void) {
    /* 256KB L2 cache, 32-byte line */
    cache_thrash_benchmark(2000, 256);
    cache_thrash_benchmark(2000, 1024);
}
#endif

#ifdef TEST_CASE_0x48
__attribute__((target("arch=core2")))
void test_case_0x48(void) {
    /* 3072KB L2 cache - Core 2 */
    cache_thrash_benchmark(800, 3072);
    cache_thrash_benchmark(800, 6144);
}
#endif

#ifdef TEST_CASE_0x4E
__attribute__((target("arch=core2")))
void test_case_0x4e(void) {
    /* 6144KB L2 cache */
    cache_thrash_benchmark(400, 6144);
    cache_thrash_benchmark(400, 12288);
}
#endif

int main(int argc, char **argv) {
    printf("Cache detection test for GCC i386 driver\n");
    
    /* Force compiler to consider cache optimizations */
    volatile int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
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
#ifdef TEST_CASE_0x2C
    test_case_0x2c();
#endif
#ifdef TEST_CASE_0x39
    test_case_0x39();
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
#ifdef TEST_CASE_0x82
    test_case_0x82();
#endif
#ifdef TEST_CASE_0x48
    test_case_0x48();
#endif
#ifdef TEST_CASE_0x4E
    test_case_0x4e();
#endif
    
    /* Multi-arch test if enabled */
#ifdef USE_MULTIVERSIONING
    multiarch_cache_test();
#endif
    
    /* Generic cache test that should work on any architecture */
    cache_thrash_benchmark(100, 1024);
    
    printf("Test completed\n");
    return 0;
}
