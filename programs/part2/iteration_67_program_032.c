/*
 * Test program to trigger GCC driver's CPUID-based cache detection
 * Targets uncovered switch cases in driver-i386.cc lines 127-244
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache-aware optimizations */
#define ARRAY_SIZE (1024*1024)
static volatile int data_array[ARRAY_SIZE];
static volatile int temp_array[ARRAY_SIZE/2];

/* Volatile variables to prevent optimization */
static volatile int cpu_feature_flags = 0;
static volatile int cache_sensitive_result = 0;

/* Function pointers to prevent constant folding */
typedef void (*cache_op_func_t)(int, int);
static cache_op_func_t cache_op_func = NULL;

/* Different cache access patterns */
void cache_access_pattern1(int stride, int iterations) {
    volatile int* ptr = data_array;
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += ptr[i];
        ptr[i] = sum & 0xFF;
    }
    cache_sensitive_result ^= sum;
}

void cache_access_pattern2(int stride, int iterations) {
    volatile int* ptr = data_array;
    volatile int* dst = temp_array;
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        dst[i] = ptr[i] * 3 + 7;
        sum += dst[i];
    }
    cache_sensitive_result ^= sum;
}

void cache_access_pattern3(int stride, int iterations) {
    volatile int* ptr = data_array;
    int sum = 0;
    /* Non-linear access pattern */
    for (int i = 0; i < iterations; i += stride) {
        int idx = (i * 13) % ARRAY_SIZE;
        sum += ptr[idx];
        ptr[idx] = (ptr[idx] * 2) & 0xFF;
    }
    cache_sensitive_result ^= sum;
}

/* Initialize CPU detection - forces driver to execute CPUID */
__attribute__((noinline))
static void force_cpuid_detection(void) {
    /* This function's main purpose is to force CPUID initialization */
    __builtin_cpu_init();
    
    /* Check various CPU features to trigger different CPUID leaves */
    cpu_feature_flags = 0;
    
    /* These checks force the driver to examine CPUID information */
    if (__builtin_cpu_supports("sse")) cpu_feature_flags |= 1;
    if (__builtin_cpu_supports("sse2")) cpu_feature_flags |= 2;
    if (__builtin_cpu_supports("sse3")) cpu_feature_flags |= 4;
    if (__builtin_cpu_supports("ssse3")) cpu_feature_flags |= 8;
    if (__builtin_cpu_supports("sse4.1")) cpu_feature_flags |= 16;
    if (__builtin_cpu_supports("sse4.2")) cpu_feature_flags |= 32;
    if (__builtin_cpu_supports("avx")) cpu_feature_flags |= 64;
    if (__builtin_cpu_supports("avx2")) cpu_feature_flags |= 128;
    if (__builtin_cpu_supports("avx512f")) cpu_feature_flags |= 256;
    
    /* Cache-specific features */
    if (__builtin_cpu_supports("clflushopt")) cpu_feature_flags |= 512;
    if (__builtin_cpu_supports("clwb")) cpu_feature_flags |= 1024;
    
    /* Choose cache operation pattern based on detected features */
    if (cpu_feature_flags & 256) { /* AVX-512 */
        cache_op_func = cache_access_pattern3;
    } else if (cpu_feature_flags & 128) { /* AVX2 */
        cache_op_func = cache_access_pattern2;
    } else {
        cache_op_func = cache_access_pattern1;
    }
}

/* Conditional compilation blocks that reference CPU builtins */
/* These force the driver to evaluate CPUID during compilation */

#ifdef __SSE__
static volatile int sse_detected = 0;
__attribute__((constructor))
static void init_sse_flag(void) {
    sse_detected = __builtin_cpu_supports("sse") ? 1 : 0;
}
#endif

#ifdef __SSE2__
static volatile int sse2_detected = 0;
__attribute__((constructor))
static void init_sse2_flag(void) {
    sse2_detected = __builtin_cpu_supports("sse2") ? 1 : 0;
}
#endif

#ifdef __AVX__
static volatile int avx_detected = 0;
__attribute__((constructor))
static void init_avx_flag(void) {
    avx_detected = __builtin_cpu_supports("avx") ? 1 : 0;
}
#endif

#ifdef __AVX2__
static volatile int avx2_detected = 0;
__attribute__((constructor))
static void init_avx2_flag(void) {
    avx2_detected = __builtin_cpu_supports("avx2") ? 1 : 0;
}
#endif

/* Architecture-specific conditional blocks */
#if defined(__tune_core2__) || defined(__tune_nehalem__)
static volatile int old_arch_detected = 0;
__attribute__((constructor))
static void init_old_arch_flag(void) {
    /* Force CPUID evaluation for older architectures */
    old_arch_detected = __builtin_cpu_supports("sse4.1") ? 1 : 0;
}
#endif

#if defined(__tune_sandybridge__) || defined(__tune_ivybridge__)
static volatile int sb_arch_detected = 0;
__attribute__((constructor))
static void init_sb_arch_flag(void) {
    sb_arch_detected = __builtin_cpu_supports("avx") ? 1 : 0;
}
#endif

#if defined(__tune_skylake__) || defined(__tune_skylake_avx512__)
static volatile int skl_arch_detected = 0;
__attribute__((constructor))
static void init_skl_arch_flag(void) {
    skl_arch_detected = __builtin_cpu_supports("avx512f") ? 1 : 0;
}
#endif

/* Main function with cache-sensitive operations */
int main(void) {
    /* Initialize data arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = (i * 3) & 0xFF;
    }
    
    /* Force CPUID detection - this triggers the driver's cache detection */
    force_cpuid_detection();
    
    /* Perform cache-sensitive operations based on detected CPU */
    int stride = 1;
    int iterations = ARRAY_SIZE / 4;
    
    /* Choose stride based on CPU features (simulating cache-aware code) */
    if (cpu_feature_flags & 256) { /* AVX-512 - assume larger caches */
        stride = 16;
        iterations = ARRAY_SIZE / 8;
    } else if (cpu_feature_flags & 128) { /* AVX2 */
        stride = 8;
        iterations = ARRAY_SIZE / 4;
    } else if (cpu_feature_flags & 64) { /* AVX */
        stride = 4;
        iterations = ARRAY_SIZE / 2;
    } else { /* SSE or older */
        stride = 2;
        iterations = ARRAY_SIZE;
    }
    
    /* Execute cache-sensitive operation */
    if (cache_op_func) {
        cache_op_func(stride, iterations);
    }
    
    /* Additional conditional compilation that forces driver evaluation */
    #ifdef __OPTIMIZE__
    /* This block is only compiled with optimizations, forcing driver
       to evaluate CPUID to determine if optimizations should consider
       cache parameters */
    if (__builtin_cpu_supports("sse2")) {
        /* Use SSE2-optimized path */
        for (int i = 0; i < 1000; i += 4) {
            data_array[i] = data_array[i] * 2 + 1;
        }
    }
    #endif
    
    /* Use the result to prevent dead code elimination */
    printf("CPU Feature Flags: %d\n", cpu_feature_flags);
    printf("Cache Sensitive Result: %d\n", cache_sensitive_result);
    
    /* Compute checksum to ensure all operations are executed */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += data_array[i * 1024];
    }
    printf("Final Checksum: %d\n", checksum);
    
    return 0;
}
