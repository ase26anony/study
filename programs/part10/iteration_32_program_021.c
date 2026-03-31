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
    /* Sequential access pattern */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Use __builtin_prefetch to hint cache behavior */
    for (size_t i = 0; i < size; i += 64) {
        __builtin_prefetch(&arr[i + 64], 0, 3);
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Strided access pattern (every 16th element) */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 5 - 2;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Reverse sequential access */
    for (size_t i = size - 1; i > 0; i -= 4) {
        arr[i] = arr[i] + arr[i-1];
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

int main() {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to exercise cache detection */
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
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* Matrix-style access pattern */
        const int dim = 512;
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                array1[i * dim + j] = array1[i * dim + j] * 2;
            }
        }
        checksum += array1[dim * dim - 1];
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* Larger stride access */
        for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 32) {
            array2[i] = array2[i] * 3 + 1;
        }
        checksum += array2[1000];
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* Random-ish access pattern using LCG */
        uint32_t idx = 0;
        for (int i = 0; i < 10000; i++) {
            idx = (idx * 1103515245 + 12345) % (sizeof(array3)/sizeof(array3[0]));
            array3[idx] = array3[idx] + i;
        }
        checksum += array3[idx];
    }
#endif
    
    /* Always execute SSE2 path if supported */
    if (__builtin_cpu_supports("sse2")) {
        /* Sequential with occasional skips */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
            if (i % 17 == 0) {
                array1[i] = array1[i] * 7 - 3;
            }
        }
        checksum += array1[17];
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    nehalem_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    sandybridge_optimized_loop(array3, sizeof(array3)/sizeof(array3[0]));
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 2 - Cache descriptors (may trigger switch cases) */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 4 - Deterministic cache parameters */
    for (uint32_t leaf = 0; leaf < 4; leaf++) {
        cpuid_deterministic_cache(leaf, &eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
        
        /* Use cache size information in computation */
        if (leaf == 0) {
            uint32_t cache_size = (eax >> 22) & 0x3FF;
            array1[0] += cache_size;
        }
    }
    
    /* Additional cache-intensive computation */
    /* Histogram calculation */
    int histogram[256] = {0};
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 128) {
        uint8_t value = array1[i] & 0xFF;
        histogram[value]++;
    }
    
    for (int i = 0; i < 256; i++) {
        checksum += histogram[i];
    }
    
    /* Matrix multiplication-like pattern */
    const int n = 128;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += array1[i * n + k] * array2[k * n + j];
            }
            array3[i * n + j] = sum;
            checksum += sum & 0xFF;
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to avoid complete optimization */
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return (int)(checksum % 256);
}
