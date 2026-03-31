/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, multiple compilation targets, and cache-aware
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

/* Helper function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, size_t size) {
    /* Access pattern that may hint at L1/L2 cache usage */
    for (size_t i = 0; i < size; i += 64) {
        data[i] = data[i] * 3 + 7;
    }
    /* Small stride to potentially use different cache lines */
    for (size_t i = 1; i < size; i += 8) {
        data[i] = data[i-1] + data[i];
    }
}

/* Helper function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 32) {
        data[i] = data[i] * 5 - 2;
    }
    /* Reverse access */
    for (size_t i = size - 1; i > 0; i -= 16) {
        data[i] = data[i] + data[i-1];
    }
}

/* Helper function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, size_t size) {
    /* Matrix-like access pattern */
    for (size_t i = 0; i < size; i += 128) {
        for (size_t j = 0; j < 8; j++) {
            data[i + j] = data[i + j] * 2 + j;
        }
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_leaf2(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    uint32_t max_leaf;
    
    /* Get maximum CPUID leaf */
    asm volatile ("cpuid" 
                  : "=a"(max_leaf), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) 
                  : "a"(0));
    
    if (max_leaf >= 2) {
        /* Read cache descriptor leaf */
        asm volatile ("cpuid" 
                      : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) 
                      : "a"(2));
        return 1;
    }
    return 0;
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void read_cpuid_leaf4(uint32_t cache_level, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    uint32_t ecx_in = cache_level;
    asm volatile ("cpuid" 
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) 
                  : "a"(4), "c"(ecx_in));
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB to exceed L1/L2 */
    static int __attribute__((aligned(64))) large_array[ARRAY_SIZE];
    
    /* Fill array with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        large_array[i] = lcg_rand() % 1000;
    }
    
    /* Conditional code paths based on CPU features */
    if (__builtin_cpu_supports("avx")) {
        /* AVX path - may trigger different cache detection */
        for (size_t i = 0; i < ARRAY_SIZE; i += 256) {
            large_array[i] = large_array[i] * 11;
        }
        checksum += 1;
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 128) {
            large_array[i] = large_array[i] + 42;
        }
        checksum += 2;
        
        /* Use prefetch hints */
        for (size_t i = 0; i < ARRAY_SIZE; i += 64) {
            __builtin_prefetch(&large_array[i + 64], 0, 3);
            large_array[i] = large_array[i] * 3;
        }
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path with different stride */
        for (size_t i = 0; i < ARRAY_SIZE; i += 512) {
            large_array[i] = large_array[i] / 2;
        }
        checksum += 4;
    }
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    if (read_cpuid_leaf2(&eax, &ebx, &ecx, &edx)) {
        /* Use cache descriptor bytes in checksum */
        checksum += (eax & 0xFF) + (ebx & 0xFF) + (ecx & 0xFF) + (edx & 0xFF);
    }
    
    /* Read deterministic cache parameters for L1 and L2 */
    read_cpuid_leaf4(0, &eax, &ebx, &ecx, &edx); /* L1 */
    checksum += (eax >> 26) & 0x3F; /* Cache level */
    
    read_cpuid_leaf4(1, &eax, &ebx, &ecx, &edx); /* L2 */
    checksum += (eax >> 26) & 0x3F;
    
    /* Call architecture-specific functions */
    #ifdef __SSE4_2__
    core2_optimized_loop(large_array, ARRAY_SIZE / 4);
    checksum += large_array[0];
    #endif
    
    #ifdef __AVX__
    nehalem_optimized_loop(large_array + ARRAY_SIZE/2, ARRAY_SIZE / 8);
    checksum += large_array[ARRAY_SIZE/2];
    #endif
    
    #if defined(__SSE2__) || defined(__SSE3__)
    sandybridge_optimized_loop(large_array + ARRAY_SIZE/4, ARRAY_SIZE / 8);
    checksum += large_array[ARRAY_SIZE/4];
    #endif
    
    /* Matrix multiplication-like pattern to exercise caches */
    #define MAT_SIZE 512
    static int __attribute__((aligned(64))) mat_a[MAT_SIZE][MAT_SIZE];
    static int __attribute__((aligned(64))) mat_b[MAT_SIZE][MAT_SIZE];
    static int __attribute__((aligned(64))) mat_c[MAT_SIZE][MAT_SIZE];
    
    /* Initialize matrices */
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            mat_a[i][j] = lcg_rand() % 100;
            mat_b[i][j] = lcg_rand() % 100;
            mat_c[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication for better cache utilization */
    #define BLOCK_SIZE 64
    for (int bi = 0; bi < MAT_SIZE; bi += BLOCK_SIZE) {
        for (int bj = 0; bj < MAT_SIZE; bj += BLOCK_SIZE) {
            for (int bk = 0; bk < MAT_SIZE; bk += BLOCK_SIZE) {
                for (int i = bi; i < bi + BLOCK_SIZE && i < MAT_SIZE; i++) {
                    for (int j = bj; j < bj + BLOCK_SIZE && j < MAT_SIZE; j++) {
                        int sum = mat_c[i][j];
                        for (int k = bk; k < bk + BLOCK_SIZE && k < MAT_SIZE; k++) {
                            sum += mat_a[i][k] * mat_b[k][j];
                        }
                        mat_c[i][j] = sum;
                    }
                }
            }
        }
    }
    
    /* Add matrix result to checksum */
    for (int i = 0; i < MAT_SIZE; i += 64) {
        checksum += mat_c[i][i] & 0xFF;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum), "r"(large_array), "r"(mat_c));
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
