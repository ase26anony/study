/*
 * CPUID Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID cache detection logic
 * during compilation with various -march options.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-aware optimizations */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
volatile compute_func_t func_ptr = NULL;

/* Force CPU initialization and feature detection */
__attribute__((noinline))
static void init_cpu_features(void) {
    /* This triggers __builtin_cpu_init in GCC */
    __builtin_cpu_init();
}

/* Different computation patterns based on CPU features */
__attribute__((noinline))
static int compute_sse_pattern(int size, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += stride) {
        array1[i] = array2[i] * 3 + array3[i % 512];
        sum += array1[i];
    }
    return sum;
}

__attribute__((noinline))
static int compute_avx_pattern(int size, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += stride * 2) {
        array1[i] = (array2[i] << 1) + (array3[i % 512] >> 1);
        array1[i + stride] = array2[i + stride] ^ array3[(i + stride) % 512];
        sum += array1[i] + array1[i + stride];
    }
    return sum;
}

__attribute__((noinline))
static int compute_basic_pattern(int size, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += i * 3 + (array1[i] & 0xFF);
    }
    return sum;
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#ifdef __OPTIMIZE__
/* This block requires CPU feature detection during compilation */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX512F__)
#define USE_ADVANCED_FEATURES 1
#else
#define USE_ADVANCED_FEATURES 0
#endif
#endif

/* Main computation that varies based on detected CPU features */
__attribute__((noinline))
static int perform_cache_sensitive_computation(void) {
    volatile int result = 0;
    volatile int has_sse2 = 0, has_avx = 0, has_avx512 = 0;
    
    /* Force CPU feature checks - these trigger CPUID in the driver */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Choose stride based on cache line assumptions */
    int stride = 1;
    
    /* Different stride patterns that might trigger cache-aware optimizations */
    if (has_avx512) {
        stride = 64;  /* AVX512 might prefer larger strides */
    } else if (has_avx) {
        stride = 32;  /* AVX stride */
    } else if (has_sse2) {
        stride = 16;  /* SSE2 stride */
    } else {
        stride = 8;   /* Basic stride */
    }
    
    /* Multiple computation patterns to use different parts of arrays */
    for (int pattern = 0; pattern < 3; pattern++) {
        switch (pattern) {
            case 0:
                result ^= compute_basic_pattern(1024 * 64, stride);
                break;
            case 1:
                if (has_sse2) {
                    result ^= compute_sse_pattern(1024 * 128, stride * 2);
                }
                break;
            case 2:
                if (has_avx) {
                    result ^= compute_avx_pattern(1024 * 256, stride * 4);
                }
                break;
        }
    }
    
    return result;
}

/* Additional architecture-specific conditional blocks */
#ifdef __tune_core2__
/* Core2-specific optimization hints */
#define CACHELINE_SIZE 64
#define L1_CACHE_SIZE 32
#endif

#ifdef __tune_nehalem__
/* Nehalem-specific optimization hints */
#define CACHELINE_SIZE 64
#define L1_CACHE_SIZE 32
#define L2_CACHE_SIZE 256
#endif

#ifdef __tune_sandybridge__
/* Sandy Bridge-specific optimization hints */
#define CACHELINE_SIZE 64
#define L1_CACHE_SIZE 32
#define L2_CACHE_SIZE 256
#define L3_CACHE_SIZE 8192
#endif

#ifdef __tune_skylake_avx512__
/* Skylake AVX512-specific optimization hints */
#define CACHELINE_SIZE 64
#define L1_CACHE_SIZE 32
#define L2_CACHE_SIZE 1024
#define L3_CACHE_SIZE 16384
#endif

/* Main function with checksum to prevent dead code elimination */
int main(void) {
    /* Initialize CPU features - triggers driver's CPUID */
    init_cpu_features();
    
    /* Perform cache-sensitive computation */
    volatile int checksum = perform_cache_sensitive_computation();
    
    /* Use the result to prevent optimization */
    printf("CPU Cache Test Checksum: %d\n", checksum & 0xFF);
    
    /* Additional CPU feature reporting that requires detection */
#ifdef __OPTIMIZE__
    printf("Compiled with optimizations\n");
    
    /* These conditionals force the driver to evaluate CPU features */
    #if USE_ADVANCED_FEATURES
    printf("Advanced CPU features available\n");
    #endif
    
    /* Architecture-specific reporting */
    #ifdef __tune_core2__
    printf("Target architecture: Core2\n");
    #elif defined(__tune_nehalem__)
    printf("Target architecture: Nehalem\n");
    #elif defined(__tune_sandybridge__)
    printf("Target architecture: Sandy Bridge\n");
    #elif defined(__tune_skylake_avx512__)
    printf("Target architecture: Skylake AVX512\n");
    #elif defined(__tune_native__)
    printf("Target architecture: Native\n");
    #endif
#endif
    
    return 0;
}
