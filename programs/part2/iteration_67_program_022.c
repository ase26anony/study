/* cache_detection_test.c
 * 
 * This program is designed to trigger GCC's CPUID-based cache detection
 * logic during compilation, specifically targeting the switch statement
 * in driver-i386.cc lines 127-244.
 * 
 * Compile with various -march= options to exercise different cache
 * descriptor paths in the GCC driver.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-aware optimizations */
static int array1[1024 * 1024];      /* 4MB */
static int array2[1024 * 1024];      /* Another 4MB */
static volatile int sink = 0;        /* Prevent dead code elimination */

/* Function pointer type to prevent optimization */
typedef void (*ComputeFunc)(int*, int*, size_t, int);

/* Different computation patterns that might benefit from
 * cache-aware optimizations based on CPU features */
void compute_basic(int* a, int* b, size_t n, int stride) {
    for (size_t i = 0; i < n; i += stride) {
        a[i] = b[i] * 3 + 7;
    }
}

void compute_sse_style(int* a, int* b, size_t n, int stride) {
    for (size_t i = 0; i < n; i += stride) {
        a[i] = (b[i] << 2) | (b[i] >> 30);  /* Some bit manipulation */
    }
}

void compute_avx_style(int* a, int* b, size_t n, int stride) {
    for (size_t i = 0; i < n; i += stride) {
        a[i] = b[i] * b[i] - b[i] / 2;
    }
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#ifdef __OPTIMIZE__
/* This block requires CPU feature detection during compilation */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX512F__)
#define USE_CPU_FEATURE_DETECTION 1
#endif
#endif

/* Main computation that uses CPU feature detection */
int main(void) {
    size_t n = sizeof(array1) / sizeof(array1[0]);
    int stride = 1;
    uint64_t checksum = 0;
    
    /* Initialize CPU detection - this triggers driver's CPUID logic */
    __builtin_cpu_init();
    
    /* Use volatile variables to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse4 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512 = 0;
    
    /* These builtin calls force the driver to execute CPUID detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse4 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Choose stride based on cache line size hints from CPU features */
    if (has_avx512) {
        stride = 16;  /* AVX512 suggests larger cache lines */
    } else if (has_avx2) {
        stride = 8;
    } else if (has_avx) {
        stride = 4;
    } else if (has_sse4) {
        stride = 2;
    }
    
    /* Select computation function based on features */
    ComputeFunc func = compute_basic;
    
#ifdef USE_CPU_FEATURE_DETECTION
    /* This conditional depends on compile-time feature detection
     * which requires the driver to evaluate CPUID */
    if (has_avx) {
        func = compute_avx_style;
    } else if (has_sse2) {
        func = compute_sse_style;
    }
#endif
    
    /* Perform computation with selected function and stride */
    func(array1, array2, n, stride);
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < n; i += 1024) {
        checksum += array1[i];
    }
    
    /* Use the results to prevent optimization */
    sink = (int)(checksum & 0xFFFFFFFF);
    
    printf("CPU Features: SSE2=%d, SSE4=%d, AVX=%d, AVX2=%d, AVX512=%d\n",
           has_sse2, has_sse4, has_avx, has_avx2, has_avx512);
    printf("Selected stride: %d\n", stride);
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}

/* Additional conditional compilation that references CPU features
 * This forces the driver to evaluate __builtin_cpu_supports during
 * preprocessing/compilation */
#if defined(__OPTIMIZE__) && (__builtin_cpu_supports("sse2") || \
                              __builtin_cpu_supports("avx") || \
                              __builtin_cpu_supports("avx2"))
/* This pragma might influence optimization decisions based on cache */
#pragma GCC optimize("unroll-loops")
#endif

/* Cache-sensitive matrix multiplication that might benefit from
 * cache-aware optimizations when CPUID is detected */
#ifdef __OPTIMIZE__
static void cache_sensitive_multiply(int size) {
    /* Using volatile to prevent excessive optimization */
    static volatile int* matrix_a __attribute__((unused));
    static volatile int* matrix_b __attribute__((unused));
    static volatile int* matrix_c __attribute__((unused));
    
    /* Reference CPU features to ensure detection runs */
    if (__builtin_cpu_supports("sse2")) {
        /* Empty but forces evaluation */
    }
}
#endif
