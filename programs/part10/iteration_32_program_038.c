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
    for (size_t i = 0; i < size; i += 64/sizeof(int)) {
        arr[i] = arr[i] * 3 + 7;
    }
    
    /* Use CPUID leaf 2 (cache descriptors) */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Use results to prevent optimization */
    volatile uint32_t cpuid_sum = eax + ebx + ecx + edx;
    (void)cpuid_sum;
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Strided access pattern (every 256 bytes) */
    for (size_t i = 0; i < size; i += 256/sizeof(int)) {
        arr[i] = arr[i] * 5 + 11;
    }
    
    /* Use CPUID leaf 4 (deterministic cache parameters) */
    for (int i = 0; i < 4; i++) {
        uint32_t eax, ebx, ecx, edx;
        eax = 4;
        ecx = i;
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(eax), "c"(ecx)
        );
        
        volatile uint32_t cache_info = eax & 0x1F; /* Cache type */
        (void)cache_info;
    }
}

/* AVX-optimized function */
__attribute__((target("avx")))
void avx_optimized_computation(float* data, size_t size) {
    /* Simple vectorizable computation */
    for (size_t i = 0; i < size; i += 8) {
        data[i] = data[i] * 2.0f;
    }
    
    /* Explicit prefetching */
    for (size_t i = 0; i < size; i += 128/sizeof(float)) {
        __builtin_prefetch(&data[i + 32], 0, 3);
    }
}

/* SSE4.2 optimized function */
__attribute__((target("sse4.2")))
void sse42_optimized_computation(int* data, size_t size) {
    /* Random-ish access pattern */
    for (size_t i = 0; i < size; i += 17) {
        data[i % size] = data[i % size] ^ 0x5A5A5A5A;
    }
}

int main() {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
    int* __attribute__((aligned(64))) large_array = 
        (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float* __attribute__((aligned(64))) float_array = 
        (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    if (!large_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        large_array[i] = lcg_rand();
        float_array[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Conditional code paths based on CPU features */
    if (__builtin_cpu_supports("avx")) {
        avx_optimized_computation(float_array, ARRAY_SIZE);
        checksum += 0xAVX;
        
        /* Matrix-style access pattern */
        const int dim = 512;
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                float_array[i * dim + j] *= 1.01f;
            }
        }
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        sse42_optimized_computation(large_array, ARRAY_SIZE);
        checksum += 0xSSE42;
        
        /* Histogram-like access pattern */
        int histogram[256] = {0};
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            histogram[large_array[i] & 0xFF]++;
        }
        for (int i = 0; i < 256; i++) {
            checksum += histogram[i];
        }
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* Different stride pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 128/sizeof(int)) {
            large_array[i] = large_array[i] * 2 - 1;
        }
        checksum += 0xSSE2;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(large_array, ARRAY_SIZE);
    nehalem_optimized_loop(large_array, ARRAY_SIZE);
    
    /* Additional CPUID usage for cache descriptors */
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 0 - get vendor string and max leaf */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    checksum += eax; /* max leaf */
    
    /* CPUID leaf 1 - processor info and feature bits */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    checksum += (ecx & 0xFF); /* some feature bits */
    
    /* Complex access pattern to hint at cache line usage */
    for (int stride = 64; stride <= 256; stride *= 2) {
        for (size_t i = 0; i < ARRAY_SIZE; i += stride/sizeof(int)) {
            large_array[i] = (large_array[i] << 3) | (large_array[i] >> 29);
        }
    }
    
    /* Prevent dead code elimination */
    volatile uint64_t final_checksum = checksum;
    
    /* Use checksum in inline asm to ensure side effects */
    __asm__ volatile("" : : "r"(final_checksum));
    
    /* Cleanup */
    free(large_array);
    free(float_array);
    
    return 0;
}
