/* test_cache_coverage.c
 * 
 * This program is designed to trigger GCC's i386 driver cache detection logic
 * for specific CPUID leaf 2 descriptor values. It uses multiple techniques:
 * 1. Architecture-specific compilation targets
 * 2. Function multi-versioning via __attribute__((target()))
 * 3. Cache-thrashing memory access patterns
 * 4. Conditional compilation for different CPU families
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Cache line size assumption for padding */
#define CACHE_LINE_SIZE 64

/* Different test configurations targeting specific cache descriptor cases */
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
static void cache_thrashing_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Use a pseudo-random access pattern that depends on cache parameters */
    for (j = 0; j < iterations; j++) {
        /* Linear access with stride - exercises cache line size */
        for (i = 0; i < size; i += 16) {
            buffer[i] = buffer[i] + 1;
        }
        MB();
        
        /* Reverse access pattern */
        for (i = size - 1; i > 0; i -= 16) {
            buffer[i] = buffer[i] - 1;
        }
        MB();
        
        /* Strided access with prime number to break simple patterns */
        for (i = 0; i < size; i = (i + 257) % size) {
            sink += buffer[i];
        }
        MB();
    }
    
    /* Prevent dead code elimination */
    (void)sink;
}

/* Multi-versioned function that will generate different code paths
 * for different CPU targets during compilation */
__attribute__((target_clones("pentium3,pentium4,nocona,k8,core2,generic")))
static void multi_version_cache_test(void) {
    const size_t l1_size = 32 * 1024;      /* 32KB - typical L1 */
    const size_t l2_size = 256 * 1024;     /* 256KB - typical L2 */
    const size_t huge_size = 4 * 1024 * 1024; /* 4MB - exceeds L3 */
    
    int *l1_buffer = aligned_alloc(CACHE_LINE_SIZE, l1_size);
    int *l2_buffer = aligned_alloc(CACHE_LINE_SIZE, l2_size);
    int *huge_buffer = aligned_alloc(CACHE_LINE_SIZE, huge_size);
    
    if (!l1_buffer || !l2_buffer || !huge_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with non-zero values */
    for (size_t i = 0; i < l1_size / sizeof(int); i++) {
        l1_buffer[i] = i & 0xFF;
    }
    for (size_t i = 0; i < l2_size / sizeof(int); i++) {
        l2_buffer[i] = i & 0xFF;
    }
    for (size_t i = 0; i < huge_size / sizeof(int); i++) {
        huge_buffer[i] = i & 0xFF;
    }
    
    /* Test different access patterns that stress cache hierarchy */
    
    /* 1. L1 cache working set */
    cache_thrashing_benchmark(l1_buffer, l1_size / sizeof(int), 1000);
    
    /* 2. L2 cache working set */
    cache_thrashing_benchmark(l2_buffer, l2_size / sizeof(int), 500);
    
    /* 3. Large working set that exceeds cache */
    cache_thrashing_benchmark(huge_buffer, huge_size / sizeof(int), 10);
    
    /* Mix access patterns between buffers to create cache contention */
    volatile int mix_sink = 0;
    for (int iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < l1_size / sizeof(int); i += 64) {
            mix_sink += l1_buffer[i];
            mix_sink += l2_buffer[i % (l2_size / sizeof(int))];
            mix_sink += huge_buffer[i % (huge_size / sizeof(int))];
        }
        MB();
    }
    
    /* Final computation to ensure all memory is used */
    volatile int final_result = 0;
    for (size_t i = 0; i < l1_size / sizeof(int); i++) {
        final_result ^= l1_buffer[i];
    }
    for (size_t i = 0; i < l2_size / sizeof(int); i++) {
        final_result ^= l2_buffer[i];
    }
    for (size_t i = 0; i < huge_size / sizeof(int); i += 1024) {
        final_result ^= huge_buffer[i];
    }
    
    /* Print something to prevent optimization */
    printf("Cache test result: %d\n", final_result);
    
    free(l1_buffer);
    free(l2_buffer);
    free(huge_buffer);
}

/* Individual test functions for specific cache descriptor cases */
#ifdef TEST_CASE_0x0A
/* 8KB L1, 2-way, 32B line - Pentium III */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_CASE_0x0C
/* 16KB L1, 4-way, 32B line - Pentium III */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_CASE_0x21
/* 256KB L2, 8-way, 64B line - Pentium III Xeon */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_CASE_0x2C
/* 32KB L1, 8-way, 64B line - Pentium 4 */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_CASE_0x39
/* 128KB L2, 4-way, 64B line - Pentium 4 */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_CASE_0x49
/* 4096KB L2, 16-way, 64B line - Xeon DP (non-MP) */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_CASE_0x60
/* 16KB L1, 8-way, 64B line - Core 2 */
__attribute__((target("arch=core2")))
#endif
#ifdef TEST_CASE_0x78
/* 1024KB L2, 4-way, 64B line - AMD K8 */
__attribute__((target("arch=k8")))
#endif
static void specific_cache_test(int descriptor_case) {
    /* Allocate buffers sized to match the target cache configuration */
    size_t buffer_size;
    
    switch (descriptor_case) {
        case 0x0A: buffer_size = 8 * 1024; break;      /* 8KB */
        case 0x0C: buffer_size = 16 * 1024; break;     /* 16KB */
        case 0x21: buffer_size = 256 * 1024; break;    /* 256KB */
        case 0x2C: buffer_size = 32 * 1024; break;     /* 32KB */
        case 0x39: buffer_size = 128 * 1024; break;    /* 128KB */
        case 0x49: buffer_size = 4096 * 1024; break;   /* 4096KB */
        case 0x60: buffer_size = 16 * 1024; break;     /* 16KB */
        case 0x78: buffer_size = 1024 * 1024; break;   /* 1024KB */
        default: buffer_size = 64 * 1024; break;       /* Default 64KB */
    }
    
    int *buffer = aligned_alloc(CACHE_LINE_SIZE, buffer_size);
    if (!buffer) {
        fprintf(stderr, "Allocation failed for case 0x%02x\n", descriptor_case);
        return;
    }
    
    /* Fill buffer with pattern */
    for (size_t i = 0; i < buffer_size / sizeof(int); i++) {
        buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Access pattern designed to exercise specific cache parameters */
    volatile int result = 0;
    const int iterations = 1000;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary access stride based on expected cache line size */
        int stride = (descriptor_case == 0x0A || descriptor_case == 0x0C) ? 8 : 16;
        
        for (size_t i = 0; i < buffer_size / sizeof(int); i += stride) {
            result ^= buffer[i];
            buffer[i] = result;
        }
        MB();
    }
    
    printf("Case 0x%02x test: %d\n", descriptor_case, result);
    free(buffer);
}

/* Main function that runs all applicable tests */
int main(void) {
    printf("Starting cache descriptor coverage tests...\n");
    
    /* Run multi-version test - this should trigger cache detection
     * for multiple CPU targets during compilation */
    multi_version_cache_test();
    
    /* Run specific tests based on compilation flags */
#ifdef TEST_CASE_0x0A
    specific_cache_test(0x0A);
#endif
#ifdef TEST_CASE_0x0C
    specific_cache_test(0x0C);
#endif
#ifdef TEST_CASE_0x0D
    specific_cache_test(0x0D);
#endif
#ifdef TEST_CASE_0x0E
    specific_cache_test(0x0E);
#endif
#ifdef TEST_CASE_0x21
    specific_cache_test(0x21);
#endif
#ifdef TEST_CASE_0x24
    specific_cache_test(0x24);
#endif
#ifdef TEST_CASE_0x2C
    specific_cache_test(0x2C);
#endif
#ifdef TEST_CASE_0x39
    specific_cache_test(0x39);
#endif
#ifdef TEST_CASE_0x3A
    specific_cache_test(0x3A);
#endif
#ifdef TEST_CASE_0x3B
    specific_cache_test(0x3B);
#endif
#ifdef TEST_CASE_0x3C
    specific_cache_test(0x3C);
#endif
#ifdef TEST_CASE_0x3D
    specific_cache_test(0x3D);
#endif
#ifdef TEST_CASE_0x3E
    specific_cache_test(0x3E);
#endif
#ifdef TEST_CASE_0x41
    specific_cache_test(0x41);
#endif
#ifdef TEST_CASE_0x42
    specific_cache_test(0x42);
#endif
#ifdef TEST_CASE_0x43
    specific_cache_test(0x43);
#endif
#ifdef TEST_CASE_0x44
    specific_cache_test(0x44);
#endif
#ifdef TEST_CASE_0x45
    specific_cache_test(0x45);
#endif
#ifdef TEST_CASE_0x48
    specific_cache_test(0x48);
#endif
#ifdef TEST_CASE_0x49
    specific_cache_test(0x49);
#endif
#ifdef TEST_CASE_0x4E
    specific_cache_test(0x4E);
#endif
#ifdef TEST_CASE_0x60
    specific_cache_test(0x60);
#endif
#ifdef TEST_CASE_0x66
    specific_cache_test(0x66);
#endif
#ifdef TEST_CASE_0x67
    specific_cache_test(0x67);
#endif
#ifdef TEST_CASE_0x68
    specific_cache_test(0x68);
#endif
#ifdef TEST_CASE_0x78
    specific_cache_test(0x78);
#endif
#ifdef TEST_CASE_0x79
    specific_cache_test(0x79);
#endif
#ifdef TEST_CASE_0x7A
    specific_cache_test(0x7A);
#endif
#ifdef TEST_CASE_0x7B
    specific_cache_test(0x7B);
#endif
#ifdef TEST_CASE_0x7C
    specific_cache_test(0x7C);
#endif
#ifdef TEST_CASE_0x7D
    specific_cache_test(0x7D);
#endif
#ifdef TEST_CASE_0x7F
    specific_cache_test(0x7F);
#endif
#ifdef TEST_CASE_0x80
    specific_cache_test(0x80);
#endif
#ifdef TEST_CASE_0x82
    specific_cache_test(0x82);
#endif
#ifdef TEST_CASE_0x83
    specific_cache_test(0x83);
#endif
#ifdef TEST_CASE_0x84
    specific_cache_test(0x84);
#endif
#ifdef TEST_CASE_0x85
    specific_cache_test(0x85);
#endif
#ifdef TEST_CASE_0x86
    specific_cache_test(0x86);
#endif
#ifdef TEST_CASE_0x87
    specific_cache_test(0x87);
#endif
    
    printf("Tests completed.\n");
    return 0;
}
