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
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
        
        volatile uint32_t cache_info = eax;
        (void)cache_info;
    }
}

/* AVX-optimized function */
__attribute__((target("avx")))
void avx_optimized_loop(float* arr, size_t size) {
    /* Vector-friendly access pattern */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 1.5f;
    }
    
    /* Prefetch hints */
    for (size_t i = 0; i < size; i += 128/sizeof(float)) {
        __builtin_prefetch(&arr[i + 128/sizeof(float)], 0, 3);
    }
}

/* SSE4.2 optimized function */
__attribute__((target("sse4.2")))
void sse42_optimized_loop(short* arr, size_t size) {
    /* Medium stride pattern */
    for (size_t i = 0; i < size; i += 32/sizeof(short)) {
        arr[i] = arr[i] * 2;
    }
}

int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    #define L1_SIZE (32 * 1024 / sizeof(int))  /* ~L1 cache size */
    #define L2_SIZE (256 * 1024 / sizeof(int)) /* ~L2 cache size */
    
    __attribute__((aligned(64))) int l1_array[L1_SIZE];
    __attribute__((aligned(64))) int l2_array[L2_SIZE];
    __attribute__((aligned(64))) float float_array[L2_SIZE];
    __attribute__((aligned(64))) short short_array[L2_SIZE * 2];
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < L1_SIZE; i++) {
        l1_array[i] = lcg_rand();
    }
    for (size_t i = 0; i < L2_SIZE; i++) {
        l2_array[i] = lcg_rand();
        float_array[i] = (float)lcg_rand() / 1000.0f;
    }
    for (size_t i = 0; i < L2_SIZE * 2; i++) {
        short_array[i] = (short)(lcg_rand() & 0xFFFF);
    }
    
    /* Conditional code paths based on CPU support */
    if (__builtin_cpu_supports("avx")) {
        avx_optimized_loop(float_array, L2_SIZE);
        checksum += (uint64_t)float_array[0];
        
        /* Matrix-style access pattern */
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 1024; j++) {
                float_array[(i * 64 + j) % L2_SIZE] += 0.1f;
            }
        }
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        sse42_optimized_loop(short_array, L2_SIZE * 2);
        checksum += short_array[0];
        
        /* Histogram-like access pattern */
        int histogram[256] = {0};
        for (size_t i = 0; i < L2_SIZE * 2; i++) {
            histogram[short_array[i] & 0xFF]++;
        }
        for (int i = 0; i < 256; i++) {
            checksum += histogram[i];
        }
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* Random-ish access pattern */
        for (int i = 0; i < 10000; i++) {
            uint32_t idx = lcg_rand() % L2_SIZE;
            l2_array[idx] = l2_array[idx] * 3 + 1;
        }
        checksum += l2_array[0];
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(l1_array, L1_SIZE);
    nehalem_optimized_loop(l2_array, L2_SIZE);
    
    /* Additional CPUID reads for various cache leaves */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 0x80000005 (L1 cache) */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x80000005)
    );
    checksum += eax + ebx;
    
    /* Leaf 0x80000006 (L2/L3 cache) */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x80000006)
    );
    checksum += ecx + edx;
    
    /* Compute final checksum from all arrays */
    for (size_t i = 0; i < L1_SIZE; i += 128/sizeof(int)) {
        checksum += l1_array[i];
    }
    for (size_t i = 0; i < L2_SIZE; i += 512/sizeof(int)) {
        checksum += l2_array[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
