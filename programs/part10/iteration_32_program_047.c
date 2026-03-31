/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID queries, architecture-specific code paths, and cache-aware
 * programming patterns.
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
        arr[i] = arr[i] * 5 - 11;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Yet another pattern */
    for (size_t i = 0; i < size; i += 32) {
        arr[i] = (arr[i] << 3) | (arr[i] >> 29);
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_cache_descriptors(uint32_t eax[4]) {
    __asm__ volatile (
        "cpuid"
        : "=a"(eax[0]), "=b"(eax[1]), "=c"(eax[2]), "=d"(eax[3])
        : "a"(2)
    );
    return eax[0];
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void read_deterministic_cache_params(uint32_t level, uint32_t eax[4]) {
    __asm__ volatile (
        "cpuid"
        : "=a"(eax[0]), "=b"(eax[1]), "=c"(eax[2]), "=d"(eax[3])
        : "a"(4), "c"(level)
    );
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 512];    /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = lcg_rand();
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = lcg_rand();
    }
    
    /* Conditional compilation based on CPU features */
#if defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__)
    /* This block will be compiled differently based on -march flags */
    uint32_t cpuid_results[4];
    
    /* Read CPUID leaf 2 (cache descriptors) */
    read_cpuid_cache_descriptors(cpuid_results);
    checksum += cpuid_results[0] + cpuid_results[1] + 
                cpuid_results[2] + cpuid_results[3];
#endif
    
    /* Different code paths based on CPU support */
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized path */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            array1[i] = array1[i] * 2 + 1;
            __builtin_prefetch(&array1[i + 128], 0, 3);
        }
        checksum += 1;
    } else if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        for (int i = 0; i < 512 * 512; i += 32) {
            array2[i] = array2[i] ^ 0x55555555;
            __builtin_prefetch(&array2[i + 64], 1, 2);
        }
        checksum += 2;
    } else if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path */
        for (int i = 0; i < 256 * 256; i += 16) {
            array1[i] = array1[i] * 3;
        }
        checksum += 3;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    nehalem_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    sandybridge_optimized_loop(array1, sizeof(array1)/sizeof(array1[0]));
    
    /* Matrix multiplication-like pattern (cache-intensive) */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            int sum = 0;
            for (int k = 0; k < 256; k++) {
                sum += array1[i * 256 + k] * array2[k * 256 + j];
            }
            checksum += sum;
        }
    }
    
    /* Random-ish access pattern */
    for (int i = 0; i < 10000; i++) {
        uint32_t idx = lcg_rand() % (1024 * 1024);
        array1[idx] = array1[idx] + i;
        checksum += array1[idx];
    }
    
    /* Read deterministic cache parameters */
    for (int level = 0; level < 3; level++) {
        uint32_t eax[4];
        read_deterministic_cache_params(level, eax);
        checksum += eax[0] + eax[1] + eax[2] + eax[3];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
