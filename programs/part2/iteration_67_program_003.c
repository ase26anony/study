/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static array to encourage cache-aware optimizations */
static volatile int data_array[1024 * 1024];

/* Function pointer to prevent optimization */
typedef void (*compute_func_t)(int*, size_t, int);
static compute_func_t func_ptr = NULL;

/* Volatile results to prevent constant folding */
static volatile int cpu_sse2 = 0;
static volatile int cpu_sse4 = 0;
static volatile int cpu_avx = 0;
static volatile int cpu_avx2 = 0;
static volatile int cpu_avx512 = 0;

/* Different computation functions for different CPU features */
void compute_basic(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        arr[i] = (arr[i] * 3 + 7) & 0xFF;
        sum += arr[i];
    }
    /* Use sum to prevent dead code elimination */
    if (sum == 0x1234) printf("Impossible\n");
}

void compute_sse2_optimized(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Pattern that might encourage SSE2 optimizations */
    for (size_t i = 0; i < size; i += stride) {
        arr[i] = (arr[i] << 2) | (arr[i] >> 6);
        sum ^= arr[i];
    }
    if (sum == 0x5678) printf("Impossible\n");
}

void compute_avx_optimized(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Different access pattern */
    for (size_t i = 0; i < size; i += (stride * 2)) {
        arr[i] = arr[i] * arr[i] + 1;
        sum += arr[i] & 1;
    }
    if (sum == 0x9ABC) printf("Impossible\n");
}

/* Main function with CPU feature detection */
int main(int argc, char** argv) {
    /* Initialize CPU detection - this triggers driver's CPUID */
    __builtin_cpu_init();
    
    /* Check CPU features - driver evaluates these during compilation */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    cpu_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Choose function based on CPU features */
    if (cpu_avx512) {
        func_ptr = compute_avx_optimized;
    } else if (cpu_avx2) {
        func_ptr = compute_avx_optimized;
    } else if (cpu_sse4) {
        func_ptr = compute_sse2_optimized;
    } else {
        func_ptr = compute_basic;
    }
    
    /* Initialize array with pseudo-random values */
    for (size_t i = 0; i < sizeof(data_array)/sizeof(data_array[0]); i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Perform computation with different strides based on CPU features */
    int stride = 1;
    
    /* Conditional compilation blocks that reference CPU builtins */
#if defined(__OPTIMIZE__) && __OPTIMIZE__ > 0
    /* These conditionals force driver to evaluate CPU features */
    if (cpu_sse2) {
        stride = 2;
    }
#ifdef __AVX__
    if (cpu_avx) {
        stride = 4;
    }
#endif
#ifdef __AVX2__
    if (cpu_avx2) {
        stride = 8;
    }
#endif
#ifdef __AVX512F__
    if (cpu_avx512) {
        stride = 16;
    }
#endif
#endif
    
    /* Execute computation */
    if (func_ptr) {
        func_ptr((int*)data_array, sizeof(data_array)/sizeof(data_array[0]), stride);
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (size_t i = 0; i < 1024; i++) {
        checksum ^= data_array[i * 64];  /* Non-contiguous access */
    }
    
    printf("CPU Features: SSE2=%d, SSE4.2=%d, AVX=%d, AVX2=%d, AVX512=%d\n",
           cpu_sse2, cpu_sse4, cpu_avx, cpu_avx2, cpu_avx512);
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Additional functions with architecture-specific optimizations */
#ifdef __x86_64__
/* Force evaluation for 64-bit x86 */
void __attribute__((used)) x86_64_helper(void) {
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    (void)has_sse3;
    (void)has_ssse3;
}
#endif

#ifdef __i386__
/* Force evaluation for 32-bit x86 */
void __attribute__((used)) i386_helper(void) {
    volatile int has_mmx = __builtin_cpu_supports("mmx");
    volatile int has_3dnow = __builtin_cpu_supports("3dnow");
    (void)has_mmx;
    (void)has_3dnow;
}
#endif
