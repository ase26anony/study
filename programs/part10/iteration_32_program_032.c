/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Multiple compilation units with different -march flags
 * 3. Inline assembly with CPUID instructions
 * 4. Compiler pragmas and target attributes
 * 5. Large array access patterns with prefetch hints
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

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* Access pattern that might hint at L1 cache size */
    for (size_t i = 0; i < size; i += 8) {
        data[i] = data[i] * 3 + 7;
    }
    /* Use CPUID to get cache info */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)  /* Leaf 2: Cache and TLB Descriptor */
    );
    /* Use results to prevent optimization */
    asm volatile ("" : : "r"(eax), "r"(ebx), "r"(ecx), "r"(edx));
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(float* data, size_t size) {
    /* Different stride for L2 cache */
    for (size_t i = 0; i < size; i += 16) {
        data[i] = data[i] * 1.5f;
    }
    /* CPUID leaf 4: Deterministic Cache Parameters */
    uint32_t eax, ebx, ecx, edx;
    uint32_t cache_level = 0;
    do {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(cache_level)
        );
        cache_level++;
    } while ((eax & 0x1F) != 0);  /* Continue until cache type = 0 */
}

/* Function with AVX target attribute */
__attribute__((target("avx")))
void avx_optimized_compute(double* data, size_t size) {
    /* Large stride for potential L3 cache */
    for (size_t i = 0; i < size; i += 32) {
        data[i] = data[i] * 2.0;
    }
    /* Prefetch hints */
    for (size_t i = 0; i < size; i += 64) {
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
}

/* SSE4.2 optimized function */
__attribute__((target("sse4.2")))
void sse42_optimized_compute(int64_t* data, size_t size) {
    /* Random-ish access pattern */
    uint32_t idx = 0;
    for (size_t i = 0; i < size / 2; i++) {
        idx = (idx * 1103515245 + 12345) % size;
        data[idx] = data[idx] + i;
    }
}

/* Main computation with architecture-specific paths */
static uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays */
    __attribute__((aligned(64))) static int int_data[1024 * 1024];
    __attribute__((aligned(64))) static float float_data[1024 * 1024];
    __attribute__((aligned(64))) static double double_data[512 * 1024];
    __attribute__((aligned(64))) static int64_t int64_data[256 * 1024];
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < sizeof(int_data)/sizeof(int_data[0]); i++) {
        int_data[i] = lcg_rand();
    }
    for (size_t i = 0; i < sizeof(float_data)/sizeof(float_data[0]); i++) {
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    for (size_t i = 0; i < sizeof(double_data)/sizeof(double_data[0]); i++) {
        double_data[i] = (double)lcg_rand() / 1000.0;
    }
    for (size_t i = 0; i < sizeof(int64_data)/sizeof(int64_data[0]); i++) {
        int64_data[i] = (int64_t)lcg_rand() << 32 | lcg_rand();
    }
    
    /* Conditional execution based on CPU features */
    if (__builtin_cpu_supports("avx")) {
        avx_optimized_compute(double_data, sizeof(double_data)/sizeof(double_data[0]));
        checksum += (uint64_t)double_data[0];
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        sse42_optimized_compute(int64_data, sizeof(int64_data)/sizeof(int64_data[0]));
        for (size_t i = 0; i < 100; i++) {
            checksum += int64_data[i];
        }
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* Matrix-style access pattern */
        const size_t dim = 512;
        for (size_t i = 0; i < dim; i++) {
            for (size_t j = 0; j < dim; j++) {
                int_data[i * dim + j] = int_data[i * dim + j] * 2 - 1;
            }
        }
    }
    
    /* Always call target-specific functions */
    core2_optimized_compute(int_data, sizeof(int_data)/sizeof(int_data[0]));
    nehalem_optimized_compute(float_data, sizeof(float_data)/sizeof(float_data[0]));
    
    /* Additional CPUID calls for various cache leaves */
    uint32_t eax, ebx, ecx, edx;
    
    /* Leaf 0x80000005: L1 Cache and TLB Information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x80000005)
    );
    checksum += eax + ebx + ecx + edx;
    
    /* Leaf 0x80000006: L2 Cache and TLB Information */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x80000006)
    );
    checksum += eax + ebx + ecx + edx;
    
    /* Histogram computation with different strides */
    uint32_t histogram[256] = {0};
    for (size_t i = 0; i < sizeof(int_data)/sizeof(int_data[0]); i += 1) {
        histogram[(uint8_t)int_data[i]]++;
    }
    for (size_t i = 0; i < sizeof(histogram)/sizeof(histogram[0]); i++) {
        checksum += histogram[i];
    }
    
    /* Use all arrays in final computation */
    for (size_t i = 0; i < 1000; i++) {
        checksum += int_data[i] + (uint64_t)float_data[i] + (uint64_t)double_data[i % 100] + int64_data[i % 100];
    }
    
    return checksum;
}

int main(void) {
    printf("Starting CPU cache detection test...\n");
    
    /* Force initialization of CPU features */
    if (!__builtin_cpu_supports("mmx")) {
        printf("MMX not supported - using fallback\n");
    }
    
    uint64_t result = compute_checksum();
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
    
    printf("Computation complete. Checksum: %llu\n", (unsigned long long)result);
    
    return 0;
}
