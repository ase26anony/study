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

/* Simple linear congruential generator for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
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
        arr[i] = arr[i] * 5 + 11;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Yet another pattern */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 7 + 13;
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static void cpuid_leaf2(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_leaf4(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
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
    if (__builtin_cpu_supports("sse2")) {
        /* Sequential access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
            array1[i] = array1[i] * 2 + 1;
        }
        checksum += array1[0];
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* Strided access pattern (every 4th element) */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 4) {
            __builtin_prefetch(&array1[i + 32], 0, 3);
            array1[i] = array1[i] * 3 + 2;
        }
        checksum += array1[4];
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* Larger stride pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 16) {
            __builtin_prefetch(&array1[i + 64], 0, 2);
            array1[i] = array1[i] * 5 + 3;
        }
        checksum += array1[16];
        
        /* Call AVX-optimized function */
        #ifdef __AVX__
        nehalem_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
        #endif
    }
    
    if (__builtin_cpu_supports("avx2")) {
        /* Random-ish access pattern within cache lines */
        for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 64) {
            array2[i] = array2[i] * 7 + 5;
            array2[i + 3] = array2[i + 3] * 11 + 7;
            array2[i + 7] = array2[i + 7] * 13 + 11;
        }
        checksum += array2[64];
        
        /* Call Sandybridge-optimized function */
        #ifdef __AVX2__
        sandybridge_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
        #endif
    }
    
    /* Always call Core2 optimized function */
    core2_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    
    /* Matrix multiplication-like pattern (cache intensive) */
    for (size_t i = 0; i < 256; i++) {
        for (size_t j = 0; j < 256; j++) {
            int sum = 0;
            for (size_t k = 0; k < 256; k++) {
                sum += array1[i * 256 + k] * array2[k * 256 + j];
            }
            checksum += sum;
        }
    }
    
    /* Execute CPUID leaf 2 (cache descriptors) */
    {
        uint32_t eax, ebx, ecx, edx;
        cpuid_leaf2(&eax, &ebx, &ecx, &edx);
        
        /* Use the results to affect checksum */
        checksum += eax + ebx + ecx + edx;
        
        /* The driver will see this cpuid during compilation with -fverbose-asm */
        __asm__ volatile (
            "# Cache descriptor bytes: %0 %1 %2 %3"
            : 
            : "r"(eax), "r"(ebx), "r"(ecx), "r"(edx)
        );
    }
    
    /* Execute CPUID leaf 4 for cache parameters */
    for (uint32_t leaf = 0; leaf < 4; leaf++) {
        uint32_t eax, ebx, ecx, edx;
        cpuid_leaf4(leaf, &eax, &ebx, &ecx, &edx);
        
        checksum += eax + ebx + ecx + edx;
        
        /* Force the compiler to consider cache hierarchy */
        if ((eax & 0x1F) != 0) {  /* Cache type field */
            __builtin_prefetch(&array1[0], 0, 0);
        }
    }
    
    /* Histogram calculation (cache sensitive) */
    int histogram[256] = {0};
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        histogram[array1[i] % 256]++;
    }
    for (int i = 0; i < 256; i++) {
        checksum += histogram[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
