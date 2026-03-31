/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */

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
#endif

/* Cache-thrashing benchmark function */
static void cache_benchmark(size_t buffer_size, int iterations) {
    volatile int result = 0;
    
    /* Allocate buffers larger than typical L2 cache */
    int *buffer1 = (int*)malloc(buffer_size * sizeof(int));
    int *buffer2 = (int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        free(buffer1);
        free(buffer2);
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < buffer_size; i++) {
        seed = seed * 1103515245 + 12345;
        buffer1[i] = (int)(seed & 0x7FFFFFFF);
        buffer2[i] = (int)((seed >> 16) & 0x7FFF);
    }
    
    COMPILER_BARRIER();
    
    /* Main cache-access pattern */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pattern 1: Sequential access with stride */
        for (size_t i = 0; i < buffer_size; i += 64) {
            buffer1[i] = buffer1[i] * 3 + 1;
        }
        
        COMPILER_BARRIER();
        
        /* Pattern 2: Pseudo-random access to defeat prefetch */
        seed = iter;
        for (size_t j = 0; j < buffer_size / 4; j++) {
            seed = seed * 1664525 + 1013904223;
            size_t idx = seed % buffer_size;
            buffer2[idx] = buffer2[idx] ^ buffer1[j % buffer_size];
        }
        
        COMPILER_BARRIER();
        
        /* Pattern 3: Matrix-style access pattern */
        size_t block_size = 256;  /* Approximately 1KB block */
        for (size_t block = 0; block < buffer_size; block += block_size) {
            size_t end = (block + block_size < buffer_size) ? block + block_size : buffer_size;
            for (size_t i = block; i < end; i++) {
                for (size_t j = i + 1; j < end && j < i + 32; j++) {
                    buffer1[i] += buffer2[j];
                }
            }
        }
    }
    
    COMPILER_BARRIER();
    
    /* Final computation to prevent dead code elimination */
    for (size_t i = 0; i < buffer_size; i += 128) {
        result ^= buffer1[i] + buffer2[i];
    }
    
    /* Volatile use of result */
    if (result == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    free(buffer1);
    free(buffer2);
}

/* Specialized functions for different cache descriptor cases */
#ifdef COVER_CASE_0x0A
/* Targets: Intel Pentium III (Coppermine) - 8KB L1D, 2-way */
__attribute__((target("arch=pentium3")))
void test_case_0x0a(void) {
    printf("Testing for CPUID descriptor 0x0A (8KB L1, 2-way)\n");
    cache_benchmark(16 * 1024, 1000);  /* 64KB working set */
}
#endif

#ifdef COVER_CASE_0x0C
/* Targets: Intel Pentium III (Coppermine) - 16KB L1D, 4-way */
__attribute__((target("arch=pentium3")))
void test_case_0x0c(void) {
    printf("Testing for CPUID descriptor 0x0C (16KB L1, 4-way)\n");
    cache_benchmark(32 * 1024, 1000);
}
#endif

#ifdef COVER_CASE_0x21
/* Targets: Intel Pentium 4 (Willamette) - 256KB L2, 8-way */
__attribute__((target("arch=pentium4")))
void test_case_0x21(void) {
    printf("Testing for CPUID descriptor 0x21 (256KB L2, 8-way)\n");
    cache_benchmark(512 * 1024, 500);  /* 2MB working set */
}
#endif

#ifdef COVER_CASE_0x24
/* Targets: Intel Pentium 4 (Northwood) - 1MB L2, 16-way */
__attribute__((target("arch=pentium4")))
void test_case_0x24(void) {
    printf("Testing for CPUID descriptor 0x24 (1MB L2, 16-way)\n");
    cache_benchmark(2 * 1024 * 1024, 200);  /* 8MB working set */
}
#endif

#ifdef COVER_CASE_0x49
/* Targets: Intel Xeon DP (Nocona) - 4MB L2, 16-way (non-MP) */
__attribute__((target("arch=nocona")))
void test_case_0x49(void) {
    printf("Testing for CPUID descriptor 0x49 (4MB L2, 16-way, non-Xeon-MP)\n");
    cache_benchmark(8 * 1024 * 1024, 100);  /* 32MB working set */
}
#endif

#ifdef COVER_CASE_0x60
/* Targets: AMD K8 (Athlon 64) - 16KB L1D, 8-way */
__attribute__((target("arch=k8")))
void test_case_0x60(void) {
    printf("Testing for CPUID descriptor 0x60 (16KB L1, 8-way)\n");
    cache_benchmark(64 * 1024, 800);
}
#endif

#ifdef COVER_CASE_0x78
/* Targets: AMD K8 (Athlon 64) - 1MB L2, 4-way */
__attribute__((target("arch=k8")))
void test_case_0x78(void) {
    printf("Testing for CPUID descriptor 0x78 (1MB L2, 4-way)\n");
    cache_benchmark(2 * 1024 * 1024, 200);
}
#endif

#ifdef COVER_CASE_0x86
/* Targets: Intel Core 2 (Conroe) - 512KB L2, 4-way */
__attribute__((target("arch=core2")))
void test_case_0x86(void) {
    printf("Testing for CPUID descriptor 0x86 (512KB L2, 4-way)\n");
    cache_benchmark(1 * 1024 * 1024, 300);
}
#endif

/* Multi-version function using target_clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
void multiversion_cache_test(void) {
    static int call_count = 0;
    call_count++;
    
    /* Different working set sizes for different architectures */
    size_t buffer_size = 256 * 1024;  /* Default 1MB */
    
    /* This will cause GCC to consider cache parameters for each target */
    cache_benchmark(buffer_size, 100);
    
    if (call_count > 10) {
        printf("Multiversion function called %d times\n", call_count);
    }
}
#endif

int main(int argc, char **argv) {
    printf("Cache Coverage Test Program\n");
    printf("===========================\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Call architecture-specific tests based on compile-time defines */
#ifdef COVER_CASE_0x0A
    test_case_0x0a();
#endif
    
#ifdef COVER_CASE_0x0C
    test_case_0x0c();
#endif
    
#ifdef COVER_CASE_0x21
    test_case_0x21();
#endif
    
#ifdef COVER_CASE_0x24
    test_case_0x24();
#endif
    
#ifdef COVER_CASE_0x49
    test_case_0x49();
#endif
    
#ifdef COVER_CASE_0x60
    test_case_0x60();
#endif
    
#ifdef COVER_CASE_0x78
    test_case_0x78();
#endif
    
#ifdef COVER_CASE_0x86
    test_case_0x86();
#endif
    
#ifdef USE_MULTIVERSIONING
    /* Call multiversion function multiple times */
    for (int i = 0; i < 5; i++) {
        multiversion_cache_test();
    }
#endif
    
    /* Generic cache test that should work on any architecture */
    printf("\nRunning generic cache test...\n");
    cache_benchmark(1 * 1024 * 1024, 50);  /* 4MB working set */
    
    printf("\nTest completed successfully.\n");
    return 0;
}
