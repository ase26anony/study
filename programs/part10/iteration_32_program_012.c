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
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Helper function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* arr, size_t size) {
    /* Sequential access pattern */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
    
    /* Some stride-2 access */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] ^= 0xAAAAAAAA;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] * 5 - 2;
    }
    
    /* Reverse stride */
    for (size_t i = size - 1; i > 0; i -= 8) {
        arr[i] ^= 0x55555555;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Matrix-style access pattern */
    const size_t block = 64;
    for (size_t i = 0; i < size; i += block) {
        for (size_t j = 0; j < block && (i + j) < size; j++) {
            arr[i + j] = (arr[i + j] << 1) | (arr[i + j] >> 31);
        }
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_descriptors(uint32_t* eax, uint32_t* ebx, 
                                    uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_cache_parameters(uint32_t leaf, uint32_t* eax, 
                                   uint32_t* ebx, uint32_t* ecx, 
                                   uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint at cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
    int* array1 __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* array2 __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = lcg_rand();
        array2[i] = lcg_rand();
    }
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE2__
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2-optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
            array1[i] += array2[i];
            /* Prefetch hint */
            if (i + 64 < ARRAY_SIZE) {
                __builtin_prefetch(&array1[i + 64], 0, 3);
            }
        }
        checksum += 0x1;
    }
    #endif
    
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2-optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            array2[i] ^= array1[i];
            /* Different stride pattern */
            if (i % 128 == 0 && i + 128 < ARRAY_SIZE) {
                __builtin_prefetch(&array2[i + 128], 1, 2);
            }
        }
        checksum += 0x2;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            array1[i] = array1[i] * 2 - array2[i];
        }
        
        /* Matrix multiplication-like pattern */
        const size_t block = 256;
        for (size_t i = 0; i < ARRAY_SIZE; i += block) {
            size_t end = (i + block < ARRAY_SIZE) ? i + block : ARRAY_SIZE;
            for (size_t j = i; j < end; j++) {
                array2[j] = (array2[j] + array1[j]) * 3;
            }
        }
        checksum += 0x4;
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2-optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 32) {
            array1[i] = (array1[i] << 2) | (array1[i] >> 30);
        }
        
        /* Random-ish access pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 97) { /* Prime stride */
            array2[i] ^= 0xDEADBEEF;
        }
        checksum += 0x8;
    }
    #endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, ARRAY_SIZE);
    nehalem_optimized_loop(array2, ARRAY_SIZE);
    sandybridge_optimized_loop(array1, ARRAY_SIZE);
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 2 - Cache descriptors */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 4 - Deterministic cache parameters (first level) */
    cpuid_cache_parameters(0, &eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Try second cache level */
    cpuid_cache_parameters(1, &eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Compute final checksum from array data */
    for (size_t i = 0; i < ARRAY_SIZE; i += 1024) {
        checksum += array1[i] + array2[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(checksum));
    
    /* Print something to avoid complete optimization */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
