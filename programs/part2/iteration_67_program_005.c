/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define USE_CPU_FEATURES 1
#else
#define USE_CPU_FEATURES 0
#endif

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef void (*loop_func_t)(volatile int*, volatile int*, int, int);
loop_func_t func_ptr = NULL;

/* Different loop patterns that might benefit from cache-aware optimization */
void loop_stride_1(volatile int* dst, volatile int* src, int size, int iterations) {
    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < size; i++) {
            dst[i] = src[i] + it;
        }
    }
}

void loop_stride_2(volatile int* dst, volatile int* src, int size, int iterations) {
    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < size; i += 2) {
            dst[i] = src[i] * it;
        }
    }
}

void loop_stride_4(volatile int* dst, volatile int* src, int size, int iterations) {
    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < size; i += 4) {
            dst[i] = src[i] - it;
        }
    }
}

void loop_stride_8(volatile int* dst, volatile int* src, int size, int iterations) {
    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < size; i += 8) {
            dst[i] = src[i] ^ it;
        }
    }
}

void loop_stride_16(volatile int* dst, volatile int* src, int size, int iterations) {
    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < size; i += 16) {
            dst[i] = src[i] | it;
        }
    }
}

/* Conditional compilation based on CPU features */
#if USE_CPU_FEATURES
/* These macros force driver to evaluate CPUID */
#define CHECK_FEATURE(feat) (__builtin_cpu_supports(feat) ? 1 : 0)
#define INIT_CPU() __builtin_cpu_init()
#else
#define CHECK_FEATURE(feat) 0
#define INIT_CPU() do {} while(0)
#endif

int main(void) {
    int checksum = 0;
    int stride = 1;
    
    /* Initialize CPU detection - triggers driver to run CPUID */
    INIT_CPU();
    
    /* Check various CPU features - each may require different cache considerations */
    volatile int has_sse2 = CHECK_FEATURE("sse2");
    volatile int has_sse3 = CHECK_FEATURE("sse3");
    volatile int has_ssse3 = CHECK_FEATURE("ssse3");
    volatile int has_sse4_1 = CHECK_FEATURE("sse4.1");
    volatile int has_sse4_2 = CHECK_FEATURE("sse4.2");
    volatile int has_avx = CHECK_FEATURE("avx");
    volatile int has_avx2 = CHECK_FEATURE("avx2");
    volatile int has_avx512f = CHECK_FEATURE("avx512f");
    
    /* Choose loop pattern based on detected features */
    if (has_avx512f) {
        func_ptr = loop_stride_16;  /* AVX-512 benefits from larger strides */
        stride = 16;
    } else if (has_avx2) {
        func_ptr = loop_stride_8;   /* AVX2 benefits from 256-bit accesses */
        stride = 8;
    } else if (has_avx) {
        func_ptr = loop_stride_4;   /* AVX benefits from 128-bit accesses */
        stride = 4;
    } else if (has_sse4_2) {
        func_ptr = loop_stride_2;   /* SSE4.2 */
        stride = 2;
    } else {
        func_ptr = loop_stride_1;   /* Baseline */
        stride = 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024 * 1024; i++) {
        array1[i] = i & 0xFF;
        if (i < 512 * 512) {
            array2[i] = (i * 3) & 0xFF;
        }
    }
    
    /* Execute cache-sensitive loop */
    if (func_ptr) {
        func_ptr(array3, array1, 512 * 512, 10);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 1024 * 1024; i += stride) {
        checksum += array1[i];
        checksum ^= array2[i % (512 * 512)];
    }
    
    /* Use checksum to prevent optimization */
    if (checksum == 0) {
        printf("Zero checksum (unlikely)\n");
    }
    
    printf("Final checksum: %d (stride: %d)\n", checksum, stride);
    printf("CPU Features: SSE2=%d, SSE3=%d, SSSE3=%d, SSE4.1=%d, SSE4.2=%d, AVX=%d, AVX2=%d, AVX512F=%d\n",
           has_sse2, has_sse3, has_ssse3, has_sse4_1, has_sse4_2, has_avx, has_avx2, has_avx512f);
    
    return 0;
}

/* Additional conditional compilation blocks to force driver evaluation */
#if defined(__SSE2__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) sse2_detected = CHECK_FEATURE("sse2");
#endif

#if defined(__SSE3__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) sse3_detected = CHECK_FEATURE("sse3");
#endif

#if defined(__SSSE3__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) ssse3_detected = CHECK_FEATURE("ssse3");
#endif

#if defined(__SSE4_1__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) sse4_1_detected = CHECK_FEATURE("sse4.1");
#endif

#if defined(__SSE4_2__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) sse4_2_detected = CHECK_FEATURE("sse4.2");
#endif

#if defined(__AVX__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) avx_detected = CHECK_FEATURE("avx");
#endif

#if defined(__AVX2__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) avx2_detected = CHECK_FEATURE("avx2");
#endif

#if defined(__AVX512F__) && USE_CPU_FEATURES
static volatile int __attribute__((used)) avx512f_detected = CHECK_FEATURE("avx512f");
#endif
