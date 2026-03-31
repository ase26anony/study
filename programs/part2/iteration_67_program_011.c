/*
 * CPU Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march and -mtune options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Large static array to encourage cache-sensitive optimizations */
static volatile int data_array[1024 * 1024];

/* Function pointer to prevent optimization */
typedef int (*compute_func_t)(int stride, int iterations);

/* Different computation strategies based on CPU features */
static int compute_simple(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += data_array[i & 0xFFFFF]; /* Mask to avoid overflow */
    }
    return sum;
}

static int compute_sse_optimized(int stride, int iterations) {
    int sum = 0;
    /* Simulate SSE-style processing */
    for (int i = 0; i < iterations; i += stride * 4) {
        sum += data_array[(i + 0) & 0xFFFFF];
        sum += data_array[(i + stride) & 0xFFFFF];
        sum += data_array[(i + stride * 2) & 0xFFFFF];
        sum += data_array[(i + stride * 3) & 0xFFFFF];
    }
    return sum;
}

static int compute_avx_optimized(int stride, int iterations) {
    int sum = 0;
    /* Simulate AVX-style processing (8 elements at once) */
    for (int i = 0; i < iterations; i += stride * 8) {
        for (int j = 0; j < 8; j++) {
            sum += data_array[(i + stride * j) & 0xFFFFF];
        }
    }
    return sum;
}

/* Initialize array with pseudo-random data */
static void init_array(void) {
    for (size_t i = 0; i < sizeof(data_array)/sizeof(data_array[0]); i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
}

/* 
 * Conditional compilation blocks that force driver to evaluate
 * __builtin_cpu_supports() during compilation
 */

/* Block 1: SSE2-dependent code */
#ifdef __SSE2__
static volatile int sse2_detected = 0;

static void check_sse2_features(void) {
    /* This forces driver to evaluate CPUID for SSE2 */
    if (__builtin_cpu_supports("sse2")) {
        sse2_detected = 1;
    }
    if (__builtin_cpu_supports("ssse3")) {
        sse2_detected |= 2;
    }
}
#else
static void check_sse2_features(void) {
    /* Dummy function when SSE2 not enabled */
}
#endif

/* Block 2: AVX-dependent code */
#ifdef __AVX__
static volatile int avx_detected = 0;

static void check_avx_features(void) {
    /* Force driver to evaluate CPUID for AVX features */
    if (__builtin_cpu_supports("avx")) {
        avx_detected = 1;
    }
    if (__builtin_cpu_supports("avx2")) {
        avx_detected |= 2;
    }
    if (__builtin_cpu_supports("fma")) {
        avx_detected |= 4;
    }
}
#else
static void check_avx_features(void) {
    /* Dummy function when AVX not enabled */
}
#endif

/* Block 3: AVX512-dependent code */
#ifdef __AVX512F__
static volatile int avx512_detected = 0;

static void check_avx512_features(void) {
    /* Force driver to evaluate CPUID for AVX512 */
    if (__builtin_cpu_supports("avx512f")) {
        avx512_detected = 1;
    }
    if (__builtin_cpu_supports("avx512cd")) {
        avx512_detected |= 2;
    }
    if (__builtin_cpu_supports("avx512vl")) {
        avx512_detected |= 4;
    }
}
#else
static void check_avx512_features(void) {
    /* Dummy function when AVX512 not enabled */
}
#endif

/* Main function with CPU feature detection */
int main(void) {
    int result = 0;
    compute_func_t compute_func = compute_simple;
    
    /* Initialize CPU detection - triggers CPUID in driver */
    __builtin_cpu_init();
    
    /* Check various CPU features (forces driver to evaluate CPUID) */
    check_sse2_features();
    check_avx_features();
    check_avx512_features();
    
    /* Initialize array */
    init_array();
    
    /* Choose computation strategy based on CPU features */
    volatile int use_sse = 0;
    volatile int use_avx = 0;
    
    /* These conditionals force driver to evaluate __builtin_cpu_supports */
#if defined(__OPTIMIZE__) && __OPTIMIZE__ > 0
    if (__builtin_cpu_supports("sse2")) {
        use_sse = 1;
    }
    if (__builtin_cpu_supports("avx")) {
        use_avx = 1;
    }
#endif
    
    /* Select function based on (volatile) feature detection */
    if (use_avx) {
        compute_func = compute_avx_optimized;
    } else if (use_sse) {
        compute_func = compute_sse_optimized;
    } else {
        compute_func = compute_simple;
    }
    
    /* Perform computations with different strides to test cache behavior */
    int strides[] = {1, 2, 4, 8, 16, 32, 64};
    int iterations = 1000000;
    
    for (int i = 0; i < sizeof(strides)/sizeof(strides[0]); i++) {
        result += compute_func(strides[i], iterations);
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Additional cache-sensitive pattern: blocked matrix access */
    {
        const int block_size = 64; /* Typical cache line size */
        const int n = 1024;
        int temp_sum = 0;
        
        for (int i = 0; i < n; i += block_size) {
            for (int j = 0; j < n; j += block_size) {
                for (int ii = i; ii < i + block_size && ii < n; ii++) {
                    for (int jj = j; jj < j + block_size && jj < n; jj++) {
                        temp_sum += data_array[(ii * 64 + jj) & 0xFFFFF];
                    }
                }
            }
        }
        result += temp_sum;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
