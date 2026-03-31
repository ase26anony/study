/* cpu_cache_coverage.c - Trigger GCC driver CPUID cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static compute_func_t func_ptr = NULL;

/* Force driver to evaluate CPU features during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#else
#define USE_SSE_FEATURES 0
#endif

#if defined(__AVX__) || defined(__AVX2__)
#define USE_AVX_FEATURES 1
#else
#define USE_AVX_FEATURES 0
#endif

#if defined(__AVX512F__) || defined(__AVX512BW__)
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif
#endif /* __OPTIMIZE__ */

/* Different computation functions for different CPU features */
int compute_sse(int iterations, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        array1[i % (1024*1024)] = i * 3;
        sum += array1[(i * 7) % (1024*1024)];
    }
    return sum;
}

int compute_avx(int iterations, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        array2[i % (1024*1024)] = i * 5;
        sum += array2[(i * 11) % (1024*1024)];
        array3[(i / 2) % (512*512)] = sum;
    }
    return sum;
}

int compute_avx512(int iterations, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        array1[i % (1024*1024)] = i * 7;
        array2[(i * 13) % (1024*1024)] = sum;
        sum += array1[(i * 17) % (1024*1024)];
        array3[(i / 4) % (512*512)] = sum * 3;
    }
    return sum;
}

int compute_generic(int iterations, int stride) {
    volatile int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += i * 19;
        array1[i % 1024] = sum;
    }
    return sum;
}

/* Main function with CPU feature detection */
int main(void) {
    /* Initialize CPU detection - forces driver to run CPUID */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent constant folding */
    volatile int has_sse = 0;
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* Check multiple CPU features to ensure thorough CPUID execution */
    has_sse = __builtin_cpu_supports("sse");
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Choose computation based on CPU features */
    int iterations = 1000000;
    int stride = 1;
    
    /* Vary stride based on detected features to test different cache access patterns */
    if (has_avx512f) {
        func_ptr = compute_avx512;
        stride = 16;  /* Large stride for AVX512 */
    } else if (has_avx2) {
        func_ptr = compute_avx;
        stride = 8;   /* Medium stride for AVX2 */
    } else if (has_sse4_2) {
        func_ptr = compute_sse;
        stride = 4;   /* Smaller stride for SSE4.2 */
    } else {
        func_ptr = compute_generic;
        stride = 2;   /* Basic stride */
    }
    
    /* Additional feature-based stride adjustments */
    if (has_sse3) stride |= 0x1;
    if (has_ssse3) stride |= 0x2;
    if (has_sse4_1) stride |= 0x4;
    
    /* Perform computation with cache-sensitive access pattern */
    volatile int result = func_ptr(iterations, stride);
    
    /* Use result to prevent dead code elimination */
    printf("CPU Feature Check Results:\n");
    printf("  SSE: %d, SSE2: %d, SSE3: %d\n", has_sse, has_sse2, has_sse3);
    printf("  SSSE3: %d, SSE4.1: %d, SSE4.2: %d\n", has_ssse3, has_sse4_1, has_sse4_2);
    printf("  AVX: %d, AVX2: %d, AVX512F: %d\n", has_avx, has_avx2, has_avx512f);
    printf("Computation result: %d (stride: %d)\n", result, stride);
    
    /* Additional cache-thrashing loop for good measure */
    volatile int checksum = 0;
    for (int i = 0; i < 10000; i++) {
        /* Access arrays with different strides to exercise cache logic */
        array1[(i * 31) & 0xFFFFF] = i;
        array2[(i * 47) & 0xFFFFF] = array1[(i * 31) & 0xFFFFF] * 2;
        array3[(i * 73) & 0x3FFFF] = array2[(i * 47) & 0xFFFFF] + i;
        checksum += array3[(i * 73) & 0x3FFFF];
    }
    
    printf("Cache checksum: %d\n", checksum);
    
    return 0;
}

/* Force inclusion of CPU-specific code paths through conditionals */
#ifdef __SSE__
__attribute__((used)) 
static void force_sse_path(void) {
    volatile int x = __builtin_cpu_supports("sse") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __SSE2__
__attribute__((used)) 
static void force_sse2_path(void) {
    volatile int x = __builtin_cpu_supports("sse2") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __SSE3__
__attribute__((used)) 
static void force_sse3_path(void) {
    volatile int x = __builtin_cpu_supports("sse3") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __SSSE3__
__attribute__((used)) 
static void force_ssse3_path(void) {
    volatile int x = __builtin_cpu_supports("ssse3") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __SSE4_1__
__attribute__((used)) 
static void force_sse41_path(void) {
    volatile int x = __builtin_cpu_supports("sse4.1") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __SSE4_2__
__attribute__((used)) 
static void force_sse42_path(void) {
    volatile int x = __builtin_cpu_supports("sse4.2") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __AVX__
__attribute__((used)) 
static void force_avx_path(void) {
    volatile int x = __builtin_cpu_supports("avx") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __AVX2__
__attribute__((used)) 
static void force_avx2_path(void) {
    volatile int x = __builtin_cpu_supports("avx2") ? 1 : 0;
    (void)x;
}
#endif

#ifdef __AVX512F__
__attribute__((used)) 
static void force_avx512_path(void) {
    volatile int x = __builtin_cpu_supports("avx512f") ? 1 : 0;
    (void)x;
}
#endif
