/* cache_detection_test.c
 * 
 * This program is designed to trigger GCC's CPUID-based cache detection
 * logic during compilation, specifically targeting the switch statement
 * in driver-i386.cc (lines 127-244) that maps cache descriptor bytes
 * to cache parameters.
 *
 * Compile with various -march= options to hit different switch cases:
 *   gcc -O2 -march=native -mtune=native cache_detection_test.c -o test_native
 *   gcc -O2 -march=nehalem cache_detection_test.c -o test_nehalem
 *   gcc -O2 -march=sandybridge cache_detection_test.c -o test_sandybridge
 *   gcc -O2 -march=skylake cache_detection_test.c -o test_skylake
 *   gcc -O2 -march=znver1 cache_detection_test.c -o test_znver1 (AMD)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Large static arrays to encourage cache-aware optimizations */
#define ARRAY_SIZE (1024 * 1024)
static int data_array[ARRAY_SIZE];
static volatile int sink = 0; /* Prevent dead code elimination */

/* Function pointer type to prevent optimization */
typedef void (*compute_func_t)(int*, size_t, int);

/* Different computation patterns that might benefit from cache-aware optimizations */
void compute_stride_1(int* arr, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 1) {
        arr[i] = arr[i] * multiplier + i;
    }
}

void compute_stride_2(int* arr, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 2) {
        arr[i] = arr[i] * multiplier + i;
    }
}

void compute_stride_4(int* arr, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] * multiplier + i;
    }
}

void compute_stride_8(int* arr, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * multiplier + i;
    }
}

void compute_stride_16(int* arr, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * multiplier + i;
    }
}

/* Initialize array with pseudo-random data */
void init_array(int* arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

/* 
 * Conditional compilation blocks that force the driver to evaluate
 * __builtin_cpu_supports() during compilation
 */

/* Block 1: SSE/SSE2 features - common on most x86-64 CPUs */
#if defined(__SSE__) || defined(__SSE2__)
#define USE_SSE_FEATURES 1
#else
#define USE_SSE_FEATURES 0
#endif

/* Block 2: AVX features - triggers different CPUID paths */
#ifdef __AVX__
#define USE_AVX_FEATURES 1
#else
#define USE_AVX_FEATURES 0
#endif

/* Block 3: AVX512 features - modern Intel CPUs */
#ifdef __AVX512F__
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif

/* Block 4: BMI features - different CPU generations */
#ifdef __BMI__
#define USE_BMI_FEATURES 1
#else
#define USE_BMI_FEATURES 0
#endif

int main(void) {
    /* Force CPU initialization - this triggers CPUID in the driver */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    volatile int has_bmi1 = 0;
    volatile int has_bmi2 = 0;
    
    /* Check CPU features - each call may trigger cache detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
    has_bmi1 = __builtin_cpu_supports("bmi1");
    has_bmi2 = __builtin_cpu_supports("bmi2");
    
    /* Initialize array */
    init_array(data_array, ARRAY_SIZE);
    
    /* Select computation function based on CPU features */
    compute_func_t compute_func = NULL;
    int stride_multiplier = 1;
    
    /* 
     * Complex decision logic that the compiler can't optimize away
     * Each path uses different builtin checks
     */
    if (has_avx512f && __builtin_cpu_supports("avx512cd")) {
        compute_func = compute_stride_16;  /* Large stride for AVX512 */
        stride_multiplier = 16;
    } else if (has_avx2 && __builtin_cpu_supports("fma")) {
        compute_func = compute_stride_8;   /* Medium stride for AVX2 */
        stride_multiplier = 8;
    } else if (has_avx) {
        compute_func = compute_stride_4;   /* Smaller stride for AVX */
        stride_multiplier = 4;
    } else if (has_sse4_1 && __builtin_cpu_supports("popcnt")) {
        compute_func = compute_stride_2;   /* Small stride for SSE4.1 */
        stride_multiplier = 2;
    } else if (has_sse2) {
        compute_func = compute_stride_1;   /* Unit stride for SSE2 */
        stride_multiplier = 1;
    } else {
        /* Fallback - should still trigger basic CPUID */
        compute_func = compute_stride_1;
        stride_multiplier = 1;
    }
    
    /* Additional CPU feature checks in the execution path */
    volatile int use_prefetch = 0;
    if (__builtin_cpu_supports("sse") && __builtin_cpu_supports("mmx")) {
        use_prefetch = 1;
    }
    
    /* Perform computation */
    if (compute_func) {
        compute_func(data_array, ARRAY_SIZE, stride_multiplier);
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)data_array[i];
        /* Insert pseudo-prefetch based on CPU features */
        if (use_prefetch && (i % 64 == 0)) {
            __builtin_prefetch(&data_array[i + 64], 0, 3);
        }
    }
    
    /* Use checksum to affect control flow */
    sink = (checksum & 1);
    
    /* Print minimal output */
    printf("CPU Features detected: SSE2=%d, AVX=%d, AVX2=%d, AVX512F=%d\n",
           has_sse2, has_avx, has_avx2, has_avx512f);
    printf("Checksum LSB: %d\n", sink);
    
    return sink;
}

/* 
 * Additional conditional compilation that references CPU builtins
 * These force the driver to evaluate them during preprocessing
 */
#if defined(__OPTIMIZE__) && (__builtin_cpu_supports("sse2") || 0)
/* This code block's inclusion depends on SSE2 support */
#define OPTIMIZED_FOR_SSE2 1
#else
#define OPTIMIZED_FOR_SSE2 0
#endif

#if defined(__FAST_MATH__) && (__builtin_cpu_supports("avx") || 0)
/* AVX-optimized math path */
#define USE_FAST_AVX_MATH 1
#else
#define USE_FAST_AVX_MATH 0
#endif
