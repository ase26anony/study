/* driver_cache_test.c - Comprehensive test for CPU cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    volatile int sum = 0;
    /* Access pattern that might hint at 32KB L1 cache */
    for (size_t i = 0; i < size; i += 8) {
        sum += data[i];
    }
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum));
}

/* Function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    volatile int sum = 0;
    /* Different stride for different cache behavior */
    for (size_t i = 0; i < size; i += 16) {
        sum += data[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

/* Function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    volatile int sum = 0;
    /* AVX-friendly access pattern */
    for (size_t i = 0; i < size; i += 32) {
        sum += data[i];
    }
    __asm__ volatile("" : : "r"(sum));
}

/* Function to read CPUID cache descriptors */
static void read_cpuid_cache_info(uint32_t leaf) {
    uint32_t eax, ebx, ecx, edx;
    
    /* Read CPUID leaf 2 (cache descriptors) */
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(leaf)
    );
    
    /* Use the results to prevent optimization */
    volatile uint32_t dummy = eax + ebx + ecx + edx;
    (void)dummy;
}

/* Deterministic cache parameters via CPUID leaf 4 */
static void read_deterministic_cache_params() {
    for (uint32_t i = 0; i < 8; i++) {
        uint32_t eax = 4, ecx = i;
        uint32_t ebx, edx;
        
        __asm__ volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(eax), "c"(ecx)
        );
        
        /* Cache type in bits 4:0 */
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) break;
        
        volatile uint32_t dummy = cache_type + ebx + edx;
        (void)dummy;
    }
}

/* Large array operations with different access patterns */
static void cache_sensitive_operations() {
    /* Different sized arrays to trigger different cache behaviors */
    #define L1_SIZE (32 * 1024 / sizeof(int))  /* ~32KB */
    #define L2_SIZE (256 * 1024 / sizeof(int)) /* ~256KB */
    #define L3_SIZE (8192 * 1024 / sizeof(int)) /* ~8MB */
    
    /* Aligned arrays */
    int __attribute__((aligned(64))) l1_array[L1_SIZE];
    int __attribute__((aligned(64))) l2_array[L2_SIZE];
    static int __attribute__((aligned(64))) l3_array[L3_SIZE];
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < L1_SIZE; i++) l1_array[i] = lcg_rand();
    for (size_t i = 0; i < L2_SIZE; i++) l2_array[i] = lcg_rand();
    for (size_t i = 0; i < L3_SIZE; i++) l3_array[i] = lcg_rand();
    
    volatile int checksum = 0;
    
    /* Sequential access - good for prefetching */
    for (size_t i = 0; i < L1_SIZE; i++) {
        checksum += l1_array[i];
        __builtin_prefetch(&l1_array[i + 16], 0, 3);
    }
    
    /* Strided access - may reveal cache line size */
    for (size_t i = 0; i < L2_SIZE; i += 64) {
        checksum += l2_array[i];
    }
    
    /* Random-ish access pattern */
    for (size_t i = 0; i < L3_SIZE; i += 97) { /* Prime stride */
        checksum += l3_array[i % L3_SIZE];
    }
    
    /* Matrix-style access (good for cache blocking detection) */
    const size_t MATRIX_DIM = 256;
    int __attribute__((aligned(64))) matrix_a[MATRIX_DIM][MATRIX_DIM];
    int __attribute__((aligned(64))) matrix_b[MATRIX_DIM][MATRIX_DIM];
    int __attribute__((aligned(64))) matrix_c[MATRIX_DIM][MATRIX_DIM];
    
    /* Simple matrix multiplication */
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        for (size_t k = 0; k < MATRIX_DIM; k++) {
            for (size_t j = 0; j < MATRIX_DIM; j++) {
                matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
            }
        }
    }
    
    /* Use checksum to prevent optimization */
    __asm__ volatile("" : : "r"(checksum));
}

/* Main function with conditional compilation paths */
int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t total_checksum = 0;
    
    /* Conditional code paths based on CPU features */
    #ifdef __SSE2__
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2-optimized path */
        float __attribute__((aligned(16))) sse_data[1024];
        for (int i = 0; i < 1024; i++) sse_data[i] = lcg_rand() / 4294967296.0f;
        
        volatile float sse_sum = 0;
        for (int i = 0; i < 1024; i += 4) {
            sse_sum += sse_data[i];
        }
        total_checksum += (uint64_t)(sse_sum * 1000);
    }
    #endif
    
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2-optimized path */
        int __attribute__((aligned(16))) sse42_data[2048];
        for (int i = 0; i < 2048; i++) sse42_data[i] = lcg_rand();
        
        volatile int sse42_sum = 0;
        for (int i = 0; i < 2048; i += 8) {
            sse42_sum += sse42_data[i];
        }
        total_checksum += sse42_sum;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized path */
        double __attribute__((aligned(32))) avx_data[4096];
        for (int i = 0; i < 4096; i++) avx_data[i] = lcg_rand() / 4294967296.0;
        
        volatile double avx_sum = 0;
        for (int i = 0; i < 4096; i += 4) {
            avx_sum += avx_data[i];
        }
        total_checksum += (uint64_t)(avx_sum * 1000);
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2-optimized path */
        int __attribute__((aligned(32))) avx2_data[8192];
        for (int i = 0; i < 8192; i++) avx2_data[i] = lcg_rand();
        
        volatile int avx2_sum = 0;
        for (int i = 0; i < 8192; i += 8) {
            avx2_sum += avx2_data[i];
        }
        total_checksum += avx2_sum;
    }
    #endif
    
    /* Read CPUID cache information */
    read_cpuid_cache_info(2);  /* Cache descriptors */
    read_deterministic_cache_params();
    
    /* Call target-specific functions */
    int test_data[1024];
    for (int i = 0; i < 1024; i++) test_data[i] = lcg_rand();
    
    core2_optimized_compute(test_data, 1024);
    nehalem_optimized_compute(test_data, 1024);
    sandybridge_optimized_compute(test_data, 1024);
    
    /* Perform cache-sensitive operations */
    cache_sensitive_operations();
    
    /* Final checksum to ensure all computations are used */
    printf("Checksum: %llu\n", (unsigned long long)total_checksum);
    
    return 0;
}
