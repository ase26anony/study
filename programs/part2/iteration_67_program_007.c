/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array_a[1024 * 1024];
static volatile int array_b[1024 * 1024];
static volatile int array_c[1024 * 1024];

/* Function pointers to prevent optimization */
typedef void (*compute_func_t)(volatile int*, volatile int*, volatile int*, int);
static compute_func_t compute_func = NULL;

/* Different computation patterns based on CPU features */
void compute_basic(volatile int* a, volatile int* b, volatile int* c, int stride) {
    for (int i = 0; i < 1024 * 1024; i += stride) {
        a[i] = b[i] + c[i] * 3;
    }
}

void compute_sse_optimized(volatile int* a, volatile int* b, volatile int* c, int stride) {
    for (int i = 0; i < 1024 * 1024; i += stride) {
        a[i] = (b[i] << 2) + c[i] / 2;
    }
}

void compute_avx_optimized(volatile int* a, volatile int* b, volatile int* c, int stride) {
    for (int i = 0; i < 1024 * 1024; i += stride) {
        a[i] = b[i] * 7 - c[i] * 5;
    }
}

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__) || \
    defined(__SSSE3__) || defined(__SSE4_1__) || defined(__SSE4_2__) || \
    defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
/* Dummy declarations that reference builtins */
static volatile int __driver_check_sse = __builtin_cpu_supports("sse") ? 1 : 0;
static volatile int __driver_check_sse2 = __builtin_cpu_supports("sse2") ? 1 : 0;
static volatile int __driver_check_sse3 = __builtin_cpu_supports("sse3") ? 1 : 0;
static volatile int __driver_check_ssse3 = __builtin_cpu_supports("ssse3") ? 1 : 0;
static volatile int __driver_check_sse4_1 = __builtin_cpu_supports("sse4.1") ? 1 : 0;
static volatile int __driver_check_sse4_2 = __builtin_cpu_supports("sse4.2") ? 1 : 0;
static volatile int __driver_check_avx = __builtin_cpu_supports("avx") ? 1 : 0;
static volatile int __driver_check_avx2 = __builtin_cpu_supports("avx2") ? 1 : 0;
static volatile int __driver_check_avx512f = __builtin_cpu_supports("avx512f") ? 1 : 0;
#endif
#endif

int main(void) {
    /* Initialize CPU detection - triggers driver-side CPUID */
    __builtin_cpu_init();
    
    /* Volatile results force actual CPUID checks */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Choose stride based on cache line detection */
    int stride = 1;
    
    /* Different stride patterns to test various cache behaviors */
    if (has_avx512f) {
        stride = 16;  /* AVX-512 might have different cache lines */
        compute_func = compute_avx_optimized;
    } else if (has_avx2) {
        stride = 8;
        compute_func = compute_avx_optimized;
    } else if (has_avx) {
        stride = 8;
        compute_func = compute_avx_optimized;
    } else if (has_sse4_2) {
        stride = 4;
        compute_func = compute_sse_optimized;
    } else if (has_sse2) {
        stride = 2;
        compute_func = compute_sse_optimized;
    } else {
        stride = 1;
        compute_func = compute_basic;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024 * 1024; i++) {
        array_b[i] = i & 0xFF;
        array_c[i] = (i >> 8) & 0xFF;
    }
    
    /* Perform computation with chosen stride */
    if (compute_func) {
        compute_func(array_a, array_b, array_c, stride);
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < 1024 * 1024; i += 1024) {
        checksum += array_a[i];
    }
    
    printf("CPU Feature checksum: %lu\n", (unsigned long)checksum);
    printf("Features: SSE=%d SSE2=%d SSE3=%d SSSE3=%d SSE4.1=%d SSE4.2=%d AVX=%d AVX2=%d AVX512F=%d\n",
           has_sse, has_sse2, has_sse3, has_ssse3, has_sse4_1, has_sse4_2, 
           has_avx, has_avx2, has_avx512f);
    printf("Stride used: %d\n", stride);
    
    return 0;
}
