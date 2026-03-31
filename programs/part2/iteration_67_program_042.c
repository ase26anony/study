/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force driver to evaluate __builtin_cpu_supports during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#endif

#if defined(__AVX__) || defined(__AVX2__)
#define USE_AVX_FEATURES 1
#endif

#if defined(__AVX512F__) || defined(__AVX512BW__)
#define USE_AVX512_FEATURES 1
#endif
#endif

/* Different computation functions for different CPU features */
static int compute_sse(int stride, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        array1[i] = array2[i] + array3[i % 8192];
        sum += array1[i];
    }
    return sum;
}

static int compute_avx(int stride, int size) {
    int sum = 0;
    /* Non-linear access pattern to stress cache */
    for (int i = 0; i < size; i += stride) {
        int idx = (i * 17) % size; /* Non-constant stride */
        array1[idx] = array2[idx] * 2 - array3[idx % 8192];
        sum += array1[idx];
    }
    return sum;
}

static int compute_generic(int stride, int size) {
    int sum = 0;
    /* Different access pattern */
    for (int i = 0; i < size; i += stride) {
        array1[i] = i;
        sum += array1[i];
    }
    return sum;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(void) {
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = (i * 13) % 256;
        if (i < sizeof(array2)/sizeof(array2[0])) {
            array2[i] = (i * 17) % 256;
        }
        if (i < sizeof(array3)/sizeof(array3[0])) {
            array3[i] = (i * 19) % 256;
        }
    }
}

int main(void) {
    /* Force CPU initialization - this triggers CPUID in driver */
    __builtin_cpu_init();
    
    /* Store feature checks in volatile to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Cache-specific feature checks */
    volatile int has_xsave = __builtin_cpu_supports("xsave");
    volatile int has_xsaveopt = __builtin_cpu_supports("xsaveopt");
    volatile int has_clflushopt = __builtin_cpu_supports("clflushopt");
    
    /* Choose function based on CPU features */
    if (has_avx512f) {
        func_ptr = compute_avx;
    } else if (has_avx2) {
        func_ptr = compute_avx;
    } else if (has_avx) {
        func_ptr = compute_avx;
    } else if (has_sse4_2) {
        func_ptr = compute_sse;
    } else if (has_sse2) {
        func_ptr = compute_sse;
    } else {
        func_ptr = compute_generic;
    }
    
    /* Initialize data */
    init_arrays();
    
    /* Perform computation with different strides to test cache effects */
    int total_sum = 0;
    
    /* Varying strides to exercise different cache line scenarios */
    int strides[] = {1, 2, 4, 8, 16, 32, 64, 128};
    int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    for (int s = 0; s < num_strides; s++) {
        int stride = strides[s];
        int size = 1024 * 256; /* Large enough to exceed L1 cache */
        
        if (func_ptr) {
            total_sum += func_ptr(stride, size);
        }
        
        /* Mix access patterns */
        if (has_sse2) {
            /* Additional SSE-optimized path */
            for (int i = 0; i < size; i += stride * 2) {
                array2[i] = array1[i] + total_sum;
            }
        }
    }
    
    /* Final computation that depends on all previous results */
    int final_result = 0;
    for (int i = 0; i < 8192; i += 64) { /* Cache line sized accesses */
        final_result += array1[i] + array2[i % 4096] + array3[i % 2048];
    }
    
    printf("Result: %d (CPU features: SSE2=%d, AVX=%d, AVX2=%d, AVX512F=%d)\n",
           final_result, has_sse2, has_avx, has_avx2, has_avx512f);
    
    return final_result != 0 ? 0 : 1;
}

/* Additional functions to ensure various code paths exist */
#ifdef __SSE2__
static void sse2_detected(void) {
    /* This function only exists if SSE2 is detected at compile time */
    volatile int check = __builtin_cpu_supports("sse2");
    (void)check;
}
#endif

#ifdef __AVX__
static void avx_detected(void) {
    /* This function only exists if AVX is detected at compile time */
    volatile int check = __builtin_cpu_supports("avx");
    (void)check;
}
#endif

/* Force inclusion of CPU feature checks in dead code */
static void unused_function(void) {
    /* These checks force driver to evaluate CPUID */
    volatile int unused = 0;
    unused += __builtin_cpu_supports("mmx");
    unused += __builtin_cpu_supports("sse");
    unused += __builtin_cpu_supports("sse2");
    unused += __builtin_cpu_supports("sse3");
    unused += __builtin_cpu_supports("ssse3");
    unused += __builtin_cpu_supports("sse4.1");
    unused += __builtin_cpu_supports("sse4.2");
    unused += __builtin_cpu_supports("avx");
    unused += __builtin_cpu_supports("avx2");
    unused += __builtin_cpu_supports("avx512f");
    unused += __builtin_cpu_supports("fma");
    unused += __builtin_cpu_supports("bmi");
    unused += __builtin_cpu_supports("bmi2");
    (void)unused;
}
