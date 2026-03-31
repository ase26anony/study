/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, architecture-specific code paths, and
 * cache-aware programming patterns.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Helper function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* arr, size_t size) {
    /* Different stride pattern */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] * 5 - 2;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Yet another pattern */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 11 + 1;
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_descriptors(uint32_t* eax, uint32_t* ebx, 
                                    uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_deterministic_cache(uint32_t* eax, uint32_t* ebx,
                                      uint32_t* ecx, uint32_t* edx,
                                      uint32_t cache_level) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(cache_level)
    );
}

int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (1024 * 1024)  /* 1M elements */
    static int __attribute__((aligned(64))) large_array1[ARRAY_SIZE];
    static int __attribute__((aligned(64))) large_array2[ARRAY_SIZE];
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        large_array1[i] = lcg_rand() % 1000;
        large_array2[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 64) {
            large_array1[i] = large_array1[i] ^ large_array2[i];
            __builtin_prefetch(&large_array1[i + 128], 0, 3);
        }
        checksum += 0x42;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 128) {
            large_array1[i] = large_array1[i] * 2 - large_array2[i];
            __builtin_prefetch(&large_array1[i + 256], 0, 2);
        }
        checksum += 0x100;
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 256) {
            large_array1[i] = large_array1[i] + large_array2[i] * 3;
            __builtin_prefetch(&large_array1[i + 512], 0, 1);
        }
        checksum += 0x200;
    }
    #endif
    
    /* Always execute SSE2 path if supported */
    if (__builtin_cpu_supports("sse2")) {
        /* Sequential access pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            large_array2[i] = large_array2[i] + i;
        }
        checksum += 0x20;
    }
    
    /* Execute architecture-specific functions */
    core2_optimized_loop(large_array1, ARRAY_SIZE);
    nehalem_optimized_loop(large_array2, ARRAY_SIZE);
    sandybridge_optimized_loop(large_array1, ARRAY_SIZE);
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 2 - Cache descriptors (may trigger multiple switch cases) */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 4 - Deterministic cache parameters for L1 */
    cpuid_deterministic_cache(&eax, &ebx, &ecx, &edx, 0);
    checksum += eax * 2;
    
    /* Leaf 4 - Deterministic cache parameters for L2 */
    cpuid_deterministic_cache(&eax, &ebx, &ecx, &edx, 1);
    checksum += ebx * 3;
    
    /* Leaf 4 - Deterministic cache parameters for L3 if present */
    cpuid_deterministic_cache(&eax, &ebx, &ecx, &edx, 2);
    checksum += ecx * 4;
    
    /* Matrix multiplication-like pattern (cache intensive) */
    #define MATRIX_SIZE 512
    static int __attribute__((aligned(64))) matrix_a[MATRIX_SIZE][MATRIX_SIZE];
    static int __attribute__((aligned(64))) matrix_b[MATRIX_SIZE][MATRIX_SIZE];
    static int __attribute__((aligned(64))) matrix_c[MATRIX_SIZE][MATRIX_SIZE];
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = lcg_rand() % 100;
            matrix_b[i][j] = lcg_rand() % 100;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication with different block sizes */
    const int block_sizes[] = {16, 32, 64, 128};
    for (int b = 0; b < 4; b++) {
        int block = block_sizes[b];
        for (int i = 0; i < MATRIX_SIZE; i += block) {
            for (int j = 0; j < MATRIX_SIZE; j += block) {
                for (int k = 0; k < MATRIX_SIZE; k += block) {
                    for (int ii = i; ii < i + block && ii < MATRIX_SIZE; ii++) {
                        for (int jj = j; jj < j + block && jj < MATRIX_SIZE; jj++) {
                            int sum = matrix_c[ii][jj];
                            for (int kk = k; kk < k + block && kk < MATRIX_SIZE; kk++) {
                                sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                            }
                            matrix_c[ii][jj] = sum;
                        }
                    }
                }
            }
        }
        checksum += matrix_c[MATRIX_SIZE-1][MATRIX_SIZE-1];
    }
    
    /* Random access pattern to stress cache */
    for (int i = 0; i < 1000000; i++) {
        uint32_t idx = lcg_rand() % ARRAY_SIZE;
        large_array1[idx] = large_array1[idx] * 13 + 17;
    }
    
    /* Final checksum computation */
    for (size_t i = 0; i < ARRAY_SIZE; i += 997) {  /* Prime stride */
        checksum += large_array1[i] + large_array2[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
