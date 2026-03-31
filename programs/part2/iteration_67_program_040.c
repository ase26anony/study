/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These macros will force driver to check CPU features */
#define USE_SSE_FEATURES __builtin_cpu_supports("sse2")
#define USE_AVX_FEATURES __builtin_cpu_supports("avx")
#define USE_AVX2_FEATURES __builtin_cpu_supports("avx2")
#define USE_AVX512_FEATURES __builtin_cpu_supports("avx512f")
#else
#define USE_SSE_FEATURES 0
#define USE_AVX_FEATURES 0
#define USE_AVX2_FEATURES 0
#define USE_AVX512_FEATURES 0
#endif

/* Large arrays to encourage cache consideration */
static volatile int large_array[1024 * 1024];  /* 4MB */
static volatile int medium_array[256 * 1024];  /* 1MB */
static volatile int small_array[64 * 1024];    /* 256KB */

/* Function pointers to prevent optimization */
typedef void (*ComputeFunc)(int*, size_t, int);
volatile ComputeFunc func_ptr = NULL;

/* Different computation patterns targeting different cache behaviors */
void compute_simple(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += arr[i];
    }
    /* Use sum to prevent dead code elimination */
    arr[0] = sum;
}

void compute_sse_style(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Pattern that might benefit from SSE */
    for (size_t i = 0; i < size; i += stride) {
        sum += arr[i] * 2 - arr[size - i - 1];
    }
    arr[0] = sum;
}

void compute_avx_style(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* More complex pattern */
    for (size_t i = 0; i < size; i += stride) {
        sum += (arr[i] << 2) | (arr[size - i - 1] & 0xFF);
    }
    arr[0] = sum;
}

/* Initialize arrays with pseudo-random but deterministic values */
void init_arrays(void) {
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i++) {
        large_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(medium_array)/sizeof(medium_array[0]); i++) {
        medium_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(small_array)/sizeof(small_array[0]); i++) {
        small_array[i] = (i * 214013 + 2531011) & 0x7FFF;
    }
}

int main(void) {
    /* Force CPU initialization - this triggers CPUID in GCC driver */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512 = 0;
    
    /* Check CPU features - each call may trigger cache detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Also check cache-related features */
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_fma = __builtin_cpu_supports("fma");
    
    /* Initialize arrays */
    init_arrays();
    
    /* Choose computation based on CPU features */
    int stride = 1;
    ComputeFunc compute = compute_simple;
    
    if (has_sse2) {
        stride = 2;
        compute = compute_sse_style;
    }
    if (has_avx) {
        stride = 4;
        compute = compute_avx_style;
    }
    if (has_avx2) {
        stride = 8;
    }
    if (has_avx512) {
        stride = 16;
    }
    
    /* Store function pointer to prevent optimization */
    func_ptr = compute;
    
    /* Perform computations on different array sizes */
    int result = 0;
    
    /* Test different strides to exercise cache behavior */
    for (int s = 1; s <= 16; s *= 2) {
        if (func_ptr) {
            func_ptr((int*)small_array, 
                    sizeof(small_array)/sizeof(small_array[0]), 
                    s);
            result += small_array[0];
        }
    }
    
    /* Medium array with varying strides */
    for (int s = 1; s <= 8; s *= 2) {
        compute_simple((int*)medium_array,
                      sizeof(medium_array)/sizeof(medium_array[0]),
                      s);
        result += medium_array[0];
    }
    
    /* Large array - more likely to trigger cache considerations */
    compute_simple((int*)large_array,
                  sizeof(large_array)/sizeof(large_array[0]),
                  stride);
    result += large_array[0];
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE2__
    if (USE_SSE_FEATURES) {
        result ^= 0x5555;
    }
    #endif
    
    #ifdef __AVX__
    if (USE_AVX_FEATURES) {
        result ^= 0xAAAA;
    }
    #endif
    
    #ifdef __AVX2__
    if (USE_AVX2_FEATURES) {
        result ^= 0xCCCC;
    }
    #endif
    
    #ifdef __AVX512F__
    if (USE_AVX512_FEATURES) {
        result ^= 0xF0F0;
    }
    #endif
    
    printf("Result checksum: %d\n", result & 0xFFFF);
    printf("CPU Features detected: SSE2=%d, AVX=%d, AVX2=%d, AVX512=%d\n",
           has_sse2, has_avx, has_avx2, has_avx512);
    
    return (result & 0xFF);
}
