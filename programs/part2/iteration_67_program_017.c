/* cpu_cache_coverage.c
 * 
 * This program is designed to trigger GCC's CPUID-based cache detection
 * logic during compilation, specifically targeting the switch statement
 * in driver-i386.cc (lines 127-244) that maps cache descriptor bytes
 * to cache parameters.
 *
 * Compile with various -march options to exercise different CPUID paths:
 *   gcc -O2 -march=native -mtune=native -std=gnu11 cpu_cache_coverage.c -o test_native
 *   gcc -O2 -march=nehalem -mtune=nehalem -std=gnu11 cpu_cache_coverage.c -o test_nehalem
 *   gcc -O2 -march=sandybridge -mtune=sandybridge -std=gnu11 cpu_cache_coverage.c -o test_sandybridge
 *   gcc -O2 -march=skylake -mtune=skylake -std=gnu11 cpu_cache_coverage.c -o test_skylake
 *   gcc -O2 -march=znver1 -mtune=znver1 -std=gnu11 cpu_cache_coverage.c -o test_zen
 *   gcc -O0 -march=x86-64 -std=gnu11 cpu_cache_coverage.c -o test_generic
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-aware optimizations */
#define ARRAY_SIZE (1024*1024)
static int data_array[ARRAY_SIZE];
static volatile int sink; /* Prevent dead code elimination */

/* Function pointers to prevent constant folding */
typedef void (*cache_op_func_t)(int*, size_t, int);
static cache_op_func_t current_cache_op = NULL;

/* Different cache access patterns */
void stride_1_access(int* array, size_t size, int multiplier) {
    volatile int* varray = (volatile int*)array;
    for (size_t i = 0; i < size; i += 1) {
        varray[i] = varray[i] * multiplier + i;
    }
}

void stride_2_access(int* array, size_t size, int multiplier) {
    volatile int* varray = (volatile int*)array;
    for (size_t i = 0; i < size; i += 2) {
        varray[i] = varray[i] * multiplier + i;
    }
}

void stride_4_access(int* array, size_t size, int multiplier) {
    volatile int* varray = (volatile int*)array;
    for (size_t i = 0; i < size; i += 4) {
        varray[i] = varray[i] * multiplier + i;
    }
}

void stride_8_access(int* array, size_t size, int multiplier) {
    volatile int* varray = (volatile int*)array;
    for (size_t i = 0; i < size; i += 8) {
        varray[i] = varray[i] * multiplier + i;
    }
}

void stride_16_access(int* array, size_t size, int multiplier) {
    volatile int* varray = (volatile int*)array;
    for (size_t i = 0; i < size; i += 16) {
        varray[i] = varray[i] * multiplier + i;
    }
}

/* Initialize CPU detection - this triggers the driver's CPUID logic */
__attribute__((constructor)) 
static void init_cpu_detection(void) {
    /* Force CPU initialization at program start */
    __builtin_cpu_init();
    
    /* Store results in volatile to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    sink = has_sse2 | has_sse4_2 | has_avx | has_avx2 | has_avx512f;
}

/* Conditional compilation blocks that force driver to evaluate CPU features */
#ifdef __OPTIMIZE__
/* This block will be evaluated during compilation with optimizations */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
#define USE_VECTORIZED_PATHS 1
#else
#define USE_VECTORIZED_PATHS 0
#endif
#endif

/* Main computation that uses CPU features to select cache access pattern */
static int compute_checksum(void) {
    int checksum = 0;
    
    /* Initialize array with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Select stride based on CPU features (volatile prevents constant folding) */
    volatile int stride_selector = 0;
    
    /* These builtin calls force driver to execute CPUID detection */
    if (__builtin_cpu_supports("sse2")) {
        stride_selector |= 1;
    }
    if (__builtin_cpu_supports("avx")) {
        stride_selector |= 2;
    }
    if (__builtin_cpu_supports("avx2")) {
        stride_selector |= 4;
    }
    
    /* Choose cache access pattern based on detected features */
    switch (stride_selector & 0x7) {
        case 0: current_cache_op = stride_1_access; break;
        case 1: current_cache_op = stride_2_access; break;
        case 2: 
        case 3: current_cache_op = stride_4_access; break;
        case 4:
        case 5: current_cache_op = stride_8_access; break;
        case 6:
        case 7: current_cache_op = stride_16_access; break;
    }
    
    /* Execute the selected cache access pattern */
    if (current_cache_op) {
        current_cache_op(data_array, ARRAY_SIZE, 3);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < ARRAY_SIZE; i += 64) {
        checksum ^= data_array[i];
    }
    
    return checksum;
}

/* Architecture-specific code paths to trigger different CPUID evaluations */
#if defined(__i386__) || defined(__x86_64__)
/* x86-specific optimizations that require cache size knowledge */
#ifdef __OPTIMIZE__
static void cache_aware_operation(void) {
    /* This function's optimization depends on cache parameters */
    volatile int* ptr = (volatile int*)data_array;
    size_t block_size = 1024; /* Will be optimized based on cache line size */
    
    /* Loop that compiler might optimize based on cache associativity */
    for (size_t outer = 0; outer < 8; outer++) {
        for (size_t i = 0; i < ARRAY_SIZE; i += block_size) {
            ptr[i] = ptr[i] * 2 + outer;
        }
    }
}
#endif
#endif

int main(void) {
    int result;
    
    /* Ensure CPU detection runs */
    init_cpu_detection();
    
    /* Perform cache-sensitive computation */
    result = compute_checksum();
    
#ifdef __OPTIMIZE__
    /* Execute architecture-specific path if optimizing */
    cache_aware_operation();
#endif
    
    /* Use result to prevent optimization */
    printf("CPU Cache Test - Checksum: 0x%08x\n", result);
    
    /* Additional CPU feature checks for driver evaluation */
    volatile int check_more_features = 0;
    
    /* Check various features to trigger more CPUID evaluations */
    check_more_features |= __builtin_cpu_supports("mmx") ? 1 : 0;
    check_more_features |= __builtin_cpu_supports("sse") ? 2 : 0;
    check_more_features |= __builtin_cpu_supports("sse3") ? 4 : 0;
    check_more_features |= __builtin_cpu_supports("ssse3") ? 8 : 0;
    check_more_features |= __builtin_cpu_supports("sse4.1") ? 16 : 0;
    check_more_features |= __builtin_cpu_supports("fma") ? 32 : 0;
    check_more_features |= __builtin_cpu_supports("aes") ? 64 : 0;
    
    sink = check_more_features;
    
    return 0;
}
