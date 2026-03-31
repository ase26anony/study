/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These macros will force driver to check CPU features during parsing */
#define CHECK_FEATURE(feat) __builtin_cpu_supports(feat)
#define INIT_CPU() __builtin_cpu_init()
#else
#define CHECK_FEATURE(feat) 0
#define INIT_CPU() do {} while(0)
#endif

/* Large arrays to encourage cache consideration */
static volatile int data_array[1024 * 1024];  /* 4MB */
static volatile int temp_array[512 * 512];    /* 1MB */

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Different computation patterns that might benefit from cache-aware optimizations */
static int compute_stride_1(int start, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 1) {
        sum += data_array[i & 0xFFFFF];
    }
    return sum;
}

static int compute_stride_2(int start, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 2) {
        sum += data_array[i & 0xFFFFF] * 2;
    }
    return sum;
}

static int compute_stride_4(int start, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 4) {
        sum += data_array[i & 0xFFFFF] * 3;
    }
    return sum;
}

static int compute_stride_8(int start, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 8) {
        sum += data_array[i & 0xFFFFF] * 5;
    }
    return sum;
}

static int compute_stride_16(int start, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 16) {
        sum += data_array[i & 0xFFFFF] * 7;
    }
    return sum;
}

/* Matrix multiplication - cache sensitive */
static void cache_sensitive_multiply(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                /* Non-contiguous access pattern */
                sum += data_array[(i * n + k) & 0xFFFFF] * 
                       data_array[(k * n + j) & 0xFFFFF];
            }
            temp_array[(i * n + j) & 0x3FFFF] = sum;
        }
    }
}

int main(void) {
    /* Initialize CPU detection - forces driver to run CPUID */
    INIT_CPU();
    
    /* Volatile results to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* Check various CPU features - each forces driver to evaluate CPUID */
#ifdef __SSE2__
    has_sse2 = CHECK_FEATURE("sse2");
#endif
#ifdef __SSE3__
    has_sse3 = CHECK_FEATURE("sse3");
#endif
#ifdef __SSSE3__
    has_ssse3 = CHECK_FEATURE("ssse3");
#endif
#ifdef __SSE4_1__
    has_sse4_1 = CHECK_FEATURE("sse4.1");
#endif
#ifdef __SSE4_2__
    has_sse4_2 = CHECK_FEATURE("sse4.2");
#endif
#ifdef __AVX__
    has_avx = CHECK_FEATURE("avx");
#endif
#ifdef __AVX2__
    has_avx2 = CHECK_FEATURE("avx2");
#endif
#ifdef __AVX512F__
    has_avx512f = CHECK_FEATURE("avx512f");
#endif
    
    /* Initialize array with pseudo-random data */
    for (int i = 0; i < 1024 * 1024; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Choose computation based on CPU features */
    int stride = 1;
    if (has_avx512f) {
        func_ptr = compute_stride_16;
        stride = 16;
    } else if (has_avx2) {
        func_ptr = compute_stride_8;
        stride = 8;
    } else if (has_avx) {
        func_ptr = compute_stride_4;
        stride = 4;
    } else if (has_sse4_2) {
        func_ptr = compute_stride_2;
        stride = 2;
    } else {
        func_ptr = compute_stride_1;
        stride = 1;
    }
    
    /* Perform cache-sensitive computation */
    int result = 0;
    if (func_ptr) {
        result = func_ptr(0, 1000000);
    }
    
    /* Additional cache-sensitive operation */
    cache_sensitive_multiply(64);
    
    /* Use temp_array to prevent dead code elimination */
    for (int i = 0; i < 512 * 512; i += 64) {
        result += temp_array[i];
    }
    
    /* Mix in CPU feature flags */
    result = (result ^ has_sse2) + (has_sse3 << 3) + 
             (has_ssse3 << 6) + (has_sse4_1 << 9) +
             (has_sse4_2 << 12) + (has_avx << 15) +
             (has_avx2 << 18) + (has_avx512f << 21);
    
    printf("Result: %d (stride: %d)\n", result, stride);
    printf("CPU Features: SSE2=%d SSE3=%d SSSE3=%d SSE4.1=%d SSE4.2=%d AVX=%d AVX2=%d AVX512F=%d\n",
           has_sse2, has_sse3, has_ssse3, has_sse4_1, 
           has_sse4_2, has_avx, has_avx2, has_avx512f);
    
    return result & 0xFF;
}
