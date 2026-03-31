/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force the driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__AVX__)
#define USE_CPU_BUILTINS 1
#endif
#endif

/* Large arrays to encourage cache consideration */
static volatile int large_array[1024 * 1024];
static volatile int large_array2[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static compute_func_t func_ptr = NULL;

/* Different computation functions for different CPU features */
static int compute_sse(int a, int b) {
    return a + b;
}

static int compute_avx(int a, int b) {
    return a * b - b;
}

static int compute_basic(int a, int b) {
    return (a << 2) | (b & 0xF);
}

/* Main function with CPU feature detection */
int main(int argc, char **argv) {
    int result = 0;
    int stride = 1;
    
    /* Initialize CPU detection - this triggers driver's CPUID logic */
    __builtin_cpu_init();
    
    /* Store feature checks in volatile to prevent constant folding */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Choose stride based on CPU features (cache line consideration) */
    if (has_avx512f) {
        stride = 16;  /* AVX-512 might have different cache behavior */
    } else if (has_avx2) {
        stride = 8;
    } else if (has_sse4_2) {
        stride = 4;
    } else if (has_sse2) {
        stride = 2;
    }
    
    /* Select computation function based on features */
#ifdef USE_CPU_BUILTINS
    if (has_avx) {
        func_ptr = compute_avx;
    } else if (has_sse2) {
        func_ptr = compute_sse;
    } else {
        func_ptr = compute_basic;
    }
#else
    func_ptr = compute_basic;
#endif
    
    /* Cache-sensitive loop with variable stride */
    for (int i = 0; i < 1024 * 1024; i += stride) {
        large_array[i] = func_ptr(i, argc);
        result ^= large_array[i];
    }
    
    /* Another loop with different access pattern */
    for (int i = 0; i < 512 * 512; i += (stride * 2)) {
        large_array2[i] = func_ptr(large_array[i % (1024 * 1024)], i);
        result += large_array2[i];
    }
    
    /* Mix in feature detection results */
    result ^= (has_sse << 0);
    result ^= (has_sse2 << 1);
    result ^= (has_sse3 << 2);
    result ^= (has_ssse3 << 3);
    result ^= (has_sse4_1 << 4);
    result ^= (has_sse4_2 << 5);
    result ^= (has_avx << 6);
    result ^= (has_avx2 << 7);
    result ^= (has_avx512f << 8);
    
    printf("Result: %d (stride: %d)\n", result, stride);
    
    /* Additional conditional compilation blocks that force driver evaluation */
#if defined(__OPTIMIZE__) && (defined(__i386__) || defined(__x86_64__))
    /* This block specifically targets x86 cache detection */
    volatile int use_cache_opt = 0;
    
    /* Force evaluation of cache-related builtins if available */
#ifdef __GNUC__
    /* These conditionals make the driver examine CPU cache topology */
    if (__builtin_cpu_supports("sse") && __builtin_cpu_supports("sse2")) {
        use_cache_opt = 1;
    }
#endif
    
    if (use_cache_opt) {
        /* Perform cache-blocked matrix-style access */
        const int block_size = 64;  /* Typical cache line size */
        for (int i = 0; i < 1024; i += block_size) {
            for (int j = 0; j < 1024; j += block_size) {
                int limit_i = i + block_size < 1024 ? i + block_size : 1024;
                int limit_j = j + block_size < 1024 ? j + block_size : 1024;
                for (int ii = i; ii < limit_i; ii++) {
                    for (int jj = j; jj < limit_j; jj++) {
                        int idx = ii * 1024 + jj;
                        if (idx < 1024 * 1024) {
                            large_array[idx] = (ii * jj) & 0xFF;
                        }
                    }
                }
            }
        }
    }
#endif
    
    return result & 0xFF;
}

/* Additional functions to prevent dead code elimination */
void __attribute__((noinline)) touch_memory(volatile int *arr, int size) {
    for (int i = 0; i < size; i += 64) {  /* Cache line granularity */
        arr[i] = i;
    }
}

int __attribute__((noinline)) checksum(volatile int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 1) {
        sum ^= arr[i];
    }
    return sum;
}
