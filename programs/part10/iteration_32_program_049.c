/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Multiple compilation units with different -march flags
 * 3. Inline assembly with CPUID instruction
 * 4. Compiler pragmas and target attributes
 * 5. Large array access patterns with prefetch hints
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple linear congruential generator for pseudo-random data */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* Large array access with stride patterns */
    int temp = 0;
    for (size_t i = 0; i < size; i += 64) { /* 64-byte stride */
        temp += data[i];
    }
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(temp));
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    /* Different access pattern */
    int temp = 0;
    for (size_t i = 0; i < size; i += 32) { /* 32-byte stride */
        temp += data[i] ^ 0x55;
    }
    __asm__ volatile("" : : "r"(temp));
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    /* Matrix-style access pattern */
    int temp = 0;
    const size_t dim = 256;
    for (size_t i = 0; i < dim; ++i) {
        for (size_t j = 0; j < dim; ++j) {
            temp += data[i * dim + j];
        }
    }
    __asm__ volatile("" : : "r"(temp));
}

/* Function that uses CPUID directly via inline assembly */
static void cpuid_cache_info(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                             uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid\n\t"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Main function with conditional compilation paths */
int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (1024 * 1024) /* 1M elements */
    static int __attribute__((aligned(64))) large_array[ARRAY_SIZE];
    
    /* Fill array with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        large_array[i] = lcg_rand() & 0xFF;
    }
    
    /* Conditional paths based on CPU support */
    if (__builtin_cpu_supports("avx")) {
        /* AVX path - may trigger different cache detection */
        int temp = 0;
        for (size_t i = 0; i < ARRAY_SIZE; i += 128) {
            __builtin_prefetch(&large_array[i + 256], 0, 0);
            temp += large_array[i];
        }
        checksum += temp;
        
        /* Call target-specific function */
        sandybridge_optimized_compute(large_array, ARRAY_SIZE);
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        int temp = 0;
        /* Random-ish access pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 97) { /* Prime stride */
            temp ^= large_array[i];
        }
        checksum += temp;
        
        nehalem_optimized_compute(large_array, ARRAY_SIZE / 2);
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path - most common, will likely execute */
        int temp = 0;
        /* Sequential access */
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            temp += large_array[i];
        }
        checksum += temp;
        
        core2_optimized_compute(large_array, ARRAY_SIZE);
    }
    
    /* Execute CPUID leaf 2 (cache descriptors) */
    uint32_t eax, ebx, ecx, edx;
    cpuid_cache_info(2, &eax, &ebx, &ecx, &edx);
    
    /* Use the results to affect checksum */
    checksum += eax + ebx + ecx + edx;
    
    /* Execute CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; ++i) {
        eax = 4;
        ecx = i;
        __asm__ volatile (
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Additional architecture-specific code using macros */
    #ifdef __SSE4_2__
    {
        /* SSE4.2 specific operations */
        int temp = 0;
        for (size_t i = 0; i < ARRAY_SIZE; i += 64) {
            temp |= large_array[i];
        }
        checksum += temp;
    }
    #endif
    
    #ifdef __AVX__
    {
        /* AVX specific operations */
        int temp = 0;
        /* Reverse access pattern */
        for (size_t i = ARRAY_SIZE - 1; i > 0; i -= 128) {
            temp &= large_array[i];
        }
        checksum += temp;
    }
    #endif
    
    /* Final output to prevent complete optimization */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
