/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Creating multiple compilation units with different -march flags
 * 3. Embedding CPUID inline assembly
 * 4. Using target attributes for different architectures
 * 5. Using large array access patterns with prefetch hints
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

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, int size, int* result) {
    /* Access pattern that might hint at L1/L2 cache usage */
    int sum = 0;
    for (int i = 0; i < size; i += 8) {  /* 64-byte stride for cache lines */
        sum += data[i];
    }
    /* Force CPUID to be executed during compilation analysis */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)  /* Cache descriptor leaf */
        : "memory"
    );
    *result = sum + (eax & 0xFF);
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, int size, int* result) {
    int sum = 0;
    /* Different access pattern */
    for (int i = 0; i < size; i += 16) {  /* 64-byte stride */
        __builtin_prefetch(&data[i + 64], 0, 3);  /* High temporal locality */
        sum += data[i];
    }
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(4), "c"(0)  /* Deterministic cache parameters */
        : "memory"
    );
    *result = sum + (ebx & 0xFF);
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, int size, int* result) {
    int sum = 0;
    /* Matrix-style access pattern */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            sum += data[(i * 1024 + j) % size];
        }
    }
    *result = sum;
}

/* AVX-optimized function */
__attribute__((target("avx")))
void avx_optimized_compute(float* data, int size, float* result) {
    float sum = 0.0f;
    for (int i = 0; i < size; i += 8) {
        sum += data[i];
    }
    *result = sum;
}

/* SSE4.2 optimized function */
__attribute__((target("sse4.2")))
void sse42_optimized_compute(int* data, int size, int* result) {
    int sum = 0;
    for (int i = 0; i < size; i += 4) {
        sum += data[i];
    }
    *result = sum;
}

/* Main computation function with multiple optimization paths */
int perform_computations() {
    int checksum = 0;
    
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays */
    #define ARRAY_SIZE (2 * 1024 * 1024)  /* 2MB to exceed L1/L2 */
    static int __attribute__((aligned(64))) big_array[ARRAY_SIZE];
    static float __attribute__((aligned(64))) float_array[ARRAY_SIZE];
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        big_array[i] = lcg_rand() % 100;
        float_array[i] = (lcg_rand() % 100) / 100.0f;
    }
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        int sse_result;
        sse42_optimized_compute(big_array, ARRAY_SIZE, &sse_result);
        checksum += sse_result;
        
        /* Additional CPUID for cache info */
        uint32_t eax, ebx, ecx, edx;
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(2)
        );
        checksum += (eax & 0xFF) + (ebx & 0xFF) + (ecx & 0xFF) + (edx & 0xFF);
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        float avx_result;
        avx_optimized_compute(float_array, ARRAY_SIZE, &avx_result);
        checksum += (int)avx_result;
        
        /* Read deterministic cache parameters */
        for (int i = 0; i < 4; i++) {
            uint32_t eax, ebx, ecx, edx;
            asm volatile (
                "cpuid"
                : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                : "a"(4), "c"(i)
            );
            checksum += (eax & 0xFFF);
        }
    }
    #endif
    
    /* Always execute these to ensure driver evaluates multiple targets */
    int core2_result, nehalem_result, sandybridge_result;
    core2_optimized_compute(big_array, ARRAY_SIZE, &core2_result);
    nehalem_optimized_compute(big_array, ARRAY_SIZE, &nehalem_result);
    sandybridge_optimized_compute(big_array, ARRAY_SIZE, &sandybridge_result);
    
    checksum += core2_result + nehalem_result + sandybridge_result;
    
    /* Complex access pattern to hint at cache behavior */
    int temp_sum = 0;
    for (int stride = 64; stride <= 1024; stride *= 2) {
        for (int i = 0; i < ARRAY_SIZE; i += stride) {
            temp_sum += big_array[i];
            if (stride == 256 || stride == 512) {
                __builtin_prefetch(&big_array[i + stride], 0, 1);
            }
        }
    }
    checksum += temp_sum;
    
    /* Random-ish access to defeat prefetching */
    for (int i = 0; i < 100000; i++) {
        int idx = lcg_rand() % ARRAY_SIZE;
        checksum += big_array[idx];
    }
    
    return checksum;
}

int main() {
    printf("Starting CPU cache detection test...\n");
    
    /* Force evaluation of multiple CPUID leaves during compilation */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 0 - Vendor ID */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    /* Leaf 1 - Feature information */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    /* Leaf 2 - Cache descriptors (triggers the switch cases) */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(2));
    
    /* Leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 8; i++) {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(i)
        );
    }
    
    int result = perform_computations();
    
    /* Use result to prevent dead code elimination */
    __asm__ volatile("" : : "r"(result));
    
    printf("Computation checksum: %d\n", result);
    return 0;
}
