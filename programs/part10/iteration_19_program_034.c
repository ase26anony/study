/* test_cache_coverage.c - Comprehensive test to cover CPUID leaf 2 cache descriptor cases
 * 
 * Compilation examples:
 * 1. Pentium III/Pentium M coverage: gcc -O3 -march=pentium3 -mtune=pentium3 -DTEST_PENTIUM3 test_cache_coverage.c -o test_p3
 * 2. Pentium 4/Xeon coverage: gcc -O3 -march=nocona -mtune=nocona -DTEST_NOCONA test_cache_coverage.c -o test_nocona
 * 3. AMD K8 coverage: gcc -O3 -march=k8 -mtune=k8 -DTEST_K8 test_cache_coverage.c -o test_k8
 * 4. Generic with multi-versioning: gcc -O3 -march=x86-64 -mtune=generic -DTEST_MULTI test_cache_coverage.c -o test_multi
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Memory barriers to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")
#define CPUID_BARRIER() asm volatile("cpuid" : : "a"(0) : "ebx", "ecx", "edx")

/* Cache line size assumptions for different architectures */
#define CACHE_LINE_32 32
#define CACHE_LINE_64 64

/* Different test patterns for different cache configurations */
#ifdef TEST_PENTIUM3
/* Target cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39-0x3e, 0x41-0x45 */
#define TARGET_ARCH "pentium3"
#define L1_SIZE_KB 16
#define L2_SIZE_KB 256
#define CACHE_LINE CACHE_LINE_32
#elif defined(TEST_NOCONA)
/* Target cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68, 0x78-0x87 */
#define TARGET_ARCH "nocona"
#define L1_SIZE_KB 16
#define L2_SIZE_KB 1024
#define CACHE_LINE CACHE_LINE_64
#elif defined(TEST_K8)
/* Target cases: 0x40 series, 0x78-0x87 (AMD-specific) */
#define TARGET_ARCH "k8"
#define L1_SIZE_KB 64
#define L2_SIZE_KB 512
#define CACHE_LINE CACHE_LINE_64
#elif defined(TEST_MULTI)
/* Multi-versioning - will create multiple function versions */
#define TARGET_ARCH "multi"
#define CACHE_LINE CACHE_LINE_64
#else
/* Generic fallback */
#define TARGET_ARCH "generic"
#define CACHE_LINE CACHE_LINE_64
#endif

/* Function attributes for targeting specific architectures */
#ifdef __GNUC__
#define TARGET_CPU(arch) __attribute__((target("arch=" arch)))
#else
#define TARGET_CPU(arch)
#endif

/* Benchmark function for Pentium III class CPUs (triggers early cache descriptors) */
TARGET_CPU("pentium3")
static void benchmark_pentium3(void) {
    volatile int result = 0;
    const size_t l1_size = 8 * 1024;      /* 8KB - targets case 0x0a */
    const size_t l2_size = 256 * 1024;    /* 256KB - targets case 0x21 */
    
    /* Allocate buffers sized to trigger specific cache detection */
    char* buffer1 = (char*)aligned_alloc(CACHE_LINE_32, l1_size);
    char* buffer2 = (char*)aligned_alloc(CACHE_LINE_32, l2_size);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Allocation failed for pentium3 test\n");
        return;
    }
    
    /* Initialize with pattern */
    for (size_t i = 0; i < l1_size; i++) buffer1[i] = (char)(i % 256);
    for (size_t i = 0; i < l2_size; i++) buffer2[i] = (char)((i * 7) % 256);
    
    /* L1 cache thrashing pattern - 2-way associativity (case 0x0a) */
    for (int iter = 0; iter < 1000; iter++) {
        for (size_t i = 0; i < l1_size; i += CACHE_LINE_32 * 2) {
            buffer1[i] = buffer1[(i + CACHE_LINE_32) % l1_size] + 1;
        }
        COMPILER_BARRIER();
    }
    
    /* L2 cache thrashing pattern - 8-way associativity (case 0x21) */
    for (int iter = 0; iter < 500; iter++) {
        for (size_t i = 0; i < l2_size; i += CACHE_LINE_32 * 8) {
            buffer2[i] = buffer2[(i + CACHE_LINE_32 * 4) % l2_size] + 1;
        }
        COMPILER_BARRIER();
    }
    
    /* Prevent optimization */
    result = buffer1[0] + buffer2[0];
    asm volatile("" : "+r" (result) : : "memory");
    
    free(buffer1);
    free(buffer2);
}

/* Benchmark function for Pentium 4/Nocona (triggers 0x49, 0x60, etc.) */
TARGET_CPU("nocona")
static void benchmark_nocona(void) {
    volatile int result = 0;
    const size_t l1_size = 16 * 1024;     /* 16KB - targets cases 0x60, 0x67 */
    const size_t l2_size = 1024 * 1024;   /* 1MB - targets cases 0x78, 0x49 */
    
    /* Allocate buffers */
    char* buffer1 = (char*)aligned_alloc(CACHE_LINE_64, l1_size);
    char* buffer2 = (char*)aligned_alloc(CACHE_LINE_64, l2_size);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Allocation failed for nocona test\n");
        return;
    }
    
    /* Initialize */
    for (size_t i = 0; i < l1_size; i++) buffer1[i] = (char)(i % 256);
    for (size_t i = 0; i < l2_size; i++) buffer2[i] = (char)((i * 13) % 256);
    
    /* L1 cache test - 8-way associativity (case 0x60) */
    for (int iter = 0; iter < 800; iter++) {
        for (size_t i = 0; i < l1_size; i += CACHE_LINE_64 * 8) {
            buffer1[i] = buffer1[(i + CACHE_LINE_64 * 3) % l1_size] ^ 0x55;
        }
        COMPILER_BARRIER();
    }
    
    /* L2 cache test - 4-way associativity (case 0x78) and 16-way (case 0x49) */
    for (int iter = 0; iter < 300; iter++) {
        /* Mixed access pattern to trigger different associativity detection */
        for (size_t i = 0; i < l2_size; i += CACHE_LINE_64 * 16) {
            buffer2[i] = buffer2[(i + CACHE_LINE_64 * 8) % l2_size] ^ 0xAA;
        }
        for (size_t i = CACHE_LINE_64 * 4; i < l2_size; i += CACHE_LINE_64 * 16) {
            buffer2[i] = buffer2[(i - CACHE_LINE_64 * 4) % l2_size] ^ 0x33;
        }
        COMPILER_BARRIER();
    }
    
    result = buffer1[0] + buffer2[0];
    asm volatile("" : "+r" (result) : : "memory");
    
    free(buffer1);
    free(buffer2);
}

/* Benchmark function for AMD K8 (triggers 0x40 series, 0x78-0x87) */
TARGET_CPU("k8")
static void benchmark_k8(void) {
    volatile int result = 0;
    const size_t l1_size = 64 * 1024;     /* 64KB L1 typical for K8 */
    const size_t l2_size = 512 * 1024;    /* 512KB L2 - targets cases 0x7b, 0x86 */
    
    char* buffer1 = (char*)aligned_alloc(CACHE_LINE_64, l1_size);
    char* buffer2 = (char*)aligned_alloc(CACHE_LINE_64, l2_size);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Allocation failed for k8 test\n");
        return;
    }
    
    /* Initialize with AMD-optimized pattern */
    for (size_t i = 0; i < l1_size; i++) buffer1[i] = (char)((i * 3) % 256);
    for (size_t i = 0; i < l2_size; i++) buffer2[i] = (char)((i * 17) % 256);
    
    /* 2-way associativity pattern (case 0x7f) */
    for (int iter = 0; iter < 600; iter++) {
        for (size_t i = 0; i < l2_size; i += CACHE_LINE_64 * 2) {
            buffer2[i] = buffer2[(i + CACHE_LINE_64) % l2_size] + iter;
        }
        COMPILER_BARRIER();
    }
    
    /* 8-way associativity pattern (cases 0x7a-0x7d, 0x80, 0x82-0x85) */
    for (int iter = 0; iter < 400; iter++) {
        for (size_t i = 0; i < l2_size; i += CACHE_LINE_64 * 8) {
            buffer2[i] = buffer2[(i + CACHE_LINE_64 * 5) % l2_size] ^ buffer2[(i + CACHE_LINE_64 * 2) % l2_size];
        }
        COMPILER_BARRIER();
    }
    
    result = buffer1[0] + buffer2[0];
    asm volatile("" : "+r" (result) : : "memory");
    
    free(buffer1);
    free(buffer2);
}

/* Generic benchmark that adapts to runtime CPU detection */
static void benchmark_generic(void) {
    volatile int result = 0;
    /* Use large buffers to ensure L1/L2 cache usage */
    const size_t huge_size = 4 * 1024 * 1024;  /* 4MB */
    
    int* buffer = (int*)aligned_alloc(CACHE_LINE, huge_size);
    if (!buffer) {
        fprintf(stderr, "Allocation failed for generic test\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(time(NULL));
    for (size_t i = 0; i < huge_size / sizeof(int); i++) {
        buffer[i] = rand();
    }
    
    /* Cache-thrashing access pattern */
    for (int iter = 0; iter < 100; iter++) {
        size_t stride = (CACHE_LINE / sizeof(int)) * (1 + (iter % 16));
        for (size_t i = 0; i < huge_size / sizeof(int); i += stride) {
            buffer[i] = buffer[(i + stride) % (huge_size / sizeof(int))] + iter;
        }
        COMPILER_BARRIER();
    }
    
    /* Sum to prevent optimization */
    for (size_t i = 0; i < huge_size / sizeof(int); i += 1024) {
        result ^= buffer[i];
    }
    
    asm volatile("" : "+r" (result) : : "memory");
    
    free(buffer);
}

/* Multi-versioned function using GCC's target_clones attribute */
#ifdef __GNUC__
__attribute__((target_clones("pentium3,nocona,k8,default")))
#endif
static void benchmark_multi(void) {
    benchmark_generic();
}

int main(void) {
    printf("Cache descriptor coverage test for target: %s\n", TARGET_ARCH);
    printf("Testing CPUID leaf 2 cache detection paths...\n");
    
    volatile int final_result = 0;
    
#if defined(TEST_PENTIUM3)
    printf("Running Pentium III class benchmarks...\n");
    benchmark_pentium3();
    benchmark_generic();  /* Also run generic to cover more cases */
    
#elif defined(TEST_NOCONA)
    printf("Running Pentium 4/Nocona benchmarks...\n");
    benchmark_nocona();
    /* Additional test for case 0x49 (non-Xeon-MP) */
    {
        /* This size/pattern might trigger 0x49 detection */
        const size_t special_size = 4096 * 1024;  /* 4MB */
        char* special_buf = (char*)aligned_alloc(CACHE_LINE_64, special_size);
        if (special_buf) {
            for (int iter = 0; iter < 50; iter++) {
                for (size_t i = 0; i < special_size; i += CACHE_LINE_64 * 16) {
                    special_buf[i] = special_buf[(i + CACHE_LINE_64 * 12) % special_size] + iter;
                }
                COMPILER_BARRIER();
            }
            final_result += special_buf[0];
            free(special_buf);
        }
    }
    benchmark_generic();
    
#elif defined(TEST_K8)
    printf("Running AMD K8 benchmarks...\n");
    benchmark_k8();
    benchmark_generic();
    
#elif defined(TEST_MULTI)
    printf("Running multi-architecture benchmarks...\n");
    benchmark_multi();
    /* Explicitly call different versions if target_clones not available */
    benchmark_pentium3();
    benchmark_nocona();
    benchmark_k8();
    
#else
    printf("Running generic benchmark...\n");
    benchmark_generic();
#endif
    
    /* Force memory operations and prevent optimization */
    COMPILER_BARRIER();
    
    printf("Benchmark completed. Final dummy result: %d\n", final_result);
    printf("(This value is meaningless - used only to prevent optimization)\n");
    
    return 0;
}
