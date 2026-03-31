/* test_cpuid_cache.c - Triggers GCC driver's CPUID-based cache detection */
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

/* Force CPU initialization and feature detection */
__attribute__((noinline)) 
static int detect_cpu_features(void) {
    volatile int features = 0;
    
    /* This forces __builtin_cpu_init() */
    __builtin_cpu_init();
    
    /* Check various CPU features - each requires CPUID evaluation */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    if (__builtin_cpu_supports("avx512f")) features |= 256;
    
    /* Cache-specific features */
    if (__builtin_cpu_supports("clflushopt")) features |= 512;
    if (__builtin_cpu_supports("clwb")) features |= 1024;
    
    return features;
}

/* Different computation patterns based on CPU features */
__attribute__((noinline))
static int compute_pattern_a(int size, int stride) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        array1[i] = array2[i] + array3[i % 8192];
        sum += array1[i];
    }
    return sum;
}

__attribute__((noinline))
static int compute_pattern_b(int size, int stride) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        array2[i] = array1[i] * 3 - array3[i % 8192];
        sum += array2[i];
    }
    return sum;
}

__attribute__((noinline))
static int compute_pattern_c(int size, int stride) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        array3[i % 8192] = (array1[i] + array2[i]) / 2;
        sum += array3[i % 8192];
    }
    return sum;
}

/* Conditional compilation blocks that force driver evaluation */
#ifdef __OPTIMIZE__
#if defined(__SSE__) || defined(__AVX__) || defined(__AVX512F__)
/* This block references CPU builtins, forcing driver to evaluate them */
static volatile int force_cpuid_eval = 
    (__builtin_cpu_supports("sse") ? 1 : 0) +
    (__builtin_cpu_supports("avx") ? 2 : 0) +
    (__builtin_cpu_supports("avx512f") ? 4 : 0);
#endif
#endif

#ifdef __FAST_MATH__
/* Another conditional that might trigger cache detection */
static volatile int math_features =
    (__builtin_cpu_supports("sse") && __builtin_cpu_supports("sse2")) ? 1 : 0;
#endif

int main(void) {
    volatile int cpu_features;
    volatile int result = 0;
    volatile int stride = 1;
    
    /* Initialize arrays */
    for (int i = 0; i < 1024 * 1024; i++) {
        array1[i] = i % 256;
        array2[i] = (i * 3) % 256;
    }
    for (int i = 0; i < 512 * 512; i++) {
        array3[i] = (i * 5) % 256;
    }
    
    /* Detect CPU features - triggers CPUID in driver */
    cpu_features = detect_cpu_features();
    
    /* Choose stride based on detected features */
    if (cpu_features & 64) { /* AVX */
        stride = 8;
        func_ptr = compute_pattern_a;
    } else if (cpu_features & 16) { /* SSE4.1 */
        stride = 4;
        func_ptr = compute_pattern_b;
    } else if (cpu_features & 2) { /* SSE2 */
        stride = 2;
        func_ptr = compute_pattern_c;
    } else {
        stride = 1;
        func_ptr = compute_pattern_a;
    }
    
    /* Perform computation with chosen pattern */
    if (func_ptr) {
        result = func_ptr(1024 * 1024, stride);
    }
    
    /* Additional computation to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 1000; i += 16) {
        checksum += array1[i] + array2[i * 2] + array3[i % 512];
    }
    
    printf("CPU Features: %d\n", cpu_features);
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Architecture-specific compilation blocks */
#if defined(__i386__) || defined(__x86_64__)
/* Force evaluation of cache-related builtins if available */
#ifdef __GNUC__
#if __GNUC__ > 4
static void __attribute__((constructor)) init_cache_detection(void) {
    /* This constructor runs before main, potentially triggering 
       early CPU detection in the driver */
    volatile int dummy = __builtin_cpu_supports("sse") ? 1 : 0;
    (void)dummy;
}
#endif
#endif
#endif
