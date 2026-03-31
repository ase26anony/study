/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC compiler driver (driver-i386.cc lines 127-244).
 * It uses multiple techniques to force the driver to evaluate
 * different cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, int size) {
    /* Large stride access pattern that might hint at L1 cache size */
    for (int i = 0; i < size; i += 8) {
        data[i] = data[i] * 3 + 7;
    }
}

/* Helper function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 16) {
        data[i] = data[i] * 5 - 3;
    }
}

/* Helper function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, int size) {
    /* Yet another pattern */
    for (int i = 0; i < size; i += 32) {
        data[i] = data[i] * 2 + data[i + 1];
    }
}

/* Function to read CPUID cache descriptor information */
static void read_cache_descriptors(uint32_t* descriptors, int max_count) {
    uint32_t eax, ebx, ecx, edx;
    int desc_idx = 0;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Store the descriptor bytes */
    if ((eax & 0xFF) > 0) {
        descriptors[desc_idx++] = eax & 0xFF;
    }
    if ((ebx & 0xFF) > 0 && desc_idx < max_count) {
        descriptors[desc_idx++] = ebx & 0xFF;
    }
    if ((ecx & 0xFF) > 0 && desc_idx < max_count) {
        descriptors[desc_idx++] = ecx & 0xFF;
    }
    if ((edx & 0xFF) > 0 && desc_idx < max_count) {
        descriptors[desc_idx++] = edx & 0xFF;
    }
    
    /* Additional descriptor bytes from the upper parts */
    for (int i = 1; i <= 3; i++) {
        if ((ebx >> (8 * i)) & 0xFF) {
            if (desc_idx < max_count) {
                descriptors[desc_idx++] = (ebx >> (8 * i)) & 0xFF;
            }
        }
    }
}

/* Deterministic cache parameters from CPUID leaf 4 */
static void read_deterministic_cache(uint32_t* params) {
    uint32_t eax, ebx, ecx, edx;
    
    for (int i = 0; i < 3; i++) {  /* Check first 3 cache levels */
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
        
        params[i * 4 + 0] = eax;
        params[i * 4 + 1] = ebx;
        params[i * 4 + 2] = ecx;
        params[i * 4 + 3] = edx;
    }
}

int main() {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Large aligned arrays to hint at cache usage */
    __attribute__((aligned(64))) static int large_array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int large_array2[512 * 1024];   /* 2MB */
    __attribute__((aligned(64))) static int large_array3[256 * 1024];   /* 1MB */
    
    /* Fill arrays with pseudo-random data using LCG */
    uint32_t lcg_state = 123456789;
    for (int i = 0; i < 1024 * 1024; i++) {
        lcg_state = lcg_state * 1103515245 + 12345;
        large_array1[i] = (int)(lcg_state & 0x7FFFFFFF);
    }
    
    /* Read cache descriptors - forces driver to handle CPUID results */
    uint32_t cache_descriptors[16] = {0};
    read_cache_descriptors(cache_descriptors, 16);
    
    /* Use descriptors in checksum to prevent elimination */
    for (int i = 0; i < 16; i++) {
        checksum ^= cache_descriptors[i];
    }
    
    /* Read deterministic cache parameters */
    uint32_t cache_params[12] = {0};
    read_deterministic_cache(cache_params);
    
    for (int i = 0; i < 12; i++) {
        checksum += cache_params[i];
    }
    
    /* Conditional compilation paths based on CPU features */
    /* Each path uses different cache access patterns */
    
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* Matrix-style access pattern */
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 1024; j++) {
                large_array1[i * 1024 + j] = 
                    large_array1[i * 1024 + j] * 2 + 
                    large_array2[j * 512 + (i % 512)];
            }
        }
        
        /* Prefetch hints */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            __builtin_prefetch(&large_array1[i + 128], 0, 3);
        }
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* Different stride for AVX */
        for (int i = 0; i < 1024 * 1024; i += 8) {
            large_array2[i % (512 * 1024)] = 
                large_array1[i] * 3 - large_array1[i + 1];
        }
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* Yet another pattern */
        for (int i = 0; i < 256 * 1024; i++) {
            large_array3[i] = 
                (large_array1[i * 4] + large_array2[i * 2]) / 2;
        }
    }
#endif
    
    /* Call architecture-specific functions */
    /* These will cause the driver to consider different cache configs */
    core2_optimized_loop(large_array1, 1024 * 1024);
    nehalem_optimized_loop(large_array2, 512 * 1024);
    sandybridge_optimized_loop(large_array3, 256 * 1024);
    
    /* Additional CPUID calls with different leaves */
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 1 - Feature information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    checksum ^= eax ^ ebx ^ ecx ^ edx;
    
    /* CPUID leaf 0x80000005 - L1 cache information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x80000005)
    );
    checksum += eax + ebx + ecx + edx;
    
    /* CPUID leaf 0x80000006 - L2/L3 cache information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x80000006)
    );
    checksum += eax + ebx + ecx + edx;
    
    /* Final computation to use all arrays and checksum */
    uint32_t final_result = 0;
    for (int i = 0; i < 1024 * 1024; i += 1024) {
        final_result ^= large_array1[i];
    }
    for (int i = 0; i < 512 * 1024; i += 512) {
        final_result ^= large_array2[i];
    }
    for (int i = 0; i < 256 * 1024; i += 256) {
        final_result ^= large_array3[i];
    }
    
    final_result ^= checksum;
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(final_result));
    
    /* Print something to avoid complete optimization */
    printf("Cache test checksum: %u\n", final_result & 0xFF);
    
    return (final_result & 0xFF) == 0 ? 0 : 1;
}
