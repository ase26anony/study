/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force CPU initialization early */
__attribute__((constructor)) 
static void init_cpu_features(void) {
    __builtin_cpu_init();
}

/* Different computation strategies based on CPU features */
static int compute_sse(int a, int b) {
    return (a * b) + (a ^ b);
}

static int compute_avx(int a, int b) {
    return (a + b) * (a - b);
}

static int compute_basic(int a, int b) {
    return a + b * 2;
}

int main(void) {
    volatile int result = 0;
    volatile int stride = 1;
    
    /* Initialize CPU features - this triggers driver's CPUID logic */
    __builtin_cpu_init();
    
    /* Store feature detection results in volatile to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse4 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Select function based on CPU features - forces driver to evaluate */
    if (has_avx512) {
        func_ptr = compute_avx;
        stride = 16;
    } else if (has_avx2) {
        func_ptr = compute_avx;
        stride = 8;
    } else if (has_avx) {
        func_ptr = compute_avx;
        stride = 4;
    } else if (has_sse4) {
        func_ptr = compute_sse;
        stride = 2;
    } else if (has_sse2) {
        func_ptr = compute_sse;
        stride = 1;
    } else {
        func_ptr = compute_basic;
        stride = 1;
    }
    
    /* Cache-sensitive loop with variable stride */
    for (volatile size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += stride) {
        array1[i] = func_ptr(i, result);
        result ^= array1[i];
    }
    
    /* Additional loops to increase cache pressure */
#if defined(__OPTIMIZE__) && __OPTIMIZE__ > 0
    /* This section only compiled with optimization, forcing driver to consider cache */
    if (has_sse2 || has_avx) {
        for (volatile size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += (stride * 2)) {
            array2[i] = array1[i % (sizeof(array1)/sizeof(array1[0]))] + i;
            result += array2[i];
        }
    }
#endif
    
    /* Conditional compilation based on math optimizations */
#ifdef __FAST_MATH__
    if (has_avx || has_avx2) {
        /* Use faster math operations */
        for (volatile size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i++) {
            array3[i] = (array3[i] * 3) / 2;
            result ^= array3[i];
        }
    }
#endif
    
    /* Architecture-specific optimizations */
#if defined(__tune_core2__) || defined(__tune_nehalem__)
    /* Special handling for older Intel architectures */
    if (has_sse2 && !has_avx) {
        for (volatile size_t i = 0; i < 1000; i++) {
            result = (result << 3) | (result >> 29);
        }
    }
#endif
    
#if defined(__tune_sandybridge__) || defined(__tune_ivybridge__)
    /* Sandy Bridge/Ivy Bridge specific */
    if (has_avx) {
        for (volatile size_t i = 0; i < 500; i++) {
            result = (result * 1103515245 + 12345) & 0x7fffffff;
        }
    }
#endif
    
#if defined(__tune_skylake__) || defined(__tune_skylake_avx512__)
    /* Skylake and newer */
    if (has_avx2 || has_avx512) {
        for (volatile size_t i = 0; i < 250; i++) {
            result = result ^ (result >> 16);
            result = result * 0x45d9f3b;
        }
    }
#endif
    
    printf("Result checksum: %d\n", result);
    printf("CPU Features detected: SSE2=%d, SSE4.2=%d, AVX=%d, AVX2=%d, AVX512=%d\n",
           has_sse2, has_sse4, has_avx, has_avx2, has_avx512);
    
    return result != 0 ? 0 : 1;
}

/* Additional functions to prevent dead code elimination */
__attribute__((noinline))
static void touch_memory(volatile int* arr, size_t size) {
    for (size_t i = 0; i < size; i += 64) {
        arr[i] = i;
    }
}

__attribute__((used))
static void ensure_cpuid_usage(void) {
    /* This function references CPU features but isn't called directly */
    /* It prevents aggressive optimization from removing CPU detection */
    volatile int dummy = __builtin_cpu_supports("sse") +
                        __builtin_cpu_supports("sse2") +
                        __builtin_cpu_supports("sse3") +
                        __builtin_cpu_supports("ssse3") +
                        __builtin_cpu_supports("sse4.1") +
                        __builtin_cpu_supports("sse4.2") +
                        __builtin_cpu_supports("avx") +
                        __builtin_cpu_supports("avx2") +
                        __builtin_cpu_supports("avx512f");
    (void)dummy;
}
