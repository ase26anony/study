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
static inline uint32_t lcg_rand(void) {
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
    /* Some strided access */
    for (size_t i = 0; i < 1024; i++) {
        arr[(i * 17) % size] += i;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* More aggressive prefetching pattern */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_prefetch(&arr[i + 64], 0, 3);
        arr[i] = arr[i] * 5 - 3;
    }
    /* Reverse access pattern */
    for (size_t i = size - 1; i > 0; i -= 4) {
        arr[i] += arr[i - 1];
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* AVX-friendly pattern */
    for (size_t i = 0; i < size; i += 32) {
        arr[i] = arr[i] ^ 0xAAAAAAAA;
    }
    /* Random-ish access to stress cache */
    for (size_t i = 0; i < 2048; i++) {
        size_t idx = (i * 97) % size;
        arr[idx] = arr[idx] * 2 + 1;
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
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
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
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
            array1[i] = array1[i] + array2[i];
            checksum += array1[i];
        }
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            array2[i] = array2[i] * 3 - 5;
            checksum += array2[i];
        }
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            array1[i] = array1[i] ^ array2[i];
            checksum += array1[i];
        }
    }
    #endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, ARRAY_SIZE);
    nehalem_optimized_loop(array2, ARRAY_SIZE);
    sandybridge_optimized_loop(array1, ARRAY_SIZE);
    
    /* Read CPUID cache information */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 2 - Cache descriptors */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 4 - Deterministic cache parameters (first level) */
    cpuid_deterministic_cache(0, &eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Matrix multiplication-like pattern to hint cache blocking */
    #define MATRIX_SIZE 512
    int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
    int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
    int matrix_c[MATRIX_SIZE][MATRIX_SIZE];
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = lcg_rand() % 100;
            matrix_b[i][j] = lcg_rand() % 100;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication with different strides */
    const int BLOCK_SIZE = 32;
    for (int bi = 0; bi < MATRIX_SIZE; bi += BLOCK_SIZE) {
        for (int bj = 0; bj < MATRIX_SIZE; bj += BLOCK_SIZE) {
            for (int bk = 0; bk < MATRIX_SIZE; bk += BLOCK_SIZE) {
                for (int i = bi; i < bi + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (int j = bj; j < bj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                        int sum = matrix_c[i][j];
                        for (int k = bk; k < bk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                        }
                        matrix_c[i][j] = sum;
                        checksum += sum;
                    }
                }
            }
        }
    }
    
    /* Use checksum to prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
