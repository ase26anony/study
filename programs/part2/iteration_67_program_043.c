/* cache_coverage.c - Trigger GCC driver CPUID cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache consideration */
static volatile int array_a[1024 * 1024];
static volatile int array_b[1024 * 1024];
static volatile int array_c[1024 * 1024];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#else
#define USE_SSE_FEATURES 0
#endif

#if defined(__AVX__) || defined(__AVX2__)
#define USE_AVX_FEATURES 1
#else
#define USE_AVX_FEATURES 0
#endif

#if defined(__AVX512F__) || defined(__AVX512BW__)
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif
#endif /* __OPTIMIZE__ */

/* Different computation functions for different CPU features */
static int compute_sse(int a, int b) {
    return (a * b) + (a ^ b);
}

static int compute_avx(int a, int b) {
    return (a + b) * (a - b);
}

static int compute_avx512(int a, int b) {
    return (a << 2) | (b >> 2);
}

static int compute_basic(int a, int b) {
    return a + b * 3;
}

/* Main computation with cache-sensitive access patterns */
static uint64_t cache_sensitive_compute(int stride) {
    uint64_t checksum = 0;
    const int size = sizeof(array_a) / sizeof(array_a[0]);
    
    /* Different access patterns based on stride */
    for (int i = 0; i < size; i += stride) {
        array_a[i] = i;
        array_b[i] = size - i;
        
        /* Conditional based on CPU features */
        if (func_ptr) {
            array_c[i] = func_ptr(array_a[i], array_b[i]);
        } else {
            array_c[i] = array_a[i] + array_b[i];
        }
        
        checksum += array_c[i];
    }
    
    /* Second pass with different pattern */
    for (int i = size - 1; i >= 0; i -= (stride * 2)) {
        if (i >= 0) {
            checksum ^= array_c[i];
        }
    }
    
    return checksum;
}

int main(void) {
    /* Initialize CPU detection - forces driver to run CPUID logic */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent optimization */
    volatile int has_sse2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512 = 0;
    
    /* Check CPU features - each call may trigger cache detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Select computation function based on CPU features */
    if (has_avx512) {
        func_ptr = compute_avx512;
    } else if (has_avx2) {
        func_ptr = compute_avx;
    } else if (has_sse2) {
        func_ptr = compute_sse;
    } else {
        func_ptr = compute_basic;
    }
    
    /* Determine stride based on cache line assumptions */
    int stride = 1;
    
    /* These conditionals encourage driver to consider cache parameters */
#ifdef __OPTIMIZE__
    if (has_avx512) {
        stride = 16;  /* AVX512 might have different cache behavior */
    } else if (has_avx2) {
        stride = 8;
    } else if (has_sse2) {
        stride = 4;
    }
#endif
    
    /* Perform cache-sensitive computation */
    uint64_t result = cache_sensitive_compute(stride);
    
    /* Additional CPU-specific computations */
    volatile uint64_t extra_result = 0;
    
    /* Force evaluation of multiple CPU feature checks */
    if (__builtin_cpu_supports("ssse3")) {
        extra_result ^= result * 3;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        extra_result ^= result << 2;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        extra_result ^= result >> 2;
    }
    if (__builtin_cpu_supports("fma")) {
        extra_result += result * 7;
    }
    
    /* Final result prevents dead code elimination */
    printf("CPU Checksum: %llu\n", (unsigned long long)(result ^ extra_result));
    
    return 0;
}

/* Additional functions that might trigger different optimization paths */
#ifdef __FAST_MATH__
static double math_intensive(double *data, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i];
    }
    return sum;
}
#endif

/* Conditional compilation based on architecture macros */
#if defined(__tune_core2__) || defined(__tune_nehalem__)
static void legacy_cache_pattern(void) {
    /* Pattern that might benefit from L1/L2 cache tuning */
    volatile int temp[256];
    for (int i = 0; i < 256; i++) {
        temp[i] = i * i;
    }
}
#endif

#if defined(__tune_sandybridge__) || defined(__tune_ivybridge__)
static void modern_cache_pattern(void) {
    /* Different access pattern for newer architectures */
    volatile int temp[512];
    for (int i = 0; i < 512; i += 8) {
        temp[i] = i * 2;
    }
}
#endif

#if defined(__tune_skylake__) || defined(__tune_skylake_avx512__)
static void avx512_cache_pattern(void) {
    /* Pattern for AVX-512 capable CPUs */
    volatile int temp[1024];
    for (int i = 0; i < 1024; i += 16) {
        temp[i] = i * 4;
    }
}
#endif
