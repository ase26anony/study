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
    /* Strided access pattern that may hint at L1 cache size */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Small working set that fits in Core2's typical L1 */
    int temp[256];
    for (int i = 0; i < 256; i++) {
        temp[i] = i;
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different stride for Nehalem's cache architecture */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 5 - 11;
    }
    /* Prefetching pattern */
    for (size_t i = 0; i < size; i += 64) {
        __builtin_prefetch(&arr[i + 128], 0, 3);
    }
}

/* Helper function with Sandy Bridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* AVX-optimized access pattern */
    for (size_t i = 0; i < size; i += 32) {
        arr[i] = arr[i] * 2 + 1;
    }
}

/* Function that directly reads CPUID cache descriptors */
static uint32_t read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t checksum = 0;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple calls) */
    asm volatile (
        "mov $2, %%eax\n\t"
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        :
        : 
    );
    
    checksum += eax + ebx + ecx + edx;
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "mov $4, %%eax\n\t"
            "mov %1, %%ecx\n\t"
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "r"(i)
            : 
        );
        checksum += eax + ebx + ecx + edx;
    }
    
    return checksum;
}

/* Large matrix multiplication to stress cache hierarchy */
static void matrix_multiply_cache_test(void) {
    const int N = 256;
    static int A[256][256] __attribute__((aligned(64)));
    static int B[256][256] __attribute__((aligned(64)));
    static int C[256][256] __attribute__((aligned(64)));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = lcg_rand() % 100;
            B[i][j] = lcg_rand() % 100;
            C[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication for better cache utilization */
    const int BLOCK = 32;
    for (int i0 = 0; i0 < N; i0 += BLOCK) {
        for (int j0 = 0; j0 < N; j0 += BLOCK) {
            for (int k0 = 0; k0 < N; k0 += BLOCK) {
                for (int i = i0; i < i0 + BLOCK && i < N; i++) {
                    for (int j = j0; j < j0 + BLOCK && j < N; j++) {
                        int sum = C[i][j];
                        for (int k = k0; k < k0 + BLOCK && k < N; k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Function with architecture-specific compilation paths */
static void arch_specific_code(void) {
    /* Large array that exceeds L1 but fits in L2 */
    static int large_array[512 * 1024] __attribute__((aligned(64)));
    
    /* Initialize array */
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i++) {
        large_array[i] = lcg_rand() % 256;
    }
    
    /* Different code paths based on compiler-defined macros */
#ifdef __SSE4_2__
    /* SSE4.2 optimized path */
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i += 4) {
        large_array[i] = large_array[i] ^ 0xAAAAAAAA;
    }
#endif
    
#ifdef __AVX__
    /* AVX optimized path */
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i += 8) {
        large_array[i] = large_array[i] * 2;
    }
#endif
    
#ifdef __AVX2__
    /* AVX2 optimized path */
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i += 16) {
        large_array[i] = large_array[i] + 1;
    }
#endif
    
    /* Call target-specific functions */
    core2_optimized_loop(large_array, sizeof(large_array)/sizeof(large_array[0]));
    nehalem_optimized_loop(large_array, sizeof(large_array)/sizeof(large_array[0]));
    sandybridge_optimized_loop(large_array, sizeof(large_array)/sizeof(large_array[0]));
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Execute different code paths based on CPU support */
    if (__builtin_cpu_supports("sse2")) {
        checksum += 0x1;
        /* SSE2-optimized array processing */
        int sse2_array[4096] __attribute__((aligned(16)));
        for (int i = 0; i < 4096; i++) {
            sse2_array[i] = i;
        }
        for (int i = 0; i < 4096; i += 2) {
            sse2_array[i] = sse2_array[i] * 3;
        }
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 0x2;
        /* More aggressive optimization for SSE4.2 */
        volatile int temp = 0;
        for (int i = 0; i < 1000; i++) {
            temp += i;
        }
    }
    
    if (__builtin_cpu_supports("avx")) {
        checksum += 0x4;
        /* AVX-optimized computation */
        float avx_array[1024] __attribute__((aligned(32)));
        for (int i = 0; i < 1024; i++) {
            avx_array[i] = i * 0.5f;
        }
    }
    
    if (__builtin_cpu_supports("avx2")) {
        checksum += 0x8;
        /* AVX2-specific operations */
        long avx2_array[2048] __attribute__((aligned(32)));
        for (int i = 0; i < 2048; i++) {
            avx2_array[i] = i * 3L;
        }
    }
    
    /* Read CPUID cache information directly */
    checksum += read_cpuid_cache_descriptors();
    
    /* Perform cache-intensive matrix operations */
    matrix_multiply_cache_test();
    
    /* Execute architecture-specific code paths */
    arch_specific_code();
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print checksum to ensure side effects are visible */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
