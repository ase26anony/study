/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array_a[1024 * 1024];
static volatile int array_b[1024 * 1024];
static volatile int array_c[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These conditionals require driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__AVX__)
#define USE_CPU_BUILTINS 1
#else
#define USE_CPU_BUILTINS 0
#endif
#else
#define USE_CPU_BUILTINS 0
#endif

/* Cache-sensitive computation with variable strides */
static int compute_checksum(int stride, int iterations) {
    volatile int sum = 0;
    int i, j;
    
    /* Access pattern that depends on stride */
    for (i = 0; i < iterations; i++) {
        int idx = (i * stride) % (1024 * 1024);
        array_a[idx] = i;
        sum += array_a[idx];
        
        /* Cross-array access to potentially use different cache levels */
        if (i % 3 == 0) {
            int idx2 = (i * 2) % (512 * 512);
            array_c[idx2] = sum;
            sum ^= array_c[idx2];
        }
    }
    
    /* Second pass with different access pattern */
    for (j = iterations - 1; j >= 0; j -= stride) {
        int idx = (j * 7) % (1024 * 1024);
        array_b[idx] = j;
        sum += array_b[idx];
    }
    
    return sum;
}

/* Different computation variants based on CPU features */
static int compute_sse_variant(int stride, int iterations) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i += 4) {
        int idx = (i * stride) % (1024 * 1024);
        sum += array_a[idx] * 2;
    }
    return sum;
}

static int compute_avx_variant(int stride, int iterations) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i += 8) {
        int idx = (i * stride) % (1024 * 1024);
        sum += array_a[idx] * 3;
    }
    return sum;
}

static int compute_generic(int stride, int iterations) {
    return compute_checksum(stride, iterations);
}

int main(void) {
    volatile int checksum = 0;
    volatile int use_sse = 0;
    volatile int use_avx = 0;
    volatile int use_avx2 = 0;
    volatile int use_avx512 = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 1024 * 1024; i++) {
        array_a[i] = i & 0xFF;
        array_b[i] = (i * 3) & 0xFF;
    }
    for (int i = 0; i < 512 * 512; i++) {
        array_c[i] = (i * 5) & 0xFF;
    }
    
    /* Force CPU initialization - this triggers CPUID in driver */
    __builtin_cpu_init();
    
    /* Check various CPU features - each requires driver to examine CPUID */
    /* These cannot be optimized away due to volatile storage */
    use_sse = __builtin_cpu_supports("sse");
    use_sse |= __builtin_cpu_supports("sse2");
    use_sse |= __builtin_cpu_supports("sse3");
    use_sse |= __builtin_cpu_supports("ssse3");
    use_sse |= __builtin_cpu_supports("sse4.1");
    use_sse |= __builtin_cpu_supports("sse4.2");
    
    use_avx = __builtin_cpu_supports("avx");
    use_avx |= __builtin_cpu_supports("fma");
    
    use_avx2 = __builtin_cpu_supports("avx2");
    
    use_avx512 = __builtin_cpu_supports("avx512f");
    use_avx512 |= __builtin_cpu_supports("avx512dq");
    use_avx512 |= __builtin_cpu_supports("avx512vl");
    
    /* Choose computation based on CPU features */
    /* Volatile prevents constant folding */
    if (use_avx512) {
        func_ptr = compute_avx_variant;
    } else if (use_avx2) {
        func_ptr = compute_avx_variant;
    } else if (use_avx) {
        func_ptr = compute_avx_variant;
    } else if (use_sse) {
        func_ptr = compute_sse_variant;
    } else {
        func_ptr = compute_generic;
    }
    
    /* Variable stride based on cache line detection */
    volatile int stride = 1;
    
    /* Different strides to test various cache behaviors */
    if (use_avx512) stride = 16;
    else if (use_avx2) stride = 8;
    else if (use_avx) stride = 8;
    else if (use_sse) stride = 4;
    
    /* Perform computation */
    if (func_ptr) {
        checksum = func_ptr(stride, 10000);
        
        /* Additional passes with different strides */
        checksum += compute_checksum(stride * 2, 5000);
        checksum += compute_checksum(stride * 4, 2500);
        checksum += compute_checksum(stride * 8, 1250);
    }
    
    /* Use result to prevent dead code elimination */
    printf("CPU Feature Summary:\n");
    printf("  SSE: %d\n", use_sse);
    printf("  AVX: %d\n", use_avx);
    printf("  AVX2: %d\n", use_avx2);
    printf("  AVX512: %d\n", use_avx512);
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Compile-time CPU feature checks that force driver evaluation */
#ifdef __OPTIMIZE__
/* These macros will be evaluated during compilation */
#define CHECK_CPU_FEATURE(feat) (__builtin_cpu_supports(feat) ? 1 : 0)

/* Force evaluation of multiple CPUID leaves */
static const int compile_time_features[] = {
#if USE_CPU_BUILTINS
    CHECK_CPU_FEATURE("sse"),
    CHECK_CPU_FEATURE("sse2"),
    CHECK_CPU_FEATURE("sse3"),
    CHECK_CPU_FEATURE("ssse3"),
    CHECK_CPU_FEATURE("sse4.1"),
    CHECK_CPU_FEATURE("sse4.2"),
    CHECK_CPU_FEATURE("avx"),
    CHECK_CPU_FEATURE("avx2"),
    CHECK_CPU_FEATURE("fma"),
#ifdef __AVX512F__
    CHECK_CPU_FEATURE("avx512f"),
#endif
#endif
    0 /* sentinel */
};
#endif
