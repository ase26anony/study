/* cache_detection_test.c
 * 
 * This program is designed to trigger GCC's CPUID-based cache detection
 * logic during compilation, specifically targeting the switch statement
 * in driver-i386.cc (lines 127-244) that maps cache descriptor bytes
 * to cache parameters.
 *
 * Compile with various -march options to exercise different CPUID paths:
 *   gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection_test.c -o test_native
 *   gcc -O2 -march=nehalem -mtune=nehalem -std=gnu11 cache_detection_test.c -o test_nehalem
 *   gcc -O2 -march=sandybridge -mtune=sandybridge -std=gnu11 cache_detection_test.c -o test_sandybridge
 *   gcc -O2 -march=skylake -mtune=skylake -std=gnu11 cache_detection_test.c -o test_skylake
 *   gcc -O2 -march=znver1 -mtune=znver1 -std=gnu11 cache_detection_test.c -o test_znver1
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-sensitive optimizations */
#define ARRAY_SIZE (1024 * 1024)
static int data_array[ARRAY_SIZE];
static int temp_array[ARRAY_SIZE];

/* Volatile variables to prevent optimization of CPU feature checks */
volatile int has_sse2 = 0;
volatile int has_sse3 = 0;
volatile int has_ssse3 = 0;
volatile int has_sse4_1 = 0;
volatile int has_sse4_2 = 0;
volatile int has_avx = 0;
volatile int has_avx2 = 0;
volatile int has_avx512f = 0;
volatile int has_fma = 0;
volatile int has_aes = 0;
volatile int has_pclmul = 0;

/* Function pointers to prevent constant folding */
typedef void (*ComputeFunc)(int*, int*, size_t, int);
ComputeFunc compute_func = NULL;

/* Different computation patterns that might benefit from cache-aware optimizations */
void compute_stride_1(int* src, int* dst, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 1) {
        dst[i] = src[i] * multiplier + i;
    }
}

void compute_stride_2(int* src, int* dst, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 2) {
        dst[i] = src[i] * multiplier + i;
        if (i + 1 < size) dst[i + 1] = src[i + 1] * multiplier + (i + 1);
    }
}

void compute_stride_4(int* src, int* dst, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 4) {
        for (int j = 0; j < 4 && (i + j) < size; j++) {
            dst[i + j] = src[i + j] * multiplier + (i + j);
        }
    }
}

void compute_stride_8(int* src, int* dst, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 8) {
        for (int j = 0; j < 8 && (i + j) < size; j++) {
            dst[i + j] = src[i + j] * multiplier + (i + j);
        }
    }
}

void compute_stride_16(int* src, int* dst, size_t size, int multiplier) {
    for (size_t i = 0; i < size; i += 16) {
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            dst[i + j] = src[i + j] * multiplier + (i + j);
        }
    }
}

/* Conditional compilation blocks that force driver to evaluate CPU builtins */
#ifdef __OPTIMIZE__
/* This block specifically targets the driver's cache detection */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
/* Force evaluation of CPU features during compilation */
static inline void force_cpu_detection(void) {
    /* These builtins must be evaluated by the driver */
    if (__builtin_cpu_supports("sse2")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("sse3")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("ssse3")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("sse4.1")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("sse4.2")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("avx")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("avx2")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("avx512f")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("fma")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("aes")) {
        /* Empty but forces driver to check */
    }
    if (__builtin_cpu_supports("pclmul")) {
        /* Empty but forces driver to check */
    }
}
#endif
#endif

/* Initialize data with pseudo-random values */
void init_data(void) {
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

int main(void) {
    /* Initialize CPU detection - this triggers the driver's cache detection */
    __builtin_cpu_init();
    
    /* Force driver to evaluate CPU features during compilation */
#ifdef __OPTIMIZE__
    force_cpu_detection();
#endif
    
    /* Check CPU features at runtime (prevents dead code elimination) */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
    has_fma = __builtin_cpu_supports("fma");
    has_aes = __builtin_cpu_supports("aes");
    has_pclmul = __builtin_cpu_supports("pclmul");
    
    /* Choose computation strategy based on CPU features */
    /* This creates a data-dependent control flow that can't be optimized away */
    if (has_avx512f) {
        compute_func = compute_stride_16;  /* Wider stride for AVX-512 */
    } else if (has_avx2) {
        compute_func = compute_stride_8;   /* AVX2 can handle 8-wide */
    } else if (has_avx) {
        compute_func = compute_stride_8;   /* AVX can also handle 8-wide */
    } else if (has_sse4_2) {
        compute_func = compute_stride_4;   /* SSE4.2: 4-wide */
    } else if (has_sse2) {
        compute_func = compute_stride_2;   /* SSE2: 2-wide */
    } else {
        compute_func = compute_stride_1;   /* Scalar fallback */
    }
    
    /* Initialize data */
    init_data();
    
    /* Perform computation with chosen stride */
    int multiplier = 3;
    compute_func(data_array, temp_array, ARRAY_SIZE, multiplier);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (size_t i = 0; i < ARRAY_SIZE; i += 256) {  /* Sample every 256th element */
        checksum += temp_array[i];
    }
    
    /* Print results including CPU feature info */
    printf("CPU Feature Detection Results:\n");
    printf("  SSE2:      %s\n", has_sse2 ? "YES" : "NO");
    printf("  SSE3:      %s\n", has_sse3 ? "YES" : "NO");
    printf("  SSSE3:     %s\n", has_ssse3 ? "YES" : "NO");
    printf("  SSE4.1:    %s\n", has_sse4_1 ? "YES" : "NO");
    printf("  SSE4.2:    %s\n", has_sse4_2 ? "YES" : "NO");
    printf("  AVX:       %s\n", has_avx ? "YES" : "NO");
    printf("  AVX2:      %s\n", has_avx2 ? "YES" : "NO");
    printf("  AVX512F:   %s\n", has_avx512f ? "YES" : "NO");
    printf("  FMA:       %s\n", has_fma ? "YES" : "NO");
    printf("  AES:       %s\n", has_aes ? "YES" : "NO");
    printf("  PCLMUL:    %s\n", has_pclmul ? "YES" : "NO");
    printf("\nComputation Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
