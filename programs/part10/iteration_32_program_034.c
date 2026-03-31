/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in GCC's driver-i386.cc, specifically targeting the switch cases
 * for cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * It uses multiple techniques to force the compiler driver to
 * evaluate different CPU cache configurations.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ============================================
 * CPUID inline assembly helpers
 * ============================================ */

static inline void cpuid(uint32_t leaf, uint32_t subleaf,
                         uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(leaf), "c"(subleaf));
}

/* Read CPUID leaf 2 (cache descriptors) */
static void read_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(2, 0, &eax, &ebx, &ecx, &edx);
    /* Use the results to prevent dead code elimination */
    volatile uint32_t dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* Read CPUID leaf 4 (deterministic cache parameters) */
static void read_deterministic_cache(uint32_t cache_level) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(4, cache_level, &eax, &ebx, &ecx, &edx);
    volatile uint32_t dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* ============================================
 * Target-specific functions with different attributes
 * ============================================ */

/* Function optimized for Core2 (may trigger different cache detection) */
__attribute__((target("arch=core2")))
static void core2_optimized_compute(int *arr, size_t size) {
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 1;
    }
}

/* Function using SSE4.2 */
__attribute__((target("sse4.2")))
static void sse42_optimized_compute(int *arr, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        arr[i] = arr[i] * 5 - 2;
    }
}

/* Function using AVX */
__attribute__((target("avx")))
static void avx_optimized_compute(int *arr, size_t size) {
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 7 + 3;
    }
}

/* Function using AVX2 */
__attribute__((target("avx2")))
static void avx2_optimized_compute(int *arr, size_t size) {
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 11 - 5;
    }
}

/* ============================================
 * Large array operations with different access patterns
 * ============================================ */

#define L1_SIZE (32 * 1024 / sizeof(int))   /* ~32KB */
#define L2_SIZE (256 * 1024 / sizeof(int))  /* ~256KB */
#define L3_SIZE (8192 * 1024 / sizeof(int)) /* ~8MB */

/* Sequential access */
static void sequential_access(int *arr, size_t size) {
    int sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += arr[i];
        arr[i] = sum;
    }
    volatile int dummy = sum;
    (void)dummy;
}

/* Strided access (cache unfriendly) */
static void strided_access(int *arr, size_t size, int stride) {
    int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += arr[i];
        arr[i] = sum * 2;
    }
    volatile int dummy = sum;
    (void)dummy;
}

/* Random-ish access pattern */
static void pseudo_random_access(int *arr, size_t size) {
    uint32_t seed = 123456789;
    int sum = 0;
    for (size_t i = 0; i < size; i++) {
        /* Simple LCG */
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        size_t idx = seed % size;
        sum += arr[idx];
        arr[idx] = sum;
    }
    volatile int dummy = sum;
    (void)dummy;
}

/* Matrix-style access (good for cache blocking) */
static void matrix_multiply_style(int *a, int *b, int *c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            int sum = 0;
            for (size_t k = 0; k < n; k++) {
                sum += a[i * n + k] * b[k * n + j];
            }
            c[i * n + j] = sum;
        }
    }
}

/* ============================================
 * Main function with conditional compilation paths
 * ============================================ */

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* ============================================
     * Conditional code paths based on CPU features
     * ============================================ */
    
    /* Path 1: SSE4.2 */
    if (__builtin_cpu_supports("sse4.2")) {
#ifdef __SSE4_2__
        /* Force compiler to consider SSE4.2 path */
        static int arr1[L2_SIZE] __attribute__((aligned(64)));
        
        /* Initialize with pseudo-random data */
        for (size_t i = 0; i < L2_SIZE; i++) {
            arr1[i] = (i * 1103515245 + 12345) & 0x7fff;
        }
        
        sse42_optimized_compute(arr1, L2_SIZE);
        sequential_access(arr1, L2_SIZE);
        
        /* Add to checksum */
        for (size_t i = 0; i < 100; i++) {
            checksum += arr1[i];
        }
#endif
    }
    
    /* Path 2: AVX */
    if (__builtin_cpu_supports("avx")) {
        static int arr2[L2_SIZE * 2] __attribute__((aligned(64)));
        
        for (size_t i = 0; i < L2_SIZE * 2; i++) {
            arr2[i] = (i * 1664525 + 1013904223) & 0x7fff;
        }
        
        avx_optimized_compute(arr2, L2_SIZE * 2);
        strided_access(arr2, L2_SIZE * 2, 17);
        
        for (size_t i = 0; i < 200; i += 3) {
            checksum += arr2[i];
        }
    }
    
    /* Path 3: AVX2 */
    if (__builtin_cpu_supports("avx2")) {
        static int arr3[L3_SIZE] __attribute__((aligned(64)));
        
        for (size_t i = 0; i < L3_SIZE; i++) {
            arr3[i] = (i * 214013 + 2531011) & 0x7fff;
        }
        
        avx2_optimized_compute(arr3, L3_SIZE);
        pseudo_random_access(arr3, L3_SIZE / 4);
        
        for (size_t i = 0; i < 300; i += 5) {
            checksum += arr3[i];
        }
    }
    
    /* Path 4: Generic x86 with prefetch hints */
    {
        static int arr4[L1_SIZE] __attribute__((aligned(64)));
        static int arr5[L1_SIZE] __attribute__((aligned(64)));
        static int arr6[L1_SIZE] __attribute__((aligned(64)));
        
        for (size_t i = 0; i < L1_SIZE; i++) {
            arr4[i] = i;
            arr5[i] = L1_SIZE - i;
            arr6[i] = 0;
        }
        
        /* Use __builtin_prefetch to hint at cache usage */
        for (size_t i = 0; i < L1_SIZE; i += 16) {
            __builtin_prefetch(&arr4[i + 32], 0, 3);
            __builtin_prefetch(&arr5[i + 32], 1, 3);
        }
        
        matrix_multiply_style(arr4, arr5, arr6, 64);
        
        for (size_t i = 0; i < 64; i++) {
            checksum += arr6[i];
        }
    }
    
    /* ============================================
     * Explicit CPUID calls to force cache detection
     * ============================================ */
    
    /* Read cache descriptors (leaf 2) */
    read_cache_descriptors();
    
    /* Read deterministic cache parameters for levels 1-3 */
    for (int i = 0; i < 4; i++) {
        read_deterministic_cache(i);
    }
    
    /* ============================================
     * Call target-specific functions
     * ============================================ */
    
    static int target_arr[L2_SIZE] __attribute__((aligned(64)));
    for (size_t i = 0; i < L2_SIZE; i++) {
        target_arr[i] = (i * 127 + 19) & 0xff;
    }
    
    /* Force consideration of multiple architectures */
    core2_optimized_compute(target_arr, L2_SIZE);
    
    /* ============================================
     * Final checksum computation and output
     * ============================================ */
    
    /* Use checksum in a way that prevents dead code elimination */
    volatile uint64_t final_checksum = checksum;
    
    /* Print minimal output */
    printf("Cache test checksum: %llu\n", (unsigned long long)final_checksum);
    
    return 0;
}
