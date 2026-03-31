/*
 * CPUID Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march and -mtune options.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-aware optimizations */
static volatile int array1[1024 * 1024];  /* 4MB */
static volatile int array2[1024 * 1024];  /* 4MB */

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int stride, int iterations);
static compute_func_t compute_func = NULL;

/* Volatile results to prevent constant folding */
volatile int cpu_sse2 = 0;
volatile int cpu_sse4 = 0;
volatile int cpu_avx = 0;
volatile int cpu_avx2 = 0;
volatile int cpu_avx512 = 0;
volatile int cpu_fma = 0;
volatile int cpu_aes = 0;
volatile int cpu_pclmul = 0;

/* Different computation patterns based on CPU features */
int compute_basic(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        array1[i] = array2[i] + i;
        sum += array1[i];
    }
    return sum;
}

int compute_sse_optimized(int stride, int iterations) {
    int sum = 0;
    /* Simulate SSE-friendly access pattern */
    for (int i = 0; i < iterations; i += stride * 4) {
        array1[i] = array2[i] * 2;
        array1[i + stride] = array2[i + stride] * 3;
        array1[i + stride * 2] = array2[i + stride * 2] * 4;
        array1[i + stride * 3] = array2[i + stride * 3] * 5;
        sum += array1[i] + array1[i + stride] + 
               array1[i + stride * 2] + array1[i + stride * 3];
    }
    return sum;
}

int compute_avx_optimized(int stride, int iterations) {
    int sum = 0;
    /* Simulate AVX-friendly access pattern */
    for (int i = 0; i < iterations; i += stride * 8) {
        for (int j = 0; j < 8; j++) {
            array1[i + stride * j] = array2[i + stride * j] * (j + 2);
            sum += array1[i + stride * j];
        }
    }
    return sum;
}

/* 
 * Conditional compilation blocks that force the driver to evaluate
 * __builtin_cpu_supports() during compilation
 */

#ifdef __OPTIMIZE__
/* This block requires CPU feature detection at compile time */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX512F__)
static void init_cpu_features(void) {
    __builtin_cpu_init();
    
    /* These calls trigger CPUID execution in the GCC driver */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    cpu_avx512 = __builtin_cpu_supports("avx512f");
    cpu_fma = __builtin_cpu_supports("fma");
    cpu_aes = __builtin_cpu_supports("aes");
    cpu_pclmul = __builtin_cpu_supports("pclmul");
    
    /* Select computation function based on detected features */
    if (cpu_avx512) {
        compute_func = compute_avx_optimized;
    } else if (cpu_avx) {
        compute_func = compute_avx_optimized;
    } else if (cpu_sse2) {
        compute_func = compute_sse_optimized;
    } else {
        compute_func = compute_basic;
    }
}
#else
/* Fallback for non-x86 or minimal builds */
static void init_cpu_features(void) {
    compute_func = compute_basic;
    cpu_sse2 = 0;
    cpu_sse4 = 0;
    cpu_avx = 0;
}
#endif /* __SSE2__ || __AVX__ || __AVX512F__ */
#else
/* For -O0 builds, still initialize but with basic function */
static void init_cpu_features(void) {
    compute_func = compute_basic;
}
#endif /* __OPTIMIZE__ */

/* Cache-sensitive memory access patterns */
static int cache_sensitive_operation(int base_stride) {
    int result = 0;
    const int iterations = 1024 * 256;  /* 1MB worth of ints */
    
    /* Vary stride based on CPU features to test different cache behaviors */
    int stride = base_stride;
    
    if (cpu_avx) stride *= 2;
    if (cpu_avx512) stride *= 4;
    
    /* Prime the arrays */
    for (int i = 0; i < 1024; i++) {
        array1[i] = i;
        array2[i] = 1024 - i;
    }
    
    /* Perform computation with selected function */
    if (compute_func) {
        result = compute_func(stride, iterations);
    }
    
    /* Additional cache-thrashing pattern */
    for (int i = 0; i < iterations; i += 64) {  /* Cache line sized jumps */
        array1[i % 1024] ^= array2[(i + 32) % 1024];
    }
    
    return result;
}

/* Architecture-specific compilation triggers */
#if defined(__tune_znver1__) || defined(__tune_znver2__) || defined(__tune_znver3__)
/* AMD Zen architecture specific path */
static int amd_zen_optimization(void) {
    int sum = 0;
    /* Zen prefers specific access patterns */
    for (int i = 0; i < 8192; i += 16) {
        sum += array1[i] - array2[i];
    }
    return sum;
}
#endif

#if defined(__tune_core2__) || defined(__tune_nehalem__) || defined(__tune_sandybridge__)
/* Intel Core/Nehalem/Sandy Bridge specific path */
static int intel_core_optimization(void) {
    int sum = 0;
    /* Different stride for older Intel architectures */
    for (int i = 0; i < 8192; i += 8) {
        sum += array1[i] | array2[i];
    }
    return sum;
}
#endif

#if defined(__tune_skylake__) || defined(__tune_skylake_avx512__) || defined(__tune_icelake__)
/* Intel Skylake+ specific path */
static int intel_skylake_optimization(void) {
    int sum = 0;
    /* AVX-512 friendly pattern */
    for (int i = 0; i < 8192; i += 64) {
        sum += array1[i] ^ array2[i];
    }
    return sum;
}
#endif

int main(void) {
    int checksum = 0;
    
    /* Initialize CPU features - triggers CPUID in GCC driver */
    init_cpu_features();
    
    /* Perform cache-sensitive operations */
    checksum += cache_sensitive_operation(1);
    checksum += cache_sensitive_operation(2);
    checksum += cache_sensitive_operation(4);
    checksum += cache_sensitive_operation(8);
    checksum += cache_sensitive_operation(16);
    
    /* Architecture-specific computations */
#if defined(__tune_znver1__) || defined(__tune_znver2__) || defined(__tune_znver3__)
    checksum += amd_zen_optimization();
#endif
    
#if defined(__tune_core2__) || defined(__tune_nehalem__) || defined(__tune_sandybridge__)
    checksum += intel_core_optimization();
#endif
    
#if defined(__tune_skylake__) || defined(__tune_skylake_avx512__) || defined(__tune_icelake__)
    checksum += intel_skylake_optimization();
#endif
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum % 1000;
    
    printf("CPU Feature Checksum: %d\n", final_result);
    printf("Features detected: SSE2=%d, AVX=%d, AVX512=%d\n", 
           cpu_sse2, cpu_avx, cpu_avx512);
    
    return final_result != 0;
}
