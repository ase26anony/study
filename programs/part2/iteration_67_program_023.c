/*
 * CPU Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march and -mtune options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Large static array to encourage cache-sensitive optimizations */
static volatile int data_array[1024 * 1024];

/* Volatile variables to prevent optimization of CPU feature checks */
volatile int cpu_features[16];

/* Function pointers to prevent constant folding */
typedef void (*ComputeFunc)(int*, size_t, int);
ComputeFunc func_table[8];

/* Initialize CPU features using builtins */
__attribute__((noinline))
void init_cpu_features(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check various CPU features - each requires driver to evaluate CPUID */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("fma");
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
}

/* Different computation patterns with varying strides */
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

/* Conditional compilation blocks that force driver to evaluate CPU builtins */
#ifdef __OPTIMIZE__
/* This block requires driver to evaluate __builtin_cpu_supports during compilation */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__) || \
    defined(__SSSE3__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#define USE_VECTORIZED_LOOPS 1
#else
#define USE_VECTORIZED_LOOPS 0
#endif
#endif

#ifdef __FAST_MATH__
/* Another conditional that might trigger cache detection */
#if __builtin_cpu_supports("sse2") && __builtin_cpu_supports("sse3")
#define USE_FAST_PATH 1
#else
#define USE_FAST_PATH 0
#endif
#endif

/* Main function with cache-sensitive operations */
int main(int argc, char** argv) {
    size_t array_size = sizeof(data_array) / sizeof(data_array[0]);
    int result = 0;
    
    /* Initialize function table */
    func_table[0] = compute_stride_1;
    func_table[1] = compute_stride_2;
    func_table[2] = compute_stride_4;
    func_table[3] = compute_stride_8;
    func_table[4] = compute_stride_16;
    
    /* Initialize CPU features - triggers driver's CPUID detection */
    init_cpu_features();
    
    /* Choose computation pattern based on CPU features */
    int stride_selector = 0;
    for (int i = 0; i < 12; i++) {
        stride_selector = (stride_selector << 1) | (cpu_features[i] & 1);
    }
    stride_selector = stride_selector % 5;
    
    /* Perform computation with selected stride */
    if (stride_selector >= 0 && stride_selector <= 4) {
        func_table[stride_selector]((int*)data_array, array_size, argc > 1 ? atoi(argv[1]) : 3);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < array_size; i += 64) {  /* 64-byte cache line stride */
        result ^= data_array[i];
    }
    
    /* Use result to prevent optimization */
    printf("CPU feature checksum: %d\n", result);
    
    /* Additional architecture-specific code blocks */
#ifdef __OPTIMIZE__
    #ifdef __AVX__
    /* AVX-specific path that might influence cache detection */
    if (__builtin_cpu_supports("avx")) {
        printf("Using AVX optimizations\n");
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        printf("Using AVX2 optimizations\n");
    }
    #endif
    
    #ifdef __AVX512F__
    if (__builtin_cpu_supports("avx512f")) {
        printf("Using AVX-512 optimizations\n");
    }
    #endif
#endif
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}

/* Additional functions that might trigger different optimization paths */
__attribute__((target("sse2")))
void sse2_optimized_function(int* arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = arr[i] * 2;
    }
}

__attribute__((target("avx")))
void avx_optimized_function(int* arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = arr[i] * 4;
    }
}

__attribute__((target("avx2")))
void avx2_optimized_function(int* arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = arr[i] * 8;
    }
}
