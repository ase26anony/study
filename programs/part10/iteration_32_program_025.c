/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID queries, architecture-specific code paths, and cache-aware
 * access patterns.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Helper function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, size_t size) {
    /* Access pattern that may hint at L1/L2 cache sizes */
    for (size_t i = 0; i < size; i += 64) { /* 64-byte stride */
        data[i] = data[i] * 3 + 1;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, size_t size) {
    /* Different stride for different cache line assumptions */
    for (size_t i = 0; i < size; i += 32) { /* 32-byte stride */
        data[i] = data[i] * 7 - 3;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, size_t size) {
    /* Yet another stride pattern */
    for (size_t i = 0; i < size; i += 128) {
        data[i] = data[i] + data[(i + 64) % size];
    }
}

/* Function that uses CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t checksum = 0;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    checksum += eax + ebx + ecx + edx;
    
    /* Also try CPUID leaf 4 (deterministic cache parameters) */
    uint32_t cache_level = 0;
    do {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(cache_level)
        );
        checksum += eax + ebx + ecx + edx;
        cache_level++;
    } while ((eax & 0x1F) != 0); /* Continue until cache type = 0 */
    
    return checksum;
}

/* Matrix multiplication to exercise cache */
static void matrix_multiply(int size) {
    /* Use VLA for stack-based large arrays */
    int A[size][size];
    int B[size][size];
    int C[size][size];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i][j] = lcg_rand() % 100;
            B[i][j] = lcg_rand() % 100;
            C[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication for cache locality */
    int block = 16; /* Typical cache block size */
    for (int i0 = 0; i0 < size; i0 += block) {
        for (int j0 = 0; j0 < size; j0 += block) {
            for (int k0 = 0; k0 < size; k0 += block) {
                for (int i = i0; i < i0 + block && i < size; i++) {
                    for (int j = j0; j < j0 + block && j < size; j++) {
                        for (int k = k0; k < k0 + block && k < size; k++) {
                            C[i][j] += A[i][k] * B[k][j];
                        }
                    }
                }
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile int sink = C[size-1][size-1];
    (void)sink;
}

int main(void) {
    uint32_t checksum = 0;
    
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays */
#define ARRAY_SIZE (1024 * 1024) /* 1M elements = 4MB */
    int* big_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    if (!big_array) return 1;
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        big_array[i] = lcg_rand() % 256;
    }
    
    /* Conditional compilation based on CPU features */
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            __builtin_prefetch(&big_array[i + 64], 0, 3);
            big_array[i] = big_array[i] * 2;
        }
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            __builtin_prefetch(&big_array[i + 128], 0, 2);
            big_array[i] = big_array[i] + 1;
        }
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 32) {
            __builtin_prefetch(&big_array[i + 256], 0, 1);
            big_array[i] = big_array[i] - 1;
        }
    }
#endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(big_array, ARRAY_SIZE);
    nehalem_optimized_loop(big_array, ARRAY_SIZE);
    sandybridge_optimized_loop(big_array, ARRAY_SIZE);
    
    /* Read CPUID cache information */
    checksum += read_cpuid_cache_descriptors();
    
    /* Perform matrix multiplication with different sizes */
    matrix_multiply(64);   /* Fits in L1/L2 */
    matrix_multiply(256);  /* Exercises L2/L3 */
    
    /* Compute final checksum */
    for (size_t i = 0; i < ARRAY_SIZE; i += 1024) {
        checksum += big_array[i];
    }
    
    /* Use checksum to prevent optimization */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %u\n", checksum);
    
    free(big_array);
    return 0;
}
