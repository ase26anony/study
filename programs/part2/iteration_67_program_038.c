/*
 * CPUID Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march and -mtune options.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-sensitive optimizations */
static volatile int array1[1024 * 1024];  /* 4MB */
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Volatile variables to prevent optimization */
volatile int cpu_feature_sse2 = 0;
volatile int cpu_feature_sse4 = 0;
volatile int cpu_feature_avx = 0;
volatile int cpu_feature_avx2 = 0;
volatile int cpu_feature_avx512 = 0;
volatile int cpu_feature_fma = 0;
volatile int cpu_feature_aes = 0;
volatile int cpu_feature_pclmul = 0;

/* Function pointers to prevent constant folding */
typedef int (*ComputeFunc)(int, int);
volatile ComputeFunc func_ptr = NULL;

/* Initialize CPU detection - forces driver to execute CPUID */
__attribute__((constructor)) 
static void init_cpu_features(void) {
    __builtin_cpu_init();
    
    /* These calls force the driver to evaluate CPUID */
    cpu_feature_sse2 = __builtin_cpu_supports("sse2");
    cpu_feature_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_feature_avx = __builtin_cpu_supports("avx");
    cpu_feature_avx2 = __builtin_cpu_supports("avx2");
    cpu_feature_avx512 = __builtin_cpu_supports("avx512f");
    cpu_feature_fma = __builtin_cpu_supports("fma");
    cpu_feature_aes = __builtin_cpu_supports("aes");
    cpu_feature_pclmul = __builtin_cpu_supports("pclmul");
}

/* Different computation functions for different CPU features */
static int compute_sse2(int a, int b) { return (a + b) * (a - b); }
static int compute_sse4(int a, int b) { return (a * b) ^ (a + b); }
static int compute_avx(int a, int b) { return (a << 2) | (b >> 2); }
static int compute_avx2(int a, int b) { return (a * 3) + (b * 7); }
static int compute_generic(int a, int b) { return a + b * 2; }

/* Cache-sensitive traversal with different strides */
static uint64_t traverse_array(volatile int* arr, size_t size, int stride) {
    uint64_t sum = 0;
    
    /* Non-constant stride prevents optimization */
    for (size_t i = 0; i < size; i += stride) {
        arr[i] = i % 256;
        sum += arr[i];
        
        /* Access with different patterns */
        if (i > 0 && (i % 8192) == 0) {
            sum += arr[i - 1];
        }
    }
    
    return sum;
}

/* Main function with conditional compilation based on CPU features */
int main(void) {
    uint64_t checksum = 0;
    
    /* Force CPU initialization at runtime too */
    __builtin_cpu_init();
    
    /* 
     * Conditional compilation blocks that reference CPU builtins
     * These force the driver to evaluate CPUID during compilation
     */
    
#if defined(__OPTIMIZE__) && defined(__SSE2__)
    if (__builtin_cpu_supports("sse2")) {
        func_ptr = compute_sse2;
    }
#endif
    
#if defined(__OPTIMIZE__) && defined(__SSE4_2__)
    if (__builtin_cpu_supports("sse4.2")) {
        func_ptr = compute_sse4;
    }
#endif
    
#if defined(__OPTIMIZE__) && defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        func_ptr = compute_avx;
    }
#endif
    
#if defined(__OPTIMIZE__) && defined(__AVX2__)
    if (__builtin_cpu_supports("avx2")) {
        func_ptr = compute_avx2;
    }
#endif
    
    /* Default if no special features */
    if (!func_ptr) {
        func_ptr = compute_generic;
    }
    
    /* Perform cache-sensitive operations with different strides */
    int stride = 1;
    
    /* Choose stride based on detected features */
    if (cpu_feature_avx512) {
        stride = 16;  /* Larger stride for wide vectors */
    } else if (cpu_feature_avx2) {
        stride = 8;
    } else if (cpu_feature_avx) {
        stride = 4;
    } else if (cpu_feature_sse4) {
        stride = 2;
    }
    
    /* Traverse arrays with cache-sensitive patterns */
    checksum += traverse_array(array1, sizeof(array1)/sizeof(array1[0]), stride);
    checksum += traverse_array(array2, sizeof(array2)/sizeof(array2[0]), stride * 2);
    checksum += traverse_array(array3, sizeof(array3)/sizeof(array3[0]), stride);
    
    /* Use function pointer to prevent dead code elimination */
    int result = func_ptr(checksum & 0xFFFF, (checksum >> 16) & 0xFFFF);
    
    /* Mix in CPU feature flags */
    result ^= cpu_feature_sse2;
    result ^= cpu_feature_sse4 << 1;
    result ^= cpu_feature_avx << 2;
    result ^= cpu_feature_avx2 << 3;
    result ^= cpu_feature_avx512 << 4;
    
    printf("CPU Cache Test Result: %d (Checksum: %llu)\n", 
           result, (unsigned long long)checksum);
    
    return 0;
}

/* Additional functions to ensure various code paths exist */
#ifdef __x86_64__
__attribute__((target("sse2")))
static void sse2_path(void) {
    volatile double x = 1.0, y = 2.0;
    asm volatile("" : "+x"(x), "+x"(y));
}

__attribute__((target("avx")))
static void avx_path(void) {
    volatile double x = 1.0, y = 2.0;
    asm volatile("" : "+x"(x), "+x"(y));
}

__attribute__((target("avx2")))
static void avx2_path(void) {
    volatile double x = 1.0, y = 2.0;
    asm volatile("" : "+x"(x), "+x"(y));
}
#endif
