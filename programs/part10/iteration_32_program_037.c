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
        arr[i] ^= arr[i + 8];
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] + (arr[i] << 2);
    }
    
    /* Reverse loop */
    for (size_t i = size - 1; i > 0; i -= 4) {
        arr[i] = arr[i] - arr[i - 1];
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Matrix-style access pattern */
    const size_t dim = 256;
    for (size_t i = 0; i < dim; ++i) {
        for (size_t j = 0; j < dim; ++j) {
            size_t idx = i * dim + j;
            if (idx < size) {
                arr[idx] = arr[idx] * 2 - 1;
            }
        }
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

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint at cache usage */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 512];    /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); ++i) {
        array1[i] = lcg_rand() % 1000;
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); ++i) {
        array2[i] = lcg_rand() % 1000;
    }
    
    /* Conditional code paths based on CPU support */
    
    /* SSE4.2 path */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Sequential access with prefetch hints */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 64) {
            __builtin_prefetch(&array1[i + 256], 0, 3);
            array1[i] = array1[i] * array1[i + 32];
        }
        checksum += array1[0];
        
        /* Read CPUID leaf 2 */
        uint32_t eax, ebx, ecx, edx;
        cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
    }
    
    /* AVX path */
    if (__builtin_cpu_supports("avx")) {
        /* Strided access pattern */
        for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i += 128) {
            array2[i] = array2[i] + array2[i + 64];
        }
        checksum += array2[0];
        
        /* Read deterministic cache parameters for L1 */
        uint32_t eax, ebx, ecx, edx;
        cpuid_deterministic_cache(0, &eax, &ebx, &ecx, &edx);
        checksum += eax;
    }
    
    /* AVX2 path */
    if (__builtin_cpu_supports("avx2")) {
        /* More complex access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]) - 256; i += 128) {
            array1[i] = array1[i] ^ array1[i + 128] ^ array1[i + 256];
        }
        checksum += array1[100];
        
        /* Read deterministic cache parameters for L2 */
        uint32_t eax, ebx, ecx, edx;
        cpuid_deterministic_cache(1, &eax, &ebx, &ecx, &edx);
        checksum += ebx;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    checksum += array1[1000];
    
    nehalem_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    checksum += array2[1000];
    
    sandybridge_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    checksum += array1[2000];
    
    /* Random-ish access pattern to confuse prefetchers */
    for (int i = 0; i < 10000; ++i) {
        uint32_t idx = lcg_rand() % (sizeof(array1)/sizeof(array1[0]) - 1);
        array1[idx] = array1[idx] + array1[idx + 1];
    }
    checksum += array1[5000];
    
    /* Matrix multiplication-like pattern (cache blocking hint) */
    const size_t n = 128;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            int sum = 0;
            for (size_t k = 0; k < n; ++k) {
                size_t idx_a = i * n + k;
                size_t idx_b = k * n + j;
                if (idx_a < sizeof(array1)/sizeof(array1[0]) && 
                    idx_b < sizeof(array2)/sizeof(array2[0])) {
                    sum += array1[idx_a] * array2[idx_b];
                }
            }
            if (i * n + j < sizeof(array1)/sizeof(array1[0])) {
                array1[i * n + j] = sum;
            }
        }
    }
    checksum += array1[0];
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
