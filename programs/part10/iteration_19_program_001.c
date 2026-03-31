/* test_cache_detection.c - Comprehensive test for GCC i386 driver cache detection */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

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
static void cache_thrashing_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Pseudo-random access pattern using linear congruential generator */
    uint32_t seed = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        /* Write phase - fill buffer with pattern */
        for (i = 0; i < size; i++) {
            buffer[i] = (int)(seed ^ i);
            seed = seed * 1103515245 + 12345;
        }
        COMPILER_BARRIER();
        
        /* Read-modify-write phase with stride */
        for (i = 0; i < size; i += 64) {  /* 64-byte cache line sized stride */
            buffer[i % size] += buffer[(i + 256) % size];
        }
        COMPILER_BARRIER();
        
        /* Gather phase - sum elements with pseudo-random pattern */
        seed = 0xCAFEBABE;
        for (i = 0; i < size / 4; i++) {
            sink += buffer[seed % size];
            seed = seed * 1664525 + 1013904223;
        }
    }
    
    /* Prevent dead code elimination */
    if (sink == 0x12345678) {
        printf("Impossible condition\n");
    }
}

/* Specialized functions for specific cache descriptor cases */
#ifdef COVER_CASE_0x49
/* Target Xeon DP (not MP) to trigger case 0x49 without xeon_mp flag */
__attribute__((target("arch=nocona")))
static void test_case_0x49(void) {
    /* Large working set to engage L2 cache detection */
    size_t buffer_size = 8 * 1024 * 1024 / sizeof(int);  /* 8MB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Allocation failed for case 0x49\n");
        return;
    }
    
    /* Intensive computation to force cache consideration */
    cache_thrashing_benchmark(buffer, buffer_size, 100);
    
    free(buffer);
}
#endif

#ifdef COVER_CASE_0x0A_0x0C
/* Target early Pentium III for small L1 cache cases */
__attribute__((target("arch=pentium3")))
static void test_cases_0x0a_0x0c(void) {
    /* Smaller buffer to fit in L1/L2 boundary */
    size_t buffer_size = 32 * 1024 / sizeof(int);  /* 32KB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Allocation failed for cases 0x0a-0x0c\n");
        return;
    }
    
    /* High iteration count to stress L1 cache */
    cache_thrashing_benchmark(buffer, buffer_size, 1000);
    
    free(buffer);
}
#endif

#ifdef COVER_CASE_0x21_0x24
/* Target Pentium 4 with 256KB/1MB L2 cache */
__attribute__((target("arch=pentium4")))
static void test_cases_0x21_0x24(void) {
    /* Buffer sized between 256KB and 1MB */
    size_t buffer_size = 768 * 1024 / sizeof(int);  /* 768KB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Allocation failed for cases 0x21-0x24\n");
        return;
    }
    
    cache_thrashing_benchmark(buffer, buffer_size, 500);
    
    free(buffer);
}
#endif

#ifdef COVER_CASE_0x78_0x87
/* Target AMD K8 for various L2 cache configurations */
__attribute__((target("arch=k8")))
static void test_cases_0x78_0x87(void) {
    /* Large buffer to exceed L1, fit in various L2 sizes */
    size_t buffer_size = 2 * 1024 * 1024 / sizeof(int);  /* 2MB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Allocation failed for cases 0x78-0x87\n");
        return;
    }
    
    cache_thrashing_benchmark(buffer, buffer_size, 200);
    
    free(buffer);
}
#endif

#ifdef COVER_CASE_0x60_0x68
/* Target Core 2 for newer L1 cache configurations */
__attribute__((target("arch=core2")))
static void test_cases_0x60_0x68(void) {
    /* Buffer sized for L1 cache stress */
    size_t buffer_size = 64 * 1024 / sizeof(int);  /* 64KB */
    int *buffer = malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Allocation failed for cases 0x60-0x68\n");
        return;
    }
    
    cache_thrashing_benchmark(buffer, buffer_size, 1500);
    
    free(buffer);
}
#endif

/* Main function with multi-versioning support */
#ifdef USE_MULTI_VERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2, nehalem")))
#endif
int main(int argc, char **argv) {
    volatile int final_result = 0;
    size_t i;
    
    printf("Starting cache detection test...\n");
    
    /* Allocate buffers of different sizes to stress various cache levels */
    size_t sizes[] = {
        16 * 1024 / sizeof(int),      /* 16KB - L1 stress */
        128 * 1024 / sizeof(int),     /* 128KB - L1/L2 boundary */
        512 * 1024 / sizeof(int),     /* 512KB - Medium L2 */
        2 * 1024 * 1024 / sizeof(int), /* 2MB - Large L2 */
        8 * 1024 * 1024 / sizeof(int)  /* 8MB - Exceeds L2 */
    };
    
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    /* Run benchmarks for each buffer size */
    for (i = 0; i < num_sizes; i++) {
        int *buffer = malloc(sizes[i] * sizeof(int));
        if (!buffer) {
            fprintf(stderr, "Failed to allocate %zu bytes\n", sizes[i] * sizeof(int));
            continue;
        }
        
        /* Initialize with pattern */
        for (size_t j = 0; j < sizes[i]; j++) {
            buffer[j] = (int)(j ^ 0xAA55AA55);
        }
        
        /* Run benchmark */
        cache_thrashing_benchmark(buffer, sizes[i], 100);
        
        /* Accumulate result to prevent optimization */
        for (size_t j = 0; j < sizes[i]; j += 128) {
            final_result ^= buffer[j % sizes[i]];
        }
        
        free(buffer);
        COMPILER_BARRIER();
    }
    
    /* Call architecture-specific tests if compiled with corresponding defines */
#ifdef COVER_CASE_0x49
    test_case_0x49();
#endif
    
#ifdef COVER_CASE_0x0A_0x0C
    test_cases_0x0a_0x0c();
#endif
    
#ifdef COVER_CASE_0x21_0x24
    test_cases_0x21_0x24();
#endif
    
#ifdef COVER_CASE_0x78_0x87
    test_cases_0x78_0x87();
#endif
    
#ifdef COVER_CASE_0x60_0x68
    test_cases_0x60_0x68();
#endif
    
    printf("Test completed. Result: %d\n", final_result);
    
    return 0;
}
