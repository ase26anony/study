/* cpu_cache_coverage.c
 * Designed to trigger GCC driver's x86 CPUID cache detection logic
 * Compile with various -march options to cover different switch cases
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int data_array[1024 * 1024];
static volatile int temp_array[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force CPU detection initialization */
__attribute__((constructor)) 
static void init_cpu_detection(void) {
    /* This runs before main, forcing early CPU detection */
    __builtin_cpu_init();
}

/* Different computation strategies based on CPU features */
static int compute_sse_strategy(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += data_array[i & 0xFFFFF] * (i % 256);
    }
    return sum;
}

static int compute_avx_strategy(int stride, int iterations) {
    int sum = 0;
    /* Different access pattern for AVX */
    for (int i = 0; i < iterations; i += stride) {
        sum += temp_array[(i * 7) & 0x7FFFF] ^ (i % 512);
    }
    return sum;
}

static int compute_generic_strategy(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += data_array[i & 0xFFFFF] + temp_array[i & 0x7FFFF];
    }
    return sum;
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#if defined(__OPTIMIZE__) && !defined(__NO_CPUID__)
/* This block requires CPU feature detection during compilation */
static int select_strategy_based_on_features(void) {
    volatile int use_sse = 0;
    volatile int use_avx = 0;
    
    /* These builtins force CPUID evaluation in the driver */
    if (__builtin_cpu_supports("sse2")) {
        use_sse = 1;
    }
    
    if (__builtin_cpu_supports("avx")) {
        use_avx = 1;
    }
    
    if (__builtin_cpu_supports("avx512f")) {
        use_avx = 2; /* Different code path for AVX-512 */
    }
    
    return use_sse + use_avx;
}
#endif

/* Another conditional block for different cache levels */
#ifdef __FAST_MATH__
static void check_cache_sensitive_features(void) {
    /* Check various CPU features that might correlate with cache configs */
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Use results to prevent dead code elimination */
    if (has_sse3) data_array[0] += 1;
    if (has_ssse3) data_array[1] += 2;
    if (has_sse4_1) data_array[2] += 3;
    if (has_sse4_2) data_array[3] += 4;
    if (has_avx2) data_array[4] += 5;
}
#endif

int main(void) {
    int result = 0;
    int stride = 1;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 1024 * 1024; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (int i = 0; i < 512 * 512; i++) {
        temp_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Determine stride based on CPU features (non-constant) */
    volatile int cpu_features = 0;
    
    /* Check multiple CPU features to ensure thorough CPUID evaluation */
    if (__builtin_cpu_supports("sse")) {
        cpu_features |= 1;
        stride = 2;
    }
    if (__builtin_cpu_supports("sse2")) {
        cpu_features |= 2;
        stride = 4;
    }
    if (__builtin_cpu_supports("sse3")) {
        cpu_features |= 4;
        stride = 8;
    }
    if (__builtin_cpu_supports("ssse3")) {
        cpu_features |= 8;
        stride = 16;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        cpu_features |= 16;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        cpu_features |= 32;
    }
    if (__builtin_cpu_supports("avx")) {
        cpu_features |= 64;
        stride = 32;
    }
    if (__builtin_cpu_supports("avx2")) {
        cpu_features |= 128;
    }
    if (__builtin_cpu_supports("avx512f")) {
        cpu_features |= 256;
        stride = 64;
    }
    
    /* Select computation strategy based on features */
    if (cpu_features & 256) { /* AVX-512 */
        func_ptr = compute_avx_strategy;
        result = compute_avx_strategy(stride, 1000000);
    } else if (cpu_features & 64) { /* AVX */
        func_ptr = compute_avx_strategy;
        result = compute_avx_strategy(stride, 500000);
    } else if (cpu_features & 1) { /* SSE */
        func_ptr = compute_sse_strategy;
        result = compute_sse_strategy(stride, 250000);
    } else {
        func_ptr = compute_generic_strategy;
        result = compute_generic_strategy(stride, 100000);
    }
    
    /* Additional cache-sensitive computation */
    int cache_test_sum = 0;
    /* Prime number stride to avoid power-of-two patterns */
    for (int i = 0; i < 100000; i += 17) {
        cache_test_sum += data_array[(i * 13) & 0xFFFFF];
    }
    
    result ^= cache_test_sum;
    
    printf("CPU Features: 0x%x\n", cpu_features);
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}

/* Additional compilation unit to force different optimizations */
#ifdef __OPTIMIZE__
/* This function uses CPU-specific intrinsics conditionally */
static void cpu_specific_operations(void) {
    volatile int has_popcnt = __builtin_cpu_supports("popcnt");
    volatile int has_aes = __builtin_cpu_supports("aes");
    volatile int has_pclmul = __builtin_cpu_supports("pclmul");
    volatile int has_fma = __builtin_cpu_supports("fma");
    volatile int has_rdrand = __builtin_cpu_supports("rdrand");
    
    /* Use the results to affect array values */
    if (has_popcnt) data_array[100] = __builtin_popcount(data_array[100]);
    if (has_aes) data_array[101] ^= 0xA5A5A5A5;
    if (has_pclmul) data_array[102] *= 3;
    if (has_fma) data_array[103] += data_array[104] * data_array[105];
    if (has_rdrand) {
        unsigned int r;
        __builtin_ia32_rdrand32_step(&r);
        data_array[106] = r;
    }
}
#endif
