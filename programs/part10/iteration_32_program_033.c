/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Creating multiple compilation units with different -march flags
 * 3. Embedding CPUID inline assembly
 * 4. Using target attributes for different architectures
 * 5. Using large array access patterns with prefetch hints
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
    /* Large array access with stride patterns */
    int temp = 0;
    for (int i = 0; i < size; i += 8) {
        temp += data[i];
        __builtin_prefetch(&data[i + 64], 0, 3); /* Medium locality hint */
    }
    *result = temp;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, int size, int* result) {
    /* Different access pattern */
    int temp = 0;
    for (int i = 0; i < size; i += 16) {
        temp += data[i] * 2;
        __builtin_prefetch(&data[i + 128], 0, 1); /* Low locality hint */
    }
    *result = temp;
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, int size, int* result) {
    /* Matrix-style access pattern */
    int temp = 0;
    const int stride = 32;
    for (int i = 0; i < size; i += stride) {
        for (int j = 0; j < 8; j++) {
            temp += data[i + j];
        }
        __builtin_prefetch(&data[i + 256], 0, 2);
    }
    *result = temp;
}

/* Function that uses CPUID directly to read cache descriptors */
static void cpuid_cache_info(uint32_t leaf, uint32_t* eax, uint32_t* ebx, 
                             uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}

/* Main computation function with conditional paths based on CPU features */
static int compute_with_cpu_features(void) {
    int checksum = 0;
    
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (1024 * 1024) /* 1M elements */
    static int __attribute__((aligned(64))) large_array[ARRAY_SIZE];
    
    /* Fill array with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        large_array[i] = lcg_rand() % 100;
    }
    
    /* Different code paths based on CPU features */
    if (__builtin_cpu_supports("avx")) {
        /* AVX path - may trigger different cache detection */
        int result = 0;
        sandybridge_optimized_compute(large_array, ARRAY_SIZE, &result);
        checksum += result;
        
        /* Read CPUID leaf 2 (cache descriptors) */
        uint32_t eax, ebx, ecx, edx;
        cpuid_cache_info(2, &eax, &ebx, &ecx, &edx);
        checksum += (eax & 0xFF) + (ebx & 0xFF) + (ecx & 0xFF) + (edx & 0xFF);
        
        #ifdef __AVX__
        /* Additional AVX-specific computation */
        for (int i = 0; i < ARRAY_SIZE; i += 128) {
            checksum += large_array[i] * 3;
        }
        #endif
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        int result = 0;
        nehalem_optimized_compute(large_array, ARRAY_SIZE, &result);
        checksum += result;
        
        /* Read CPUID leaf 4 (deterministic cache parameters) */
        uint32_t eax, ebx, ecx, edx;
        cpuid_cache_info(4, &eax, &ebx, &ecx, &edx);
        checksum += (eax & 0x1F); /* Cache type field */
        
        #ifdef __SSE4_2__
        /* SSE4.2 specific pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            checksum += large_array[i] ^ 0x55;
        }
        #endif
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path - most common, always execute */
        int result = 0;
        core2_optimized_compute(large_array, ARRAY_SIZE, &result);
        checksum += result;
        
        /* Additional CPUID read */
        uint32_t eax, ebx, ecx, edx;
        cpuid_cache_info(1, &eax, &ebx, &ecx, &edx); /* Basic CPUID */
        checksum += (edx >> 8) & 0xFF; /* APIC ID */
        
        #ifdef __SSE2__
        /* SSE2 specific access pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 32) {
            checksum += large_array[i] | 0xAA;
        }
        #endif
    }
    
    /* Random-ish access pattern to stress cache detection */
    for (int i = 0; i < 10000; i++) {
        int idx = lcg_rand() % ARRAY_SIZE;
        checksum += large_array[idx];
        if (i % 8 == 0) {
            __builtin_prefetch(&large_array[(idx + 512) % ARRAY_SIZE], 0, 0);
        }
    }
    
    return checksum;
}

int main(void) {
    int final_result = 0;
    
    /* Call the main computation multiple times with different conditions */
    for (int iteration = 0; iteration < 3; iteration++) {
        final_result += compute_with_cpu_features();
        
        /* Force different optimization paths based on iteration */
        #ifdef __OPTIMIZE__
        if (iteration == 1) {
            /* Create another large array on stack */
            int stack_array[8192] __attribute__((aligned(32)));
            for (int i = 0; i < 8192; i++) {
                stack_array[i] = i * iteration;
                final_result += stack_array[i];
            }
        }
        #endif
    }
    
    /* Use the result to prevent dead code elimination */
    __asm__ volatile("" : : "r"(final_result));
    
    printf("Computation result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
