/*
 * CPU Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march and -mtune options.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
#define ARRAY_SIZE (1024 * 1024)
static int data_array[ARRAY_SIZE];
static int temp_array[ARRAY_SIZE];

/* Volatile variables to prevent optimization */
volatile int cpu_feature_sse2 = 0;
volatile int cpu_feature_sse4 = 0;
volatile int cpu_feature_avx = 0;
volatile int cpu_feature_avx2 = 0;
volatile int cpu_feature_avx512 = 0;
volatile int cpu_feature_fma = 0;

/* Function pointers to prevent constant folding */
typedef int (*ComputeFunc)(int*, int*, size_t, int);
volatile ComputeFunc compute_func = NULL;

/* Different computation patterns with varying strides */
int compute_stride_1(int* src, int* dst, size_t size, int seed) {
    int result = seed;
    for (size_t i = 0; i < size; i += 1) {
        dst[i] = src[i] * 3 + result;
        result ^= dst[i];
    }
    return result;
}

int compute_stride_2(int* src, int* dst, size_t size, int seed) {
    int result = seed;
    for (size_t i = 0; i < size; i += 2) {
        dst[i] = src[i] * 5 + result;
        result ^= dst[i];
    }
    return result;
}

int compute_stride_4(int* src, int* dst, size_t size, int seed) {
    int result = seed;
    for (size_t i = 0; i < size; i += 4) {
        dst[i] = src[i] * 7 + result;
        result ^= dst[i];
    }
    return result;
}

int compute_stride_8(int* src, int* dst, size_t size, int seed) {
    int result = seed;
    for (size_t i = 0; i < size; i += 8) {
        dst[i] = src[i] * 11 + result;
        result ^= dst[i];
    }
    return result;
}

int compute_stride_16(int* src, int* dst, size_t size, int seed) {
    int result = seed;
    for (size_t i = 0; i < size; i += 16) {
        dst[i] = src[i] * 13 + result;
        result ^= dst[i];
    }
    return result;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

/* 
 * Conditional compilation blocks that force driver to evaluate __builtin_cpu_supports
 * These are evaluated during compilation, triggering cache detection
 */

#ifdef __OPTIMIZE__
/* This block forces driver to check SSE2 support */
#if defined(__SSE2__) || defined(__SSE2_MATH__)
#define FORCE_SSE2_CHECK 1
#else
#define FORCE_SSE2_CHECK 0
#endif
#endif

#ifdef __OPTIMIZE__
/* Force AVX check if AVX is enabled via -mavx */
#if defined(__AVX__) || defined(__AVX2__)
#define FORCE_AVX_CHECK 1
#else
#define FORCE_AVX_CHECK 0
#endif
#endif

/* Main function with CPU feature detection */
int main(void) {
    int checksum = 0;
    
    /* Initialize CPU detection - triggers driver's CPUID logic */
    __builtin_cpu_init();
    
    /* 
     * Check CPU features - each call may trigger cache detection
     * Use volatile to prevent optimization
     */
    cpu_feature_sse2 = __builtin_cpu_supports("sse2");
    cpu_feature_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_feature_avx = __builtin_cpu_supports("avx");
    cpu_feature_avx2 = __builtin_cpu_supports("avx2");
    cpu_feature_avx512 = __builtin_cpu_supports("avx512f");
    cpu_feature_fma = __builtin_cpu_supports("fma");
    
    /* Initialize arrays */
    init_arrays();
    
    /* Choose computation function based on CPU features */
    if (cpu_feature_avx512) {
        compute_func = compute_stride_16;  /* Large stride for AVX512 */
    } else if (cpu_feature_avx2) {
        compute_func = compute_stride_8;   /* Medium stride for AVX2 */
    } else if (cpu_feature_avx) {
        compute_func = compute_stride_4;   /* Smaller stride for AVX */
    } else if (cpu_feature_sse4) {
        compute_func = compute_stride_2;   /* Even smaller for SSE4 */
    } else {
        compute_func = compute_stride_1;   /* Default stride */
    }
    
    /* Perform computation */
    if (compute_func != NULL) {
        checksum = compute_func(data_array, temp_array, ARRAY_SIZE, 42);
    }
    
    /* Additional conditional compilation that references CPU builtins */
#ifdef __OPTIMIZE__
    /* This will be evaluated during compilation */
    if (__builtin_cpu_supports("sse2")) {
        checksum ^= 0x55AA55AA;
    }
#endif
    
#ifdef __FAST_MATH__
    /* Another driver evaluation point */
    if (__builtin_cpu_supports("avx")) {
        checksum ^= 0xAA55AA55;
    }
#endif
    
    /* Print result to prevent dead code elimination */
    printf("CPU Cache Test Checksum: 0x%08X\n", checksum);
    
    /* Print detected features */
    printf("Detected CPU Features:\n");
    printf("  SSE2:    %s\n", cpu_feature_sse2 ? "Yes" : "No");
    printf("  SSE4.2:  %s\n", cpu_feature_sse4 ? "Yes" : "No");
    printf("  AVX:     %s\n", cpu_feature_avx ? "Yes" : "No");
    printf("  AVX2:    %s\n", cpu_feature_avx2 ? "Yes" : "No");
    printf("  AVX512F: %s\n", cpu_feature_avx512 ? "Yes" : "No");
    printf("  FMA:     %s\n", cpu_feature_fma ? "Yes" : "No");
    
    return 0;
}

/* 
 * Architecture-specific compilation blocks
 * These force the driver to consider different CPU architectures
 */

/* Core 2 Duo/Quad specific optimizations */
#if defined(__tune_core2__) || defined(__arch_core2__)
__attribute__((target("sse3")))
void core2_optimized(void) {
    /* Empty function, but target attribute triggers architecture consideration */
}
#endif

/* Nehalem specific */
#if defined(__tune_nehalem__) || defined(__arch_nehalem__)
__attribute__((target("sse4.2")))
void nehalem_optimized(void) {
    /* Triggers driver to consider Nehalem cache topology */
}
#endif

/* Sandy Bridge specific */
#if defined(__tune_sandybridge__) || defined(__arch_sandybridge__)
__attribute__((target("avx")))
void sandybridge_optimized(void) {
    /* Triggers driver to consider Sandy Bridge cache topology */
}
#endif

/* Skylake specific */
#if defined(__tune_skylake__) || defined(__arch_skylake__)
__attribute__((target("avx2")))
void skylake_optimized(void) {
    /* Triggers driver to consider Skylake cache topology */
}
#endif

/* Zen specific (AMD) */
#if defined(__tune_znver1__) || defined(__arch_znver1__)
__attribute__((target("avx2,fma")))
void zen_optimized(void) {
    /* Triggers driver to consider Zen cache topology */
}
#endif
