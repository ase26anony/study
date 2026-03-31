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
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Helper function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* arr, size_t size) {
    /* Strided access pattern that may hint at L1 cache size */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Prefetch hints */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_prefetch(&arr[i + 32], 0, 3);
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] * 5 - 2;
    }
    for (size_t i = 0; i < size; i += 64) {
        __builtin_prefetch(&arr[i + 128], 1, 2);
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Matrix-style access */
    for (size_t i = 0; i < size; i += 32) {
        for (size_t j = 0; j < 32 && (i + j) < size; j++) {
            arr[i + j] = arr[i + j] + j;
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
static void cpuid_deterministic_cache(uint32_t leaf, uint32_t* eax, 
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
    
    /* Large aligned arrays to exercise cache logic */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 512];    /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = lcg_rand() % 1000;
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation based on CPU features */
#if defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__)
    /* SSE4.2 path */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Sequential access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
            array1[i] = array1[i] * 2 + 1;
            checksum += array1[i];
        }
        
        /* Read cache descriptors via CPUID */
        uint32_t eax, ebx, ecx, edx;
        cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
    }
#endif
    
    /* AVX path */
    if (__builtin_cpu_supports("avx")) {
        /* Strided access (every 16th element) */
        for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 16) {
            array2[i] = array2[i] * 3 - 5;
            checksum += array2[i];
        }
        
        /* Read deterministic cache parameters for L1 */
        uint32_t eax, ebx, ecx, edx;
        cpuid_deterministic_cache(0, &eax, &ebx, &ecx, &edx);
        checksum += eax * 3;
    }
    
    /* AVX2 path */
    if (__builtin_cpu_supports("avx2")) {
        /* More complex access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 64) {
            for (size_t j = 0; j < 8; j++) {
                size_t idx = i + j * 8;
                if (idx < sizeof(array1)/sizeof(array1[0])) {
                    array1[idx] = array1[idx] + array2[j % 512];
                    checksum += array1[idx];
                }
            }
        }
        
        /* Read deterministic cache parameters for L2 */
        uint32_t eax, ebx, ecx, edx;
        cpuid_deterministic_cache(1, &eax, &ebx, &ecx, &edx);
        checksum += ebx * 5;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    nehalem_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    sandybridge_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    
    /* Additional CPUID reads to ensure driver sees them */
    {
        uint32_t eax, ebx, ecx, edx;
        /* Leaf 1 for feature bits */
        __asm__ volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(1)
        );
        checksum += edx;
        
        /* Leaf 3 for processor serial number (if supported) */
        __asm__ volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(3)
        );
        checksum += ebx + ecx;
    }
    
    /* Final computation to prevent dead code elimination */
    for (size_t i = 0; i < 1000; i++) {
        checksum = checksum * 1664525 + 1013904223;
    }
    
    /* Use checksum in a way that can't be optimized away */
    __asm__ volatile ("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
