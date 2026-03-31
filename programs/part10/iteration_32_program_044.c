/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, inline assembly, and architecture-specific
 * code paths.
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

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* Large stride access pattern */
    for (size_t i = 0; i < size; i += 64) {
        data[i] = data[i] * 3 + 7;
    }
    /* Prefetch hints */
    for (size_t i = 0; i < size; i += 128) {
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 32) {
        data[i] = data[i] * 5 - 11;
    }
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    /* Matrix-style access */
    for (size_t i = 0; i < size; i += 16) {
        for (size_t j = 0; j < 16 && (i + j) < size; j++) {
            data[i + j] = data[i + j] + data[i] * 2;
        }
    }
}

/* Function with target attribute for Skylake */
__attribute__((target("arch=skylake")))
void skylake_optimized_compute(int* data, size_t size) {
    /* Random-ish access pattern using LCG */
    uint32_t idx = 0;
    for (size_t i = 0; i < size / 4; i++) {
        idx = (idx + lcg_rand()) % size;
        data[idx] = data[idx] ^ 0xAAAAAAAA;
    }
}

/* Inline assembly to read CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    /* Use all results to prevent optimization */
    return eax + ebx + ecx + edx;
}

/* Inline assembly to read CPUID leaf 4 (deterministic cache parameters) */
static uint32_t read_cpuid_leaf4(uint32_t cache_level) {
    uint32_t eax, ebx, ecx, edx;
    /* CPUID leaf 4 - Deterministic cache parameters */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(cache_level)
    );
    return eax; /* Returns cache type and level info */
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
    int* array1 __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* array2 __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = lcg_rand() % 1000;
        array2[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation paths based on CPU features */
    #ifdef __SSE4_2__
    checksum += 0x1234;
    #endif
    
    #ifdef __AVX__
    checksum += 0x5678;
    #endif
    
    #ifdef __AVX2__
    checksum += 0x9ABC;
    #endif
    
    /* Execute different code paths based on CPU support */
    if (__builtin_cpu_supports("sse2")) {
        /* Sequential access pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            array1[i] = array1[i] * 2 + array2[i];
        }
        checksum += 1;
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* Strided access pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
            array1[i] = array1[i] * 3 - array2[i];
        }
        checksum += 2;
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* Larger stride pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            array1[i] = array1[i] * 5 + array2[i] / 2;
        }
        checksum += 4;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        /* Matrix multiplication-like pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            for (size_t j = 0; j < 8 && (i + j) < ARRAY_SIZE; j++) {
                array1[i + j] = array1[i + j] * array2[i] + j;
            }
        }
        checksum += 8;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_compute(array1, ARRAY_SIZE);
    nehalem_optimized_compute(array2, ARRAY_SIZE);
    sandybridge_optimized_compute(array1, ARRAY_SIZE);
    skylake_optimized_compute(array2, ARRAY_SIZE);
    
    /* Read CPUID cache information */
    checksum += read_cpuid_leaf2();
    
    /* Read deterministic cache parameters for levels 1 and 2 */
    checksum += read_cpuid_leaf4(0); /* Level 1 */
    checksum += read_cpuid_leaf4(1); /* Level 2 */
    
    /* Compute final checksum to prevent dead code elimination */
    for (size_t i = 0; i < ARRAY_SIZE; i += 1024) {
        checksum += array1[i] + array2[i];
    }
    
    /* Use checksum in a way that can't be optimized away */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
