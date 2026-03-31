/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Multiple compilation units with different -march flags
 * 3. Inline assembly with CPUID instructions
 * 4. Target attributes for different architectures
 * 5. Large array access patterns with prefetch hints
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

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, int size, int* result) {
    /* Large array access pattern that might hint at L1/L2 cache usage */
    int temp = 0;
    for (int i = 0; i < size; i += 8) { /* 8-element stride */
        temp += data[i];
        __builtin_prefetch(&data[i + 32], 0, 3); /* Medium locality hint */
    }
    *result = temp;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, int size, int* result) {
    /* Different access pattern */
    int temp = 0;
    for (int i = 0; i < size; i += 16) { /* 16-element stride */
        temp += data[i];
        __builtin_prefetch(&data[i + 64], 0, 1); /* Low locality hint */
    }
    *result = temp;
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, int size, int* result) {
    /* Matrix-style access pattern */
    int temp = 0;
    for (int i = 0; i < size; i += 32) {
        temp ^= data[i]; /* Use XOR to vary operations */
        __builtin_prefetch(&data[i + 128], 1, 3); /* Write prefetch */
    }
    *result = temp;
}

/* Inline assembly to read CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_descriptors(uint32_t* eax, uint32_t* ebx, 
                                   uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Inline assembly to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_deterministic_cache(uint32_t leaf, uint32_t* eax, 
                                     uint32_t* ebx, uint32_t* ecx, 
                                     uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB to exceed L1/L2 */
    static int __attribute__((aligned(64))) big_array[ARRAY_SIZE];
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        big_array[i] = lcg_rand() % 256;
    }
    
    /* Conditional compilation paths based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        int result = 0;
        /* Access pattern suitable for SSE4.2 */
        for (int i = 0; i < ARRAY_SIZE; i += 4) {
            result += big_array[i];
        }
        checksum += result;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        int result = 0;
        /* Larger stride for AVX */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            result += big_array[i];
        }
        checksum += result;
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        int result = 0;
        /* Even larger stride */
        for (int i = 0; i < ARRAY_SIZE; i += 16) {
            result += big_array[i];
        }
        checksum += result;
    }
    #endif
    
    /* Call architecture-specific functions */
    int core2_result = 0;
    core2_optimized_compute(big_array, ARRAY_SIZE / 4, &core2_result);
    checksum += core2_result;
    
    int nehalem_result = 0;
    nehalem_optimized_compute(big_array, ARRAY_SIZE / 4, &nehalem_result);
    checksum += nehalem_result;
    
    int sandybridge_result = 0;
    sandybridge_optimized_compute(big_array, ARRAY_SIZE / 4, &sandybridge_result);
    checksum += sandybridge_result;
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 2 - Cache descriptors (may be obsolete but still triggers logic) */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 4 - Deterministic cache parameters (for multiple cache levels) */
    for (uint32_t leaf = 0; leaf < 4; leaf++) {
        cpuid_deterministic_cache(leaf, &eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Matrix multiplication pattern to stress cache */
    #define MATRIX_SIZE 256
    static int __attribute__((aligned(64))) matrix_a[MATRIX_SIZE][MATRIX_SIZE];
    static int __attribute__((aligned(64))) matrix_b[MATRIX_SIZE][MATRIX_SIZE];
    static int __attribute__((aligned(64))) matrix_c[MATRIX_SIZE][MATRIX_SIZE];
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = lcg_rand() % 100;
            matrix_b[i][j] = lcg_rand() % 100;
        }
    }
    
    /* Perform matrix multiplication with different loop orders */
    int matrix_checksum = 0;
    
    /* ijk order - good for some cache architectures */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int sum = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
            matrix_checksum += sum;
        }
    }
    
    checksum += matrix_checksum;
    
    /* Random access pattern to stress cache associativity */
    int random_checksum = 0;
    for (int i = 0; i < 1000000; i++) {
        uint32_t idx = lcg_rand() % ARRAY_SIZE;
        random_checksum += big_array[idx];
        __builtin_prefetch(&big_array[(idx + 64) % ARRAY_SIZE], 0, 0);
    }
    checksum += random_checksum;
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
