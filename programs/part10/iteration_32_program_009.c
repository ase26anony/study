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
    /* Different stride pattern */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Use CPUID leaf 2 (cache descriptors) */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    /* Use results to prevent elimination */
    arr[0] ^= (eax ^ ebx ^ ecx ^ edx);
}

/* Helper function with Nehalem target */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Sequential access with prefetch hints */
    for (size_t i = 0; i < size; i += 8) {
        __builtin_prefetch(&arr[i + 32], 0, 3);
        arr[i] = arr[i] * 5 - 2;
    }
    /* CPUID leaf 4 (deterministic cache parameters) */
    uint32_t eax, ebx, ecx, edx;
    uint32_t level = 0;
    do {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(level)
        );
        level++;
        arr[1] ^= (eax & 0xFF);
    } while ((eax & 0x1F) != 0);
}

/* Sandy Bridge target function */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* Matrix-style access pattern */
    const int dim = 256;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            arr[i * dim + j] = arr[i * dim + j] + arr[j * dim + i];
        }
    }
}

/* Generic function that will be compiled multiple ways */
#ifdef __SSE4_2__
void sse42_vectorized(int* arr, size_t size) {
    /* Hint at SSE4.2 usage */
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = __builtin_popcount(arr[i]);
    }
}
#endif

#ifdef __AVX__
void avx_vectorized(int* arr, size_t size) {
    /* AVX-optimized pattern */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * arr[i] / 2;
    }
}
#endif

int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays to suggest cache usage */
    #define ARR_SIZE (2 * 1024 * 1024) /* 2MB */
    int* array1 __attribute__((aligned(64))) = (int*)malloc(ARR_SIZE * sizeof(int));
    int* array2 __attribute__((aligned(64))) = (int*)malloc(ARR_SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < ARR_SIZE; i++) {
        array1[i] = lcg_rand() % 1000;
        array2[i] = lcg_rand() % 1000;
    }
    
    uint64_t checksum = 0;
    
    /* Conditional paths based on CPU support */
    if (__builtin_cpu_supports("sse2")) {
        /* Sequential access */
        for (size_t i = 0; i < ARR_SIZE; i++) {
            array1[i] = array1[i] + array2[i];
            checksum += array1[i];
        }
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        #ifdef __SSE4_2__
        sse42_vectorized(array1, ARR_SIZE);
        #endif
        /* Random-ish stride pattern */
        for (size_t i = 0; i < ARR_SIZE; i = i * 2 + 1) {
            if (i >= ARR_SIZE) break;
            checksum ^= array1[i];
        }
    }
    
    if (__builtin_cpu_supports("avx")) {
        #ifdef __AVX__
        avx_vectorized(array2, ARR_SIZE);
        #endif
        /* Another CPUID query */
        uint32_t eax, ebx, ecx, edx;
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(1)
        );
        checksum += (eax ^ ebx ^ ecx ^ edx);
    }
    
    if (__builtin_cpu_supports("avx2")) {
        /* Dense matrix multiplication pattern */
        const int n = 128;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int sum = 0;
                for (int k = 0; k < n; k++) {
                    sum += array1[i * n + k] * array2[k * n + j];
                }
                checksum += sum;
            }
        }
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(array1, ARR_SIZE / 4);
    nehalem_optimized_loop(array2, ARR_SIZE / 4);
    sandybridge_optimized_loop(array1, ARR_SIZE / 4);
    
    /* Final checksum computation */
    for (size_t i = 0; i < ARR_SIZE; i += 64) {
        checksum += array1[i] * array2[i];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
