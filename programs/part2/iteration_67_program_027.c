/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define USE_CPU_BUILTINS 1
#else
#define USE_CPU_BUILTINS 0
#endif

/* Architecture-specific conditionals to trigger different CPUID paths */
#if defined(__i386__) || defined(__x86_64__)
#define X86_ARCH 1
#else
#define X86_ARCH 0
#endif

/* Large array to encourage cache consideration */
static volatile int cache_array[1024 * 1024];

/* Function pointer to prevent optimization */
typedef void (*cpu_func_t)(int*, size_t);
static cpu_func_t func_ptr = NULL;

/* Different loop patterns based on CPU features */
static void process_array_sse(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 16) {
        sum += arr[i];
    }
    cache_array[0] = sum;
}

static void process_array_avx(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 32) {
        sum += arr[i] * 2;
    }
    cache_array[1] = sum;
}

static void process_array_avx512(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 64) {
        sum += arr[i] * 3;
    }
    cache_array[2] = sum;
}

/* Main function with CPU feature checks */
int main(void) {
    /* Initialize CPU detection - this triggers driver's __builtin_cpu_init */
    __builtin_cpu_init();
    
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* Check multiple CPU features to ensure driver evaluates CPUID */
#if USE_CPU_BUILTINS && X86_ARCH
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
#endif
    
    /* Use volatile results to prevent optimization */
    volatile int feature_mask = 0;
    feature_mask |= has_sse2 ? 1 : 0;
    feature_mask |= has_sse3 ? 2 : 0;
    feature_mask |= has_ssse3 ? 4 : 0;
    feature_mask |= has_sse4_1 ? 8 : 0;
    feature_mask |= has_sse4_2 ? 16 : 0;
    feature_mask |= has_avx ? 32 : 0;
    feature_mask |= has_avx2 ? 64 : 0;
    feature_mask |= has_avx512f ? 128 : 0;
    
    /* Select function based on CPU features */
    if (has_avx512f) {
        func_ptr = process_array_avx512;
    } else if (has_avx2) {
        func_ptr = process_array_avx;
    } else if (has_sse2) {
        func_ptr = process_array_sse;
    }
    
    /* Process array with different strides based on cache considerations */
    volatile int result = 0;
    size_t array_size = sizeof(cache_array) / sizeof(cache_array[0]);
    
    /* Multiple access patterns to trigger cache-aware optimizations */
    for (int stride = 1; stride <= 16; stride *= 2) {
        volatile int local_sum = 0;
        for (size_t i = 0; i < array_size; i += stride) {
            local_sum += cache_array[i];
        }
        result ^= local_sum;
    }
    
    /* Call selected function if any */
    if (func_ptr) {
        func_ptr((int*)cache_array, array_size);
    }
    
    printf("CPU Feature Mask: %d\n", feature_mask);
    printf("Result checksum: %d\n", result);
    
    return 0;
}

/* Additional compilation-only code blocks */
#if defined(__SSE2__) && USE_CPU_BUILTINS
/* This block requires driver to evaluate __builtin_cpu_supports */
static int __attribute__((used)) check_sse2(void) {
    return __builtin_cpu_supports("sse2") ? 1 : 0;
}
#endif

#if defined(__AVX__) && USE_CPU_BUILTINS
static int __attribute__((used)) check_avx(void) {
    return __builtin_cpu_supports("avx") ? 1 : 0;
}
#endif

#if defined(__AVX2__) && USE_CPU_BUILTINS
static int __attribute__((used)) check_avx2(void) {
    return __builtin_cpu_supports("avx2") ? 1 : 0;
}
#endif

#if defined(__AVX512F__) && USE_CPU_BUILTINS
static int __attribute__((used)) check_avx512f(void) {
    return __builtin_cpu_supports("avx512f") ? 1 : 0;
}
#endif
