/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID techniques, architecture-specific code paths, and cache-aware
 * programming patterns.
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
    /* Access pattern that might hint at L1 cache size */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different stride for potentially different cache behavior */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 5 + 11;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Yet another access pattern */
    for (size_t i = 0; i < size; i += 32) {
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

int main() {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to exercise cache logic */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 1024];   /* 2MB */
    __attribute__((aligned(64))) static int array3[256 * 1024];   /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = lcg_rand() % 1000;
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = lcg_rand() % 1000;
    }
    for (size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i++) {
        array3[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation based on CPU features */
#if defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__)
    /* This block will only be compiled if SSE4.2/AVX is available */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Matrix-style access pattern */
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 1024; j++) {
                array1[i * 1024 + j] += array2[j * 512 + (i % 512)];
            }
        }
        checksum += 0xSSE42;
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* Larger stride access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 64) {
            __builtin_prefetch(&array1[i + 128], 0, 3);
            array1[i] = array1[i] * 2 - array1[i + 32];
        }
        checksum += 0xAVX;
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* Another pattern with prefetching */
        for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 128) {
            __builtin_prefetch(&array2[i + 256], 1, 2);
            array2[i] = array2[i] * 3 + array2[i + 64];
        }
        checksum += 0xAVX2;
    }
#endif
    
    /* Execute CPUID leaf 2 to get cache descriptors */
    uint32_t eax2, ebx2, ecx2, edx2;
    cpuid_leaf2(&eax2, &ebx2, &ecx2, &edx2);
    
    /* Use the results to influence checksum (prevent dead code elimination) */
    checksum += eax2 + ebx2 + ecx2 + edx2;
    
    /* Execute CPUID leaf 4 for deterministic cache parameters */
    for (uint32_t leaf = 0; leaf < 4; leaf++) {
        uint32_t eax4, ebx4, ecx4, edx4;
        cpuid_leaf4(leaf, &eax4, &ebx4, &ecx4, &edx4);
        checksum += eax4 + ebx4 + ecx4 + edx4;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array3, sizeof(array3)/sizeof(array3[0]));
    nehalem_optimized_loop(array3, sizeof(array3)/sizeof(array3[0]));
    sandybridge_optimized_loop(array3, sizeof(array3)/sizeof(array3[0]));
    
    /* Additional cache-intensive computation */
    /* Histogram calculation with varying strides */
    int histogram[256] = {0};
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        histogram[array1[i] % 256]++;
    }
    
    /* Transpose-like operation on array2 */
    for (size_t i = 0; i < 512; i++) {
        for (size_t j = i + 1; j < 512; j++) {
            int temp = array2[i * 512 + j];
            array2[i * 512 + j] = array2[j * 512 + i];
            array2[j * 512 + i] = temp;
        }
    }
    
    /* Final checksum computation */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 1024) {
        checksum += array1[i];
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 512) {
        checksum += array2[i];
    }
    for (size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i += 256) {
        checksum += array3[i];
    }
    for (int i = 0; i < 256; i++) {
        checksum += histogram[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
