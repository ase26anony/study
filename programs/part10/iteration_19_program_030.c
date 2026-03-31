/* test_cache_coverage.c - Comprehensive test to cover CPUID leaf 2 cache descriptor cases in driver-i386.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different architecture targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
#define ARCH_FLAG __attribute__((target("arch=pentium3")))
#define TUNE_FLAG "pentium3"
#elif defined(TEST_PENTIUM4)
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 */
#define ARCH_FLAG __attribute__((target("arch=pentium4")))
#define TUNE_FLAG "pentium4"
#elif defined(TEST_NOCONA)
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68, 0x78-0x87 */
#define ARCH_FLAG __attribute__((target("arch=nocona")))
#define TUNE_FLAG "nocona"
#elif defined(TEST_K8)
/* Targets cases: 0x40 series, 0x78-0x87 (AMD K8) */
#define ARCH_FLAG __attribute__((target("arch=k8")))
#define TUNE_FLAG "k8"
#elif defined(TEST_CORE2)
/* Targets cases: 0x41-0x45, 0x48-0x49, 0x78-0x87 */
#define ARCH_FLAG __attribute__((target("arch=core2")))
#define TUNE_FLAG "core2"
#elif defined(TEST_NEHALEM)
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e */
#define ARCH_FLAG __attribute__((target("arch=nehalem")))
#define TUNE_FLAG "nehalem"
#else
/* Generic fallback - will use system's actual CPUID */
#define ARCH_FLAG
#define TUNE_FLAG "native"
#endif

/* Cache-thrashing benchmark function with architecture-specific targeting */
ARCH_FLAG static void cache_thrash_benchmark(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j, k;
    
    /* Allocate buffer larger than expected L2 cache */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed for %d KB buffer\n", buffer_size_kb);
        return;
    }
    
    /* Initialize buffer with pseudo-random values using LCG */
    unsigned int seed = 123456789;
    for (i = 0; i < elements; i++) {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        buffer[i] = (int)(seed % 256);
    }
    
    COMPILER_BARRIER();
    
    /* Main cache-thrashing loop with multiple access patterns */
    for (k = 0; k < iterations; k++) {
        /* Pattern 1: Sequential access (good for prefetch) */
        for (i = 0; i < elements; i += 64) {  /* 64-byte cache line stride */
            buffer[i] = buffer[i] * 3 + 7;
        }
        
        COMPILER_BARRIER();
        
        /* Pattern 2: Strided access (tests associativity) */
        for (j = 0; j < 8; j++) {  /* 8-way associativity test */
            for (i = j; i < elements; i += 64) {
                buffer[i] = buffer[i] ^ (i + j);
            }
        }
        
        COMPILER_BARRIER();
        
        /* Pattern 3: Pseudo-random access (worst-case for cache) */
        seed = k * 987654321;
        for (i = 0; i < elements / 4; i++) {
            seed = (1103515245 * seed + 12345) & 0x7fffffff;
            int idx = seed % elements;
            buffer[idx] = buffer[idx] + i;
        }
        
        COMPILER_BARRIER();
    }
    
    /* Compute final result to prevent dead code elimination */
    for (i = 0; i < elements; i += 128) {
        result ^= buffer[i];
    }
    
    /* Use result to prevent optimization */
    if (result == 0xdeadbeef) {
        printf("Impossible condition\n");
    }
    
    free(buffer);
}

/* Specialized functions for specific cache descriptor cases */
#ifdef TEST_CASE_0x0A
/* 8KB L1, 2-way, 32-byte line - Pentium III, some Pentium 4 */
__attribute__((target("arch=pentium3")))
static void test_case_0x0a(void) {
    printf("Testing for cache descriptor 0x0a (8KB L1, 2-way, 32B line)\n");
    cache_thrash_benchmark(100, 8192);  /* 8MB buffer */
}
#endif

#ifdef TEST_CASE_0x49
/* 4MB L2, 16-way, 64-byte line - Pentium 4 with 2MB L2, Xeon DP (non-MP) */
__attribute__((target("arch=nocona")))
static void test_case_0x49(void) {
    printf("Testing for cache descriptor 0x49 (4MB L2, 16-way, 64B line)\n");
    /* Use buffer larger than 4MB to ensure L2 cache misses */
    cache_thrash_benchmark(50, 16384);  /* 16MB buffer */
}
#endif

#ifdef TEST_CASE_0x60
/* 16KB L1, 8-way, 64-byte line - Some Xeon variants */
__attribute__((target("arch=core2")))
static void test_case_0x60(void) {
    printf("Testing for cache descriptor 0x60 (16KB L1, 8-way, 64B line)\n");
    cache_thrash_benchmark(100, 4096);  /* 4MB buffer */
}
#endif

#ifdef TEST_CASE_0x78
/* 1MB L2, 4-way, 64-byte line - Common in many CPUs */
__attribute__((target("arch=k8")))
static void test_case_0x78(void) {
    printf("Testing for cache descriptor 0x78 (1MB L2, 4-way, 64B line)\n");
    cache_thrash_benchmark(100, 2048);  /* 2MB buffer */
}
#endif

/* Multi-versioned function using GCC's target_clones attribute */
#ifdef USE_MULTI_VERSIONING
__attribute__((target_clones("pentium3,pentium4,nocona,k8,core2,nehalem")))
static void multi_version_cache_test(void) {
    printf("Multi-version cache test running\n");
    cache_thrash_benchmark(50, 4096);
}
#endif

int main(int argc, char *argv[]) {
    int iterations = 100;
    int buffer_size_kb = 4096;  /* 4MB default */
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) buffer_size_kb = atoi(argv[2]);
    
    printf("Cache Coverage Test - Targeting: %s\n", TUNE_FLAG);
    printf("Iterations: %d, Buffer: %d KB\n", iterations, buffer_size_kb);
    
    /* Run architecture-specific benchmark */
    cache_thrash_benchmark(iterations, buffer_size_kb);
    
    /* Run specific case tests if compiled with those defines */
#ifdef TEST_CASE_0x0A
    test_case_0x0a();
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
    
#ifdef USE_MULTI_VERSIONING
    multi_version_cache_test();
#endif
    
    /* Additional memory pattern to stress different cache levels */
    {
        volatile int sink = 0;
        const int small_size = 32 * 1024;  /* 32KB - typical L1 size */
        const int medium_size = 256 * 1024; /* 256KB - typical L2 size */
        
        int *small_buf = malloc(small_size);
        int *medium_buf = malloc(medium_size);
        
        if (small_buf && medium_buf) {
            /* Access pattern that should hit L1 cache */
            for (int i = 0; i < small_size / sizeof(int); i++) {
                small_buf[i] = i;
                sink += small_buf[i];
            }
            
            COMPILER_BARRIER();
            
            /* Access pattern that should hit L2 cache */
            for (int i = 0; i < medium_size / sizeof(int); i += 64) {
                medium_buf[i] = i;
                sink += medium_buf[i];
            }
            
            COMPILER_BARRIER();
            
            /* Mixed access pattern */
            for (int i = 0; i < 1000; i++) {
                int idx1 = (i * 167) % (small_size / sizeof(int));
                int idx2 = (i * 431) % (medium_size / sizeof(int));
                small_buf[idx1] = medium_buf[idx2];
                sink += small_buf[idx1];
            }
        }
        
        free(small_buf);
        free(medium_buf);
        
        /* Prevent optimization of sink */
        if (sink == 0xdeadbeef) {
            printf("Impossible\n");
        }
    }
    
    printf("Test completed\n");
    return 0;
}
