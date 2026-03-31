/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, multiple compilation targets, and cache-aware
 * programming patterns.
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
void core2_optimized_compute(int* data, size_t size) {
    /* Access pattern that might hint at L1 cache size (32KB) */
    for (size_t i = 0; i < size; i += 8) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    /* Different stride for potentially different cache behavior */
    for (size_t i = 0; i < size; i += 16) {
        data[i] = data[i] * 5 - 11;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    /* Yet another access pattern */
    for (size_t i = 0; i < size; i += 32) {
        data[i] = data[i] * 2 + data[i+1];
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
static void cpuid_deterministic_cache(uint32_t leaf, uint32_t* eax, 
                                      uint32_t* ebx, uint32_t* ecx, 
                                      uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

/* Large matrix multiplication to exercise cache */
static void matrix_multiply(int size) {
    /* Use aligned arrays to ensure cache line alignment */
    int __attribute__((aligned(64))) A[256][256];
    int __attribute__((aligned(64))) B[256][256];
    int __attribute__((aligned(64))) C[256][256];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i][j] = lcg_rand() % 100;
            B[i][j] = lcg_rand() % 100;
            C[i][j] = 0;
        }
    }
    
    /* Standard matrix multiplication with prefetching hints */
    for (int i = 0; i < size; i++) {
        for (int k = 0; k < size; k++) {
            /* Prefetch next cache line */
            if (k + 4 < size) {
                __builtin_prefetch(&A[i][k+4], 0, 3);
                __builtin_prefetch(&B[k+4][0], 0, 3);
            }
            for (int j = 0; j < size; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

/* Different access patterns to hint at cache usage */
static void varied_access_patterns(void) {
    /* Large array that exceeds L1 but fits in L2 */
    static int __attribute__((aligned(64))) large_array[512 * 1024];
    size_t size = sizeof(large_array) / sizeof(large_array[0]);
    
    /* Pattern 1: Sequential access */
    for (size_t i = 0; i < size; i++) {
        large_array[i] = i;
    }
    
    /* Pattern 2: Strided access (every 64 bytes) */
    for (size_t i = 0; i < size; i += 16) {
        large_array[i] *= 2;
    }
    
    /* Pattern 3: "Random" access using LCG */
    for (int i = 0; i < 10000; i++) {
        size_t idx = lcg_rand() % size;
        large_array[idx] += idx;
    }
}

int main(void) {
    uint64_t checksum = 0;
    
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Conditional compilation paths based on CPU features */
    #ifdef __SSE4_2__
    checksum += 0x1000;
    #endif
    
    #ifdef __AVX__
    checksum += 0x2000;
    #endif
    
    #ifdef __AVX2__
    checksum += 0x4000;
    #endif
    
    /* Runtime CPU feature checks */
    if (__builtin_cpu_supports("sse2")) {
        checksum += 1;
        /* SSE2-optimized path */
        matrix_multiply(128);
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 2;
        /* SSE4.2-optimized path */
        varied_access_patterns();
    }
    
    if (__builtin_cpu_supports("avx")) {
        checksum += 4;
        /* AVX-optimized path with larger matrices */
        matrix_multiply(256);
    }
    
    if (__builtin_cpu_supports("avx2")) {
        checksum += 8;
        /* More intensive computation */
        for (int i = 0; i < 1000; i++) {
            varied_access_patterns();
        }
    }
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Read deterministic cache parameters for L1, L2, L3 */
    for (uint32_t leaf = 0; leaf < 4; leaf++) {
        cpuid_deterministic_cache(leaf, &eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Call target-specific functions */
    {
        int __attribute__((aligned(64))) data[1024 * 1024];
        size_t data_size = sizeof(data) / sizeof(data[0]);
        
        /* Initialize data */
        for (size_t i = 0; i < data_size; i++) {
            data[i] = lcg_rand() % 1000;
        }
        
        core2_optimized_compute(data, data_size);
        nehalem_optimized_compute(data, data_size);
        sandybridge_optimized_compute(data, data_size);
        
        /* Add to checksum */
        for (size_t i = 0; i < data_size; i += 256) {
            checksum += data[i];
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
