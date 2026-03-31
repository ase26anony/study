/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, architecture-specific code paths, and cache
 * hinting patterns.
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
void core2_optimized_loop(int *arr, size_t size) {
    /* Sequential access pattern */
    for (size_t i = 0; i < size; i += 64) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Some stride-2 access */
    for (size_t i = 0; i < size; i += 128) {
        arr[i] = arr[i] / 2;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int *arr, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 32) {
        arr[i] = arr[i] + i;
    }
    /* Reverse loop */
    for (size_t i = size - 1; i > 0; i -= 64) {
        arr[i] = arr[i] - 1;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int *arr, size_t size) {
    /* Blocked access pattern */
    const size_t block = 256;
    for (size_t i = 0; i < size; i += block) {
        for (size_t j = i; j < i + block && j < size; j++) {
            arr[j] = arr[j] * 2;
        }
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_descriptors(uint32_t *regs) {
    __asm__ volatile (
        "cpuid"
        : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
        : "a"(2)
    );
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_deterministic_cache(uint32_t *regs, uint32_t index) {
    __asm__ volatile (
        "cpuid"
        : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
        : "a"(4), "c"(index)
    );
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
    int *array1 __attribute__((aligned(64))) = malloc(ARRAY_SIZE * sizeof(int));
    int *array2 __attribute__((aligned(64))) = malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = lcg_rand() % 1000;
        array2[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            array1[i] = array1[i] ^ array2[i];
            /* Prefetch hint */
            if (i + 256 < ARRAY_SIZE) {
                __builtin_prefetch(&array1[i + 256], 0, 3);
            }
        }
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            array2[i] = array2[i] * 3 - array1[i];
            /* Different stride pattern */
            if (i % 3 == 0 && i + 512 < ARRAY_SIZE) {
                __builtin_prefetch(&array2[i + 512], 1, 1);
            }
        }
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path - matrix-like access */
        const size_t stride = 1024;
        for (size_t i = 0; i < ARRAY_SIZE; i += stride) {
            for (size_t j = 0; j < 64 && (i + j) < ARRAY_SIZE; j++) {
                array1[i + j] = array1[i + j] + array2[i + (j * 17) % 64];
            }
        }
    }
    #endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, ARRAY_SIZE);
    nehalem_optimized_loop(array2, ARRAY_SIZE);
    sandybridge_optimized_loop(array1, ARRAY_SIZE);
    
    /* Read CPUID cache information */
    uint32_t cpuid_regs[4];
    
    /* Leaf 2 - Cache descriptors (may trigger multiple switch cases) */
    cpuid_cache_descriptors(cpuid_regs);
    checksum += cpuid_regs[0] + cpuid_regs[1] + cpuid_regs[2] + cpuid_regs[3];
    
    /* Leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        cpuid_deterministic_cache(cpuid_regs, i);
        checksum += cpuid_regs[0] ^ cpuid_regs[1] ^ cpuid_regs[2] ^ cpuid_regs[3];
    }
    
    /* Additional cache-hinting loops with different patterns */
    
    /* Sequential access */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = array1[i] + 1;
    }
    
    /* Strided access (every 64 bytes) */
    for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
        array2[i] = array2[i] * 2;
    }
    
    /* Reverse access */
    for (size_t i = ARRAY_SIZE - 1; i > 0; i -= 32) {
        array1[i] = array1[i] - array2[i];
    }
    
    /* Random-ish access pattern */
    for (size_t i = 0; i < ARRAY_SIZE; i += 97) {
        size_t idx = (i * 17) % ARRAY_SIZE;
        array1[idx] = array1[idx] ^ 0x55AA55AA;
    }
    
    /* Compute final checksum */
    for (size_t i = 0; i < ARRAY_SIZE; i += 1024) {
        checksum += array1[i] + array2[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
