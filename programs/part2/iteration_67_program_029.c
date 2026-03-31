/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define USE_CPU_FEATURES 1
#else
#define USE_CPU_FEATURES 0
#endif

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef void (*compute_func_t)(volatile int*, volatile int*, int, int);
static compute_func_t compute_func = NULL;

/* Different computation patterns based on CPU features */
void compute_basic(volatile int* a, volatile int* b, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        a[i] = b[i] * 3 + 7;
    }
}

void compute_sse_optimized(volatile int* a, volatile int* b, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        a[i] = b[i] * 5 - 2;
    }
}

void compute_avx_optimized(volatile int* a, volatile int* b, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        a[i] = b[i] * 7 + 11;
    }
}

void compute_avx512_optimized(volatile int* a, volatile int* b, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        a[i] = b[i] * 13 - 17;
    }
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#if USE_CPU_FEATURES
/* These conditionals are evaluated at compile-time by the driver */
#if defined(__SSE2__) || defined(__SSE3__) || defined(__SSSE3__)
#define HAS_SSE_FEATURES 1
#else
#define HAS_SSE_FEATURES 0
#endif

#if defined(__AVX__) || defined(__AVX2__)
#define HAS_AVX_FEATURES 1
#else
#define HAS_AVX_FEATURES 0
#endif

#if defined(__AVX512F__) || defined(__AVX512VL__)
#define HAS_AVX512_FEATURES 1
#else
#define HAS_AVX512_FEATURES 0
#endif
#endif /* USE_CPU_FEATURES */

int main(void) {
    /* Initialize CPU detection - forces driver to run CPUID logic */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* Check various CPU features - each call may trigger cache detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Choose computation function based on CPU features */
    if (has_avx512f) {
        compute_func = compute_avx512_optimized;
    } else if (has_avx2) {
        compute_func = compute_avx_optimized;
    } else if (has_sse2) {
        compute_func = compute_sse_optimized;
    } else {
        compute_func = compute_basic;
    }
    
    /* Different stride patterns to test various cache behaviors */
    int strides[] = {1, 2, 4, 8, 16, 32, 64};
    int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    uint64_t checksum = 0;
    
    /* Perform computations with different strides */
    for (int s = 0; s < num_strides; s++) {
        int stride = strides[s];
        
        /* Use different array sizes to potentially hit different cache levels */
        if (stride <= 4) {
            compute_func(array1, array2, 1024 * 1024, stride);
        } else if (stride <= 16) {
            compute_func(array3, array1, 512 * 512, stride);
        } else {
            compute_func(array2, array3, 256 * 256, stride);
        }
        
        /* Simple checksum to prevent dead code elimination */
        for (int i = 0; i < 1000; i += stride * 2) {
            checksum += array1[i % 1024];
            checksum += array2[i % 512];
            checksum += array3[i % 256];
        }
    }
    
    printf("CPU Feature Checksum: %llu\n", (unsigned long long)checksum);
    printf("CPU Features detected: SSE2=%d, SSE3=%d, SSSE3=%d, SSE4.1=%d, SSE4.2=%d, AVX=%d, AVX2=%d, AVX512F=%d\n",
           has_sse2, has_sse3, has_ssse3, has_sse4_1, has_sse4_2, has_avx, has_avx2, has_avx512f);
    
    return 0;
}

/* Additional conditional compilation blocks to force driver evaluation */
#ifdef __FAST_MATH__
/* When fast math is enabled, check for FMA support */
static volatile int has_fma = 0;
__attribute__((constructor)) 
static void init_fma_check(void) {
    has_fma = __builtin_cpu_supports("fma");
}
#endif

/* Architecture-specific code paths */
#if defined(__i386__) || defined(__x86_64__)
/* x86-specific code that might trigger different cache detection paths */
__attribute__((used)) 
static void x86_cache_hint(void) {
    /* This function's existence may influence driver decisions */
    asm volatile("" ::: "memory");
}
#endif
