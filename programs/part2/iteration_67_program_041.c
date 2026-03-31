/* test_cpuid_cache.c - Triggers GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache consideration */
static volatile int array_a[1024 * 1024];
static volatile int array_b[1024 * 1024];
static volatile int array_c[512 * 512];

/* Function pointers to prevent optimization */
typedef void (*compute_func_t)(volatile int*, volatile int*, volatile int*, int);
static compute_func_t compute_func = NULL;

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

#if defined(__AVX512F__) || defined(__AVX512VL__)
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif
#endif /* __OPTIMIZE__ */

/* Different computation patterns based on CPU features */
static void compute_basic(volatile int* a, volatile int* b, volatile int* c, int stride) {
    unsigned long i;
    for (i = 0; i < 1024 * 1024; i += stride) {
        a[i] = b[i] + c[i % (512 * 512)];
    }
}

static void compute_sse_style(volatile int* a, volatile int* b, volatile int* c, int stride) {
    unsigned long i;
    /* Simulate SSE-like access pattern */
    for (i = 0; i < 1024 * 1024; i += stride * 4) {
        a[i] = b[i] * 2 - c[i % (512 * 512)];
        a[i + stride] = b[i + stride] + c[(i + stride) % (512 * 512)];
        a[i + stride * 2] = b[i + stride * 2] - c[(i + stride * 2) % (512 * 512)];
        a[i + stride * 3] = b[i + stride * 3] ^ c[(i + stride * 3) % (512 * 512)];
    }
}

static void compute_avx_style(volatile int* a, volatile int* b, volatile int* c, int stride) {
    unsigned long i;
    /* Simulate wider vector access pattern */
    for (i = 0; i < 1024 * 1024; i += stride * 8) {
        for (int j = 0; j < 8; j++) {
            a[i + stride * j] = (b[i + stride * j] << 1) | c[(i + stride * j) % (512 * 512)];
        }
    }
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(void) {
    unsigned long i;
    for (i = 0; i < 1024 * 1024; i++) {
        array_a[i] = 0;
        array_b[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (i = 0; i < 512 * 512; i++) {
        array_c[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
}

int main(void) {
    int checksum = 0;
    int stride = 1;
    
    /* Force CPU initialization - this triggers CPUID in GCC driver */
    __builtin_cpu_init();
    
    /* Check various CPU features - each call may influence cache detection */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Choose stride based on CPU features to test different cache behaviors */
    if (has_avx512f) {
        stride = 16;  /* Large stride for AVX512 */
        compute_func = compute_avx_style;
    } else if (has_avx2) {
        stride = 8;   /* Medium stride for AVX2 */
        compute_func = compute_avx_style;
    } else if (has_sse4_2) {
        stride = 4;   /* Smaller stride for SSE4 */
        compute_func = compute_sse_style;
    } else {
        stride = 2;   /* Basic stride */
        compute_func = compute_basic;
    }
    
    /* Force driver to consider cache topology by using feature-dependent code */
#ifdef __OPTIMIZE__
    /* These macros reference CPU features, forcing driver evaluation */
    if (USE_SSE_FEATURES && has_sse2) {
        stride = stride * 2;
    }
    if (USE_AVX_FEATURES && has_avx) {
        stride = stride * 2;
    }
    if (USE_AVX512_FEATURES && has_avx512f) {
        stride = stride * 2;
    }
#endif
    
    /* Initialize data */
    init_arrays();
    
    /* Perform computation with chosen function */
    if (compute_func) {
        compute_func(array_a, array_b, array_c, stride);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (unsigned long i = 0; i < 1024 * 1024; i += 1024) {
        checksum ^= array_a[i];
    }
    
    /* Also check cache line size sensitive computation */
    volatile int* ptr = array_a;
    for (int i = 0; i < 64; i++) {  /* 64 cache lines worth */
        checksum += ptr[i * 16];    /* Assuming 64-byte cache lines */
    }
    
    printf("Checksum: %d (CPU features: SSE=%d, SSE2=%d, AVX=%d, AVX2=%d, AVX512=%d)\n",
           checksum, has_sse, has_sse2, has_avx, has_avx2, has_avx512f);
    
    return 0;
}

/* Additional functions that might trigger specific optimizations */
#ifdef __SSE2__
static void sse2_specific_work(void) {
    /* This function only compiled if SSE2 is available */
    volatile int temp = 0;
    for (int i = 0; i < 1000; i++) {
        temp += __builtin_cpu_supports("sse2") ? 1 : 0;
    }
}
#endif

#ifdef __AVX__
static void avx_specific_work(void) {
    /* This function only compiled if AVX is available */
    volatile int temp = 0;
    for (int i = 0; i < 1000; i++) {
        temp += __builtin_cpu_supports("avx") ? 2 : 0;
    }
}
#endif
