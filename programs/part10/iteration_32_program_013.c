/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc) by using various CPUID-related
 * intrinsics, inline assembly, and architecture-specific code paths.
 * It aims to cover the switch cases for cache descriptor bytes
 * (0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b, 0x3c,
 *  0x3d, 0x3e, 0x41-0x45, 0x48, 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x80,
 *  0x82-0x87).
 */

#include <stdint.h>
#include <stdio.h>
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

/* Helper function with target attribute for Skylake architecture.
 * This may cause the driver to evaluate cache parameters for Skylake.
 */
__attribute__((target("arch=skylake")))
void skylake_optimized_loop(int* arr, int size) {
    for (int i = 0; i < size; i += 64) {
        arr[i] = arr[i] * 11 - 5;
    }
}

/* Inline assembly to execute CPUID leaf 2 (cache descriptors).
 * This forces the driver to handle CPUID results during compilation.
 */
static inline void cpuid_leaf2(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(2), "c"(0));
}

/* Inline assembly to execute CPUID leaf 4 (deterministic cache parameters).
 * This also influences driver cache detection.
 */
static inline void cpuid_leaf4(uint32_t* eax, uint32_t* ebx,
                               uint32_t* ecx, uint32_t* edx,
                               uint32_t leaf) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(4), "c"(leaf));
}

/* Function to read cache information via CPUID and use the results.
 * The driver may see these instructions and trigger cache detection.
 */
uint32_t gather_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t checksum = 0;
    
    /* Leaf 2: Cache descriptors */
    cpuid_leaf2(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 4: Deterministic cache parameters (iterate through cache levels) */
    for (uint32_t leaf = 0; leaf < 8; ++leaf) {
        cpuid_leaf4(&eax, &ebx, &ecx, &edx, leaf);
        if ((eax & 0x1F) == 0) break; /* No more cache levels */
        checksum += eax + ebx + ecx + edx;
    }
    
    return checksum;
}

/* Large array operations with different access patterns to hint cache usage. */
void cache_stress_test(void) {
    /* Large aligned arrays to stress cache detection */
#define ARR_SIZE (1024 * 1024) /* 1M elements */
    static int arr1[ARR_SIZE] __attribute__((aligned(64)));
    static int arr2[ARR_SIZE] __attribute__((aligned(64)));
    static int arr3[ARR_SIZE] __attribute__((aligned(64)));
    
    /* Simple LCG for pseudo-random data */
    uint32_t seed = 123456789;
    for (int i = 0; i < ARR_SIZE; ++i) {
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        arr1[i] = (int)seed;
        arr2[i] = (int)(seed >> 16);
        arr3[i] = (int)(seed >> 8);
    }
    
    /* Sequential access pattern */
    int sum1 = 0;
    for (int i = 0; i < ARR_SIZE; i += 1) {
        sum1 += arr1[i];
        __builtin_prefetch(&arr1[i + 64], 0, 3); /* Medium locality hint */
    }
    
    /* Strided access pattern (every 64 bytes) */
    int sum2 = 0;
    for (int i = 0; i < ARR_SIZE; i += 16) {
        sum2 += arr2[i];
        __builtin_prefetch(&arr2[i + 256], 0, 1); /* Low locality hint */
    }
    
    /* "Random-ish" access pattern using a simple hash */
    int sum3 = 0;
    for (int i = 0; i < ARR_SIZE; i += 1) {
        int idx = (i * 97) % ARR_SIZE; /* Simple pseudo-random index */
        sum3 += arr3[idx];
        __builtin_prefetch(&arr3[(idx + 128) % ARR_SIZE], 0, 2); /* High locality hint */
    }
    
    /* Use the sums to prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3));
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Conditionally execute code paths based on CPU support.
     * Each path may cause the driver to evaluate different cache configurations.
     */
    
#ifdef __SSE2__
    if (__builtin_cpu_supports("sse2")) {
        /* Large array with SSE2-optimized access pattern */
        static float sse2_arr[4096] __attribute__((aligned(16)));
        for (int i = 0; i < 4096; ++i) sse2_arr[i] = i * 0.5f;
        float sse2_sum = 0.0f;
        for (int i = 0; i < 4096; i += 4) {
            sse2_sum += sse2_arr[i];
        }
        checksum += (uint32_t)sse2_sum;
    }
#endif
    
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* Different array size and stride for SSE4.2 path */
        static int sse42_arr[8192] __attribute__((aligned(32)));
        for (int i = 0; i < 8192; ++i) sse42_arr[i] = i * 3;
        int sse42_sum = 0;
        for (int i = 0; i < 8192; i += 8) {
            sse42_sum += sse42_arr[i];
        }
        checksum += sse42_sum;
    }
#endif
    
    if (__builtin_cpu_supports("avx")) {
        /* AVX path with larger aligned array */
        static double avx_arr[16384] __attribute__((aligned(32)));
        for (int i = 0; i < 16384; ++i) avx_arr[i] = i * 1.5;
        double avx_sum = 0.0;
        for (int i = 0; i < 16384; i += 4) {
            avx_sum += avx_arr[i];
        }
        checksum += (uint32_t)avx_sum;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path with different access pattern */
        static long long avx2_arr[32768] __attribute__((aligned(64)));
        for (int i = 0; i < 32768; ++i) avx2_arr[i] = i * 7LL;
        long long avx2_sum = 0;
        for (int i = 0; i < 32768; i += 8) {
            avx2_sum += avx2_arr[i];
        }
        checksum += (uint32_t)(avx2_sum & 0xFFFFFFFF);
    }
    
    /* Call target-specific functions to trigger driver cache evaluation
     * for different architectures.
     */
    static int target_arr[2048] __attribute__((aligned(64)));
    for (int i = 0; i < 2048; ++i) target_arr[i] = i;
    
    core2_optimized_loop(target_arr, 2048);
    nehalem_optimized_loop(target_arr, 2048);
    sandybridge_optimized_loop(target_arr, 2048);
    skylake_optimized_loop(target_arr, 2048);
    
    /* Gather cache info via CPUID inline assembly */
    checksum += gather_cache_info();
    
    /* Perform cache stress test with large arrays */
    cache_stress_test();
    
    /* Final dummy computation to use checksum and prevent elimination */
    printf("Checksum: %u\n", checksum);
    __asm__ volatile("" : : "r"(checksum));
    
    return 0;
}
