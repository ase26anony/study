/* cache_detection_test.c
 * 
 * This program triggers GCC's CPUID-based cache detection during compilation.
 * The driver-i386.cc switch statement (lines 127-244) is executed when GCC
 * initializes CPU detection for optimization decisions.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-aware optimizations */
static volatile int array1[1024 * 1024];  /* 4MB */
static volatile int array2[512 * 512];    /* 1MB */
static volatile int array3[256 * 256];    /* 256KB */

/* Function pointers to prevent optimization */
typedef int (*ComputeFunc)(int, int);
static ComputeFunc func_ptr = NULL;

/* Volatile results to prevent constant folding */
volatile int cpu_feature_results[8];

/* Initialize CPU detection - forces driver to execute CPUID */
__attribute__((constructor)) 
static void init_cpu_detection(void) {
    /* This constructor runs before main, triggering early CPU detection */
    __builtin_cpu_init();
}

/* Different computation patterns based on CPU features */
static int compute_sse2(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += array1[i & 0xFFFFF] * 3;
    }
    return sum;
}

static int compute_avx(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += array2[i & 0x3FFFF] * 7;
    }
    return sum;
}

static int compute_avx512(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += array3[i & 0xFFFF] * 11;
    }
    return sum;
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#if defined(__OPTIMIZE__) && (__OPTIMIZE__ > 0)
/* This block is only compiled with optimization, forcing driver to 
   consider cache parameters for optimization decisions */
#define USE_CACHE_AWARE_LOOPS 1
#else
#define USE_CACHE_AWARE_LOOPS 0
#endif

#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX512F__)
/* Force evaluation of CPU features during compilation */
#define CHECK_CPU_FEATURES 1
#else
#define CHECK_CPU_FEATURES 0
#endif

int main(void) {
    int checksum = 0;
    int stride = 1;
    
    /* Force CPU initialization if not already done */
    __builtin_cpu_init();
    
    /* Check various CPU features - each requires driver to examine CPUID */
    cpu_feature_results[0] = __builtin_cpu_supports("sse2");
    cpu_feature_results[1] = __builtin_cpu_supports("sse3");
    cpu_feature_results[2] = __builtin_cpu_supports("ssse3");
    cpu_feature_results[3] = __builtin_cpu_supports("sse4.1");
    cpu_feature_results[4] = __builtin_cpu_supports("sse4.2");
    cpu_feature_results[5] = __builtin_cpu_supports("avx");
    cpu_feature_results[6] = __builtin_cpu_supports("avx2");
    cpu_feature_results[7] = __builtin_cpu_supports("avx512f");
    
    /* Choose stride based on cache line size hints */
    if (cpu_feature_results[7]) {  /* AVX512 */
        stride = 16;  /* 64-byte cache lines */
        func_ptr = compute_avx512;
    } else if (cpu_feature_results[5]) {  /* AVX */
        stride = 8;   /* 32-byte cache lines */
        func_ptr = compute_avx;
    } else if (cpu_feature_results[0]) {  /* SSE2 */
        stride = 4;   /* 16-byte cache lines */
        func_ptr = compute_sse2;
    } else {
        stride = 1;
        func_ptr = compute_sse2;
    }
    
    /* Cache-sensitive loop patterns */
#if USE_CACHE_AWARE_LOOPS
    /* These loops are designed to be cache-sensitive, encouraging
       the compiler to consider cache parameters during optimization */
    
    /* Pattern 1: Sequential access with stride */
    for (int i = 0; i < 1000000; i += stride) {
        array1[i & 0xFFFFF] = i * 3;
    }
    
    /* Pattern 2: Reverse stride access */
    for (int i = 999999; i >= 0; i -= stride) {
        array2[i & 0x3FFFF] = i * 7;
    }
    
    /* Pattern 3: Random-like access pattern */
    for (int i = 0; i < 500000; i++) {
        int idx = (i * 97) & 0xFFFF;  /* Pseudo-random pattern */
        array3[idx] = i * 11;
    }
#endif
    
    /* Compute checksum using selected function */
    if (func_ptr) {
        checksum = func_ptr(stride, 100000);
    }
    
    /* Additional CPU feature checks in conditional compilation */
#if CHECK_CPU_FEATURES
    /* These checks force the driver to fully initialize CPU detection */
    if (__builtin_cpu_supports("popcnt")) {
        checksum ^= 0x55555555;
    }
    if (__builtin_cpu_supports("bmi")) {
        checksum ^= 0xAAAAAAAA;
    }
    if (__builtin_cpu_supports("lzcnt")) {
        checksum ^= 0x33333333;
    }
#endif
    
    /* Architecture-specific optimizations */
#ifdef __tune_core2__
    /* Core2-specific cache pattern */
    for (int i = 0; i < 10000; i += 2) {
        checksum += array1[i & 0xFFF];
    }
#endif
    
#ifdef __tune_nehalem__
    /* Nehalem-specific cache pattern */
    for (int i = 0; i < 10000; i += 4) {
        checksum += array2[i & 0x7FF];
    }
#endif
    
#ifdef __tune_sandybridge__
    /* Sandy Bridge-specific cache pattern */
    for (int i = 0; i < 10000; i += 8) {
        checksum += array3[i & 0x3FF];
    }
#endif
    
#ifdef __tune_skylake_avx512__
    /* Skylake AVX512-specific cache pattern */
    for (int i = 0; i < 10000; i += 16) {
        checksum += array1[i & 0x1FF] + array2[i & 0x1FF];
    }
#endif
    
    printf("CPU Feature Checksum: %d\n", checksum);
    printf("Stride used: %d\n", stride);
    
    /* Print detected features */
    printf("Detected CPU features:\n");
    const char* features[] = {
        "sse2", "sse3", "ssse3", "sse4.1", 
        "sse4.2", "avx", "avx2", "avx512f"
    };
    
    for (int i = 0; i < 8; i++) {
        if (cpu_feature_results[i]) {
            printf("  %s: YES\n", features[i]);
        }
    }
    
    return 0;
}
