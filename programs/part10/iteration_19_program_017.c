/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our memory accesses */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Memory operation macros to force cache usage */
#define TOUCH_MEMORY(ptr) __asm__ __volatile__("" : : "r"(*(volatile char *)(ptr)) : "memory")

/* Different benchmark functions for different CPU targets */
#ifdef TEST_PENTIUM3
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
__attribute__((target("arch=core2")))
#endif
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Pseudo-random access pattern using linear congruential generator */
    uint32_t seed = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        /* Sequential access (good for prefetch) */
        for (i = 0; i < size; i += 64) { /* 64-byte cache line stride */
            buffer[i] = i ^ seed;
            TOUCH_MEMORY(&buffer[i]);
        }
        
        COMPILER_BARRIER();
        
        /* Random-ish access pattern */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        for (i = 0; i < size; i += 128) {
            size_t idx = (seed + i) % size;
            buffer[idx] = buffer[idx] * 3 + 1;
            TOUCH_MEMORY(&buffer[idx]);
            seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        COMPILER_BARRIER();
        
        /* Gather results to prevent elimination */
        for (i = 0; i < size; i += 256) {
            sink ^= buffer[i];
        }
    }
    
    /* Use sink to prevent dead code elimination */
    if (sink == 0x12345678) {
        printf("Impossible condition\n");
    }
}

/* Specialized functions for specific cache descriptor cases */
#ifdef TEST_CASE_0x0A
/* Target: Intel Pentium III (Coppermine) - L1: 8KB, 2-way, 32B line */
__attribute__((target("arch=pentium3")))
static void test_case_0x0a(void) {
    printf("Testing for CPUID descriptor 0x0A (Pentium III style)\n");
    size_t l1_size = 8 * 1024; /* 8KB */
    size_t l2_size = 256 * 1024; /* 256KB typical for P3 */
    
    int *buffer1 = malloc(l1_size);
    int *buffer2 = malloc(l2_size * 2); /* Larger than L2 */
    
    if (buffer1 && buffer2) {
        cache_thrash_benchmark(buffer1, l1_size / sizeof(int), 1000);
        cache_thrash_benchmark(buffer2, (l2_size * 2) / sizeof(int), 100);
    }
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef TEST_CASE_0x0C
/* Target: Intel Pentium III (Tualatin) - L1: 16KB, 4-way, 32B line */
__attribute__((target("arch=pentium3")))
static void test_case_0x0c(void) {
    printf("Testing for CPUID descriptor 0x0C (Pentium III Tualatin)\n");
    size_t l1_size = 16 * 1024;
    int *buffer = malloc(l1_size * 4);
    if (buffer) {
        cache_thrash_benchmark(buffer, (l1_size * 4) / sizeof(int), 500);
    }
    free(buffer);
}
#endif

#ifdef TEST_CASE_0x21
/* Target: Intel Pentium 4 - L2: 256KB, 8-way, 64B line */
__attribute__((target("arch=pentium4")))
static void test_case_0x21(void) {
    printf("Testing for CPUID descriptor 0x21 (Pentium 4 style)\n");
    size_t l2_size = 256 * 1024;
    int *buffer = malloc(l2_size * 3);
    if (buffer) {
        cache_thrash_benchmark(buffer, (l2_size * 3) / sizeof(int), 200);
    }
    free(buffer);
}
#endif

#ifdef TEST_CASE_0x24
/* Target: Intel Xeon MP (Foster MP) - L2: 1MB, 16-way, 64B line */
__attribute__((target("arch=pentium4")))
static void test_case_0x24(void) {
    printf("Testing for CPUID descriptor 0x24 (Xeon MP style)\n");
    size_t l2_size = 1024 * 1024;
    int *buffer = malloc(l2_size * 2);
    if (buffer) {
        cache_thrash_benchmark(buffer, (l2_size * 2) / sizeof(int), 100);
    }
    free(buffer);
}
#endif

#ifdef TEST_CASE_0x49
/* Target: Intel Xeon DP (Nocona) - L2: 4MB, 16-way, 64B line (non-MP) */
__attribute__((target("arch=nocona")))
static void test_case_0x49(void) {
    printf("Testing for CPUID descriptor 0x49 (Xeon DP/Nocona, non-MP)\n");
    /* This should trigger case 0x49 without xeon_mp flag set */
    size_t l2_size = 4096 * 1024;
    int *buffer = malloc(l2_size);
    if (buffer) {
        /* Large memory access pattern to engage L2 cache logic */
        for (size_t i = 0; i < l2_size / sizeof(int); i += 64) {
            buffer[i] = i;
            TOUCH_MEMORY(&buffer[i]);
        }
        cache_thrash_benchmark(buffer, l2_size / sizeof(int), 50);
    }
    free(buffer);
}
#endif

#ifdef TEST_CASE_0x60
/* Target: AMD K8 (Athlon 64) - L1: 16KB, 8-way, 64B line */
__attribute__((target("arch=k8")))
static void test_case_0x60(void) {
    printf("Testing for CPUID descriptor 0x60 (AMD K8 style)\n");
    size_t l1_size = 16 * 1024;
    size_t l2_size = 512 * 1024; /* Typical K8 L2 */
    
    int *buffer1 = malloc(l1_size * 2);
    int *buffer2 = malloc(l2_size * 2);
    
    if (buffer1 && buffer2) {
        cache_thrash_benchmark(buffer1, (l1_size * 2) / sizeof(int), 300);
        cache_thrash_benchmark(buffer2, (l2_size * 2) / sizeof(int), 150);
    }
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef TEST_CASE_0x78_0x87
/* Target: Various cache configurations in 0x78-0x87 range */
__attribute__((target("arch=core2")))
static void test_case_0x78_0x87(void) {
    printf("Testing for CPUID descriptors 0x78-0x87 (Core 2 style)\n");
    /* Core 2 has various cache configurations that might trigger these */
    size_t sizes[] = {128*1024, 256*1024, 512*1024, 1024*1024, 2048*1024};
    
    for (int i = 0; i < 5; i++) {
        int *buffer = malloc(sizes[i] * 2);
        if (buffer) {
            cache_thrash_benchmark(buffer, (sizes[i] * 2) / sizeof(int), 50);
            free(buffer);
        }
    }
}
#endif

/* Main function with multi-versioning support */
#ifdef USE_MULTI_VERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
int main(int argc, char **argv) {
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Allocate buffers larger than typical caches to ensure
       driver considers cache sizes during optimization */
    const size_t huge_size = 8 * 1024 * 1024; /* 8MB */
    int *huge_buffer = malloc(huge_size);
    volatile int result = 0;
    
    if (!huge_buffer) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (size_t i = 0; i < huge_size / sizeof(int); i++) {
        huge_buffer[i] = i ^ 0x55555555;
    }
    
    /* Execute architecture-specific tests */
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
    
#ifdef TEST_CASE_0x49
    test_case_0x49();
#endif
    
#ifdef TEST_CASE_0x60
    test_case_0x60();
#endif
    
#ifdef TEST_CASE_0x78_0x87
    test_case_0x78_0x87();
#endif
    
    /* Main cache-thrashing benchmark */
    printf("\nRunning main cache benchmark...\n");
    
    /* Complex access pattern to defeat prefetching */
    uint32_t seed = time(NULL);
    const int iterations = 100;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix of sequential and random access */
        for (size_t i = 0; i < huge_size / sizeof(int); i += 64) {
            huge_buffer[i] = huge_buffer[i] * 3 + 1;
        }
        
        COMPILER_BARRIER();
        
        /* Strided access */
        for (size_t i = 0; i < huge_size / sizeof(int); i += 128) {
            result ^= huge_buffer[i];
        }
        
        COMPILER_BARRIER();
        
        /* Pseudo-random access */
        for (size_t i = 0; i < 10000; i++) {
            seed = seed * 1103515245 + 12345;
            size_t idx = seed % (huge_size / sizeof(int));
            huge_buffer[idx] = seed;
            TOUCH_MEMORY(&huge_buffer[idx]);
        }
    }
    
    /* Use result to prevent elimination */
    printf("Benchmark result: %d\n", result);
    
    free(huge_buffer);
    
    return 0;
}
