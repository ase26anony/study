/* test_cache_coverage.c - Cover GCC i386 driver cache detection cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger specific cache descriptor cases */
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
static void benchmark_cache(int *buffer, size_t size, int iterations) {
    volatile int result = 0;
    size_t i, j;
    
    /* Simple LCG for pseudo-random access pattern */
    uint32_t lcg_state = 123456789;
    
    for (j = 0; j < iterations; j++) {
        /* Sequential access (good for prefetch) */
        for (i = 0; i < size; i++) {
            buffer[i] = i;
        }
        MB();
        
        /* Random access pattern to stress cache associativity */
        lcg_state = 1103515245 * lcg_state + 12345;
        size_t start = lcg_state % (size / 2);
        
        for (i = 0; i < size / 4; i++) {
            size_t idx = (start + i * 7) % size;  /* Stride 7 for conflict misses */
            result += buffer[idx];
            buffer[idx] = result;
        }
        MB();
        
        /* Reverse sequential access */
        for (i = size; i > 0; i--) {
            result += buffer[i - 1];
        }
        MB();
    }
    
    /* Use result to prevent dead code elimination */
    if (result == 0x1234) {  /* Never true, but compiler doesn't know */
        printf("Impossible\n");
    }
}

/* Multi-versioned function for different CPU targets */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
#endif
void run_cache_benchmarks() {
    const size_t l1_size = 32 * 1024;      /* 32KB - typical L1 */
    const size_t l2_size = 512 * 1024;     /* 512KB - typical L2 */
    const size_t huge_size = 4 * 1024 * 1024; /* 4MB - larger than most L2 */
    
    int *buffer_small = malloc(l1_size * sizeof(int));
    int *buffer_medium = malloc(l2_size * sizeof(int));
    int *buffer_large = malloc(huge_size * sizeof(int));
    
    if (!buffer_small || !buffer_medium || !buffer_large) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with non-zero values */
    for (size_t i = 0; i < l1_size; i++) buffer_small[i] = i;
    for (size_t i = 0; i < l2_size; i++) buffer_medium[i] = i;
    for (size_t i = 0; i < huge_size; i++) buffer_large[i] = i;
    
    MB();
    
    /* Run benchmarks with different working set sizes */
    benchmark_cache(buffer_small, l1_size / sizeof(int), 1000);
    benchmark_cache(buffer_medium, l2_size / sizeof(int), 100);
    benchmark_cache(buffer_large, huge_size / sizeof(int), 10);
    
    /* Cross-buffer access to cause cache conflicts */
    volatile int sum = 0;
    for (size_t i = 0; i < l1_size; i += 64) {  /* Cache line sized steps */
        sum += buffer_small[i];
        sum += buffer_medium[i % (l2_size / sizeof(int))];
        sum += buffer_large[i % (huge_size / sizeof(int))];
    }
    
    MB();
    
    free(buffer_small);
    free(buffer_medium);
    free(buffer_large);
    
    /* Prevent optimization of sum */
    if (sum == 0xdeadbeef) {
        printf("%d\n", sum);
    }
}

/* Specialized functions for specific cache descriptor cases */
#ifdef COVER_CASE_0x49
/* Need a non-Xeon-MP CPU that returns descriptor 0x49 */
__attribute__((target("arch=nocona")))  /* Intel Xeon DP (not MP) */
void test_case_0x49() {
    /* Large memory operations that would benefit from 4MB L2 */
    const size_t size = 8 * 1024 * 1024; /* 8MB > 4MB L2 */
    int *buffer = malloc(size * sizeof(int));
    
    if (!buffer) return;
    
    /* Pattern that benefits from large cache */
    for (int iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < size; i += 64) {
            buffer[i] = iter;
        }
        MB();
    }
    
    free(buffer);
}
#endif

#ifdef COVER_CASE_0x0A_0x0C
/* Early Pentium III variants */
__attribute__((target("arch=pentium3")))
void test_early_p3_cache() {
    /* Small array fits in 8KB/16KB L1 */
    int array[2048];  /* 8KB */
    volatile int sum = 0;
    
    for (int i = 0; i < 10000; i++) {
        for (int j = 0; j < 2048; j += 8) {
            sum += array[j];
        }
    }
}
#endif

int main() {
    printf("Cache benchmark starting...\n");
    
    /* Run main benchmark */
    run_cache_benchmarks();
    
    /* Conditional compilation for specific cases */
#if defined(COVER_CASE_0x49)
    test_case_0x49();
#endif
    
#if defined(COVER_CASE_0x0A_0x0C)
    test_early_p3_cache();
#endif
    
    /* Additional memory intensive work */
    const size_t large_size = 16 * 1024 * 1024;
    float *matrix = malloc(large_size * sizeof(float));
    
    if (matrix) {
        /* Matrix-style access pattern */
        for (size_t i = 0; i < large_size; i += 1024) {
            matrix[i] = i * 0.1f;
        }
        
        /* Use volatile to ensure computation happens */
        volatile float total = 0.0f;
        for (size_t i = 0; i < large_size; i += 128) {
            total += matrix[i];
        }
        
        free(matrix);
    }
    
    printf("Cache benchmark completed.\n");
    return 0;
}
