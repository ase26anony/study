/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
/* Compile with different -D flags and -march options to cover specific cases */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Function attributes for targeting specific architectures */
#ifdef TARGET_PENTIUM3
#define TARGET_ATTR __attribute__((target("arch=pentium3")))
#elif defined(TARGET_PENTIUM4)
#define TARGET_ATTR __attribute__((target("arch=pentium4")))
#elif defined(TARGET_NOCONA)
#define TARGET_ATTR __attribute__((target("arch=nocona")))
#elif defined(TARGET_K8)
#define TARGET_ATTR __attribute__((target("arch=k8")))
#elif defined(TARGET_CORE2)
#define TARGET_ATTR __attribute__((target("arch=core2")))
#elif defined(TARGET_NEHALEM)
#define TARGET_ATTR __attribute__((target("arch=nehalem")))
#else
#define TARGET_ATTR
#endif

/* Cache thrashing benchmark - different versions for different architectures */
TARGET_ATTR
static void cache_thrash_benchmark(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than L2 cache to ensure cache misses */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed for %d KB buffer\n", buffer_size_kb);
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    unsigned int seed = 123456789;
    for (i = 0; i < elements; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    MEMORY_BARRIER();
    
    /* Main benchmark: access pattern designed to stress cache hierarchy */
    for (j = 0; j < iterations; j++) {
        /* Strided access pattern - varies with architecture */
        #if defined(TARGET_PENTIUM3) || defined(TARGET_PENTIUM4)
        /* Pentium III/IV specific pattern */
        int stride = 16; /* Matches typical cache line sizes */
        for (i = 0; i < elements; i += stride) {
            buffer[i] = buffer[i] * 3 + 1;
        }
        #elif defined(TARGET_K8)
        /* AMD K8 specific pattern */
        int stride = 32; /* Larger stride for AMD */
        for (i = 0; i < elements; i += stride) {
            buffer[(i * 17) % elements] ^= buffer[i];
        }
        #else
        /* Generic pattern */
        for (i = 0; i < elements; i += 64) {
            buffer[i] = buffer[i] + buffer[(i + 32) % elements];
        }
        #endif
        
        MEMORY_BARRIER();
    }
    
    /* Prevent dead code elimination */
    for (i = 0; i < elements; i += 1024) {
        result ^= buffer[i];
    }
    
    MEMORY_BARRIER();
    
    /* Use result to prevent optimization */
    if (result == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    free(buffer);
}

/* Multi-version function using target_clones attribute */
#ifdef USE_MULTIVERSION
__attribute__((target_clones("default,arch=pentium3,arch=pentium4,arch=nocona,arch=k8,arch=core2")))
#endif
static void multi_version_benchmark(void) {
    /* This function will be compiled for multiple targets */
    cache_thrash_benchmark(100, 4096); /* 4MB buffer */
}

/* Architecture-specific test functions */
#ifdef TEST_CASE_0x0A
/* Targets: Intel Pentium III (Coppermine) - L1: 8KB, 2-way, 32B line */
static void test_case_0x0a(void) {
    printf("Testing for CPUID descriptor 0x0A (Pentium III Coppermine)\n");
    cache_thrash_benchmark(500, 512); /* 512KB buffer */
}
#endif

#ifdef TEST_CASE_0x0C
/* Targets: Intel Pentium III (Coppermine) - L1: 16KB, 4-way, 32B line */
static void test_case_0x0c(void) {
    printf("Testing for CPUID descriptor 0x0C (Pentium III Coppermine)\n");
    cache_thrash_benchmark(500, 512);
}
#endif

#ifdef TEST_CASE_0x21
/* Targets: Intel Pentium 4 - L2: 256KB, 8-way, 64B line */
static void test_case_0x21(void) {
    printf("Testing for CPUID descriptor 0x21 (Pentium 4)\n");
    cache_thrash_benchmark(300, 1024); /* 1MB buffer */
}
#endif

#ifdef TEST_CASE_0x24
/* Targets: Intel Pentium 4 - L2: 1MB, 16-way, 64B line */
static void test_case_0x24(void) {
    printf("Testing for CPUID descriptor 0x24 (Pentium 4 with 1MB L2)\n");
    cache_thrash_benchmark(300, 2048); /* 2MB buffer */
}
#endif

#ifdef TEST_CASE_0x2C
/* Targets: Intel Pentium 4 (Northwood) - L1: 32KB, 8-way, 64B line */
static void test_case_0x2c(void) {
    printf("Testing for CPUID descriptor 0x2C (Pentium 4 Northwood)\n");
    cache_thrash_benchmark(400, 1024);
}
#endif

#ifdef TEST_CASE_0x49
/* Targets: Intel Xeon DP (not MP) - L2: 4MB, 16-way, 64B line */
/* Need to ensure xeon_mp flag is false */
static void test_case_0x49(void) {
    printf("Testing for CPUID descriptor 0x49 (Xeon DP, not MP)\n");
    /* Use larger buffer to stress L2 cache */
    cache_thrash_benchmark(200, 8192); /* 8MB buffer */
}
#endif

#ifdef TEST_CASE_0x60
/* Targets: Intel Core/Core 2 - L1: 16KB, 8-way, 64B line */
static void test_case_0x60(void) {
    printf("Testing for CPUID descriptor 0x60 (Core/Core 2)\n");
    cache_thrash_benchmark(600, 2048);
}
#endif

#ifdef TEST_CASE_0x78
/* Targets: AMD K8 - L2: 1MB, 4-way, 64B line */
static void test_case_0x78(void) {
    printf("Testing for CPUID descriptor 0x78 (AMD K8)\n");
    cache_thrash_benchmark(500, 2048);
}
#endif

#ifdef TEST_CASE_0x7A
/* Targets: AMD K8/K10 - L2: 256KB, 8-way, 64B line */
static void test_case_0x7a(void) {
    printf("Testing for CPUID descriptor 0x7A (AMD K8/K10)\n");
    cache_thrash_benchmark(500, 1024);
}
#endif

#ifdef TEST_CASE_0x82
/* Targets: Various - L2: 256KB, 8-way, 32B line */
static void test_case_0x82(void) {
    printf("Testing for CPUID descriptor 0x82\n");
    cache_thrash_benchmark(400, 1024);
}
#endif

#ifdef TEST_CASE_0x87
/* Targets: Various - L2: 1MB, 8-way, 64B line */
static void test_case_0x87(void) {
    printf("Testing for CPUID descriptor 0x87\n");
    cache_thrash_benchmark(400, 2048);
}
#endif

int main(int argc, char *argv[]) {
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Run architecture-specific tests based on compile-time defines */
    
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
    
#ifdef TEST_CASE_0x87
    test_case_0x87();
#endif
    
    /* Run multi-version benchmark if enabled */
#ifdef USE_MULTIVERSION
    printf("\nRunning multi-version benchmark...\n");
    multi_version_benchmark();
#endif
    
    /* Generic cache benchmark */
    printf("\nRunning generic cache benchmark...\n");
    cache_thrash_benchmark(100, 4096);
    
    printf("\nTest completed.\n");
    return 0;
}
