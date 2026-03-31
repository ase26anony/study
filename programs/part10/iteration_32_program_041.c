/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc) by using various CPUID-based
 * techniques, inline assembly, and architecture-specific code paths.
 * It aims to cover the switch cases for cache descriptor bytes
 * (0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b, 0x3c,
 *  0x3d, 0x3e, 0x41-0x45, 0x48, 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x80,
 *  0x82-0x87) through different compilation scenarios.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper function with target attribute for Core2 architecture.
 * This may cause the driver to evaluate cache parameters for Core2.
 */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* arr, int size) {
    for (int i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 1;
    }
}

/* Helper function with target attribute for Nehalem architecture.
 * This may cause the driver to evaluate cache parameters for Nehalem.
 */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, int size) {
    for (int i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 5 - 2;
    }
}

/* Helper function with target attribute for Sandy Bridge architecture.
 * This may cause the driver to evaluate cache parameters for Sandy Bridge.
 */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, int size) {
    for (int i = 0; i < size; i += 32) {
        arr[i] = arr[i] * 7 + 3;
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static void cpuid_leaf2(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "a" (2)
    );
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_leaf4(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "a" (4), "c" (leaf)
    );
}

/* Function to perform a simple LCG (pseudo-random) fill */
static void lcg_fill(int* arr, int size, uint32_t seed) {
    uint32_t state = seed;
    for (int i = 0; i < size; ++i) {
        state = state * 1103515245 + 12345;
        arr[i] = (int)(state >> 16) & 0x7FFF;
    }
}

/* Matrix multiplication-like access pattern (stride) */
static int matrix_style_access(int* arr, int dim) {
    int sum = 0;
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            sum += arr[i * dim + j];
        }
    }
    return sum;
}

/* Random-ish access pattern using a fixed permutation */
static int permuted_access(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        int idx = (i * 97) % size;  // simple pseudo-random permutation
        sum += arr[idx];
        __builtin_prefetch(&arr[(idx + 64) % size], 0, 0);  // prefetch hint
    }
    return sum;
}

int main() {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define LARGE_SIZE (1024 * 1024)  /* 1M integers */
    int* array1 __attribute__((aligned(64))) = (int*)malloc(LARGE_SIZE * sizeof(int));
    int* array2 __attribute__((aligned(64))) = (int*)malloc(LARGE_SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    lcg_fill(array1, LARGE_SIZE, 42);
    lcg_fill(array2, LARGE_SIZE, 123);
    
    /* Conditional code paths based on CPU support */
    if (__builtin_cpu_supports("avx")) {
        /* AVX path: may trigger different cache detection */
        for (int i = 0; i < LARGE_SIZE; i += 64) {
            array1[i] = array1[i] * 2 + array2[i];
        }
        checksum += 1;
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        for (int i = 0; i < LARGE_SIZE; i += 32) {
            array2[i] = array2[i] * 3 - array1[i];
        }
        checksum += 2;
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path */
        for (int i = 0; i < LARGE_SIZE; i += 16) {
            array1[i] = array1[i] + array2[i] * 4;
        }
        checksum += 4;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, LARGE_SIZE);
    nehalem_optimized_loop(array2, LARGE_SIZE);
    sandybridge_optimized_loop(array1, LARGE_SIZE);
    
    /* Perform different access patterns */
    int dim = 512;  /* 512x512 matrix */
    checksum += matrix_style_access(array1, dim);
    checksum += permuted_access(array2, LARGE_SIZE);
    
    /* Inline CPUID for leaf 2 (cache descriptors) */
    uint32_t eax2, ebx2, ecx2, edx2;
    cpuid_leaf2(&eax2, &ebx2, &ecx2, &edx2);
    checksum += eax2 + ebx2 + ecx2 + edx2;
    
    /* Inline CPUID for leaf 4 (deterministic cache parameters) */
    for (uint32_t leaf = 0; leaf < 4; ++leaf) {
        uint32_t eax4, ebx4, ecx4, edx4;
        cpuid_leaf4(leaf, &eax4, &ebx4, &ecx4, &edx4);
        checksum += eax4 + ebx4 + ecx4 + edx4;
    }
    
    /* Use checksum in a way that prevents dead code elimination */
    __asm__ volatile ("" : : "r" (checksum));
    
    printf("Checksum: %u\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
