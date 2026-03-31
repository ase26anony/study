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

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static compute_func_t func_ptr = NULL;

/* CPU feature checks that force driver to initialize CPU detection */
static void init_cpu_features(void) {
    /* This triggers __builtin_cpu_init in the driver */
    __builtin_cpu_init();
    
    /* Store results in volatile to prevent constant folding */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Use the results to affect code generation */
    if (has_sse2) {
        func_ptr = &compute_sse2;
    }
    if (has_avx) {
        func_ptr = &compute_avx;
    }
    if (has_avx512f) {
        func_ptr = &compute_avx512;
    }
    
    (void)has_sse2; (void)has_sse3; (void)has_ssse3;
    (void)has_sse4_1; (void)has_sse4_2; (void)has_avx;
    (void)has_avx2; (void)has_avx512f;
}

/* Different computation functions for different CPU features */
static int compute_sse2(int a, int b) {
    return a + b;
}

static int compute_avx(int a, int b) {
    return a * b - (a + b);
}

static int compute_avx512(int a, int b) {
    return (a ^ b) + (a & b) * 2;
}

/* Cache-sensitive memory access patterns */
static uint64_t cache_sensitive_access(int stride, int iterations) {
    uint64_t sum = 0;
    volatile int* arr = array1;
    
    /* Non-constant stride to prevent optimization */
    for (int i = 0; i < iterations; i += stride) {
        sum += arr[i % (1024 * 1024)];
        arr[i % (1024 * 1024)] = i;
    }
    
    /* Cross-array access to potentially use L2/L3 cache */
    for (int i = 0; i < iterations / 2; i += stride * 2) {
        int idx = i % (512 * 512);
        array2[idx] = array3[idx] + 1;
        sum += array2[idx];
    }
    
    return sum;
}

/* Conditional compilation based on CPU features */
#if USE_CPU_BUILTINS && defined(__SSE2__)
static void sse2_optimized_loop(void) {
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    if (has_sse2) {
        for (int i = 0; i < 1000; i++) {
            array1[i] = array1[i] * 2 + 1;
        }
    }
}
#endif

#if USE_CPU_BUILTINS && defined(__AVX__)
static void avx_optimized_loop(void) {
    volatile int has_avx = __builtin_cpu_supports("avx");
    if (has_avx) {
        for (int i = 0; i < 2000; i += 4) {
            array2[i] = array2[i] * 3 - 2;
        }
    }
}
#endif

#if USE_CPU_BUILTINS && defined(__AVX512F__)
static void avx512_optimized_loop(void) {
    volatile int has_avx512 = __builtin_cpu_supports("avx512f");
    if (has_avx512) {
        for (int i = 0; i < 4000; i += 8) {
            array3[i / 8] = array3[i / 8] ^ 0x5A5A5A5A;
        }
    }
}
#endif

/* Main function with checksum computation */
int main(int argc, char** argv) {
    uint64_t checksum = 0;
    
    /* Initialize CPU detection - triggers driver path */
    init_cpu_features();
    
    /* Different access patterns based on compilation flags */
    int stride = 1;
    
#if defined(__OPTIMIZE__)
    /* Use different strides for different optimization levels */
    stride = (__builtin_cpu_supports("sse2") ? 2 : 1);
    if (__builtin_cpu_supports("avx")) stride = 4;
    if (__builtin_cpu_supports("avx512f")) stride = 8;
#endif
    
    /* Perform cache-sensitive operations */
    checksum += cache_sensitive_access(stride, 100000);
    
    /* Execute feature-specific optimized loops */
#if USE_CPU_BUILTINS && defined(__SSE2__)
    sse2_optimized_loop();
#endif
    
#if USE_CPU_BUILTINS && defined(__AVX__)
    avx_optimized_loop();
#endif
    
#if USE_CPU_BUILTINS && defined(__AVX512F__)
    avx512_optimized_loop();
#endif
    
    /* Use function pointer to prevent dead code elimination */
    if (func_ptr) {
        checksum += func_ptr((int)checksum, 12345);
    }
    
    /* Final computation using all arrays */
    for (int i = 0; i < 10000; i += 16) {
        int idx = i % 1024;
        checksum += array1[idx] + array2[idx * 2] + array3[idx / 2];
    }
    
    printf("CPU Cache Test Checksum: %llu\n", (unsigned long long)checksum);
    return (int)(checksum % 256);
}
