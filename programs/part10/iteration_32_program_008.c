/* driver_cache_test.c - Target GCC driver cache detection logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function with target attribute for Core 2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* Access pattern that might hint at 32KB L1 cache (0x2c case) */
    for (size_t i = 0; i < size; i += 64) { /* 64-byte stride */
        data[i] = data[i] * 3 + 7;
    }
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 32) { /* 32-byte stride */
        data[i] = data[i] * 5 - 3;
    }
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size) {
    /* Another pattern */
    for (size_t i = 0; i < size; i += 128) { /* 128-byte stride */
        data[i] = data[i] * 2 + data[i + 64];
    }
}

/* Function with AVX target */
__attribute__((target("avx")))
void avx_optimized_compute(float* data, size_t size) {
    /* Simulate AVX operations */
    for (size_t i = 0; i < size; i += 8) {
        data[i] = data[i] * 2.0f;
    }
}

/* Function with SSE4.2 target */
__attribute__((target("sse4.2")))
void sse42_optimized_compute(float* data, size_t size) {
    /* Simulate SSE operations */
    for (size_t i = 0; i < size; i += 4) {
        data[i] = data[i] * 1.5f;
    }
}

/* Read CPUID leaf 2 (cache descriptors) */
static void cpuid_leaf2(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_leaf4(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    __attribute__((aligned(64))) int large_array[1024 * 1024];      /* 4MB */
    __attribute__((aligned(64))) float float_array[512 * 1024];     /* 2MB */
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i++) {
        large_array[i] = lcg_rand() % 1000;
    }
    for (size_t i = 0; i < sizeof(float_array)/sizeof(float_array[0]); i++) {
        float_array[i] = (lcg_rand() % 1000) / 10.0f;
    }
    
    /* Conditional compilation based on CPU features */
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        sse42_optimized_compute(float_array, sizeof(float_array)/sizeof(float_array[0]));
        checksum += 0x1234;
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        avx_optimized_compute(float_array, sizeof(float_array)/sizeof(float_array[0]));
        checksum += 0x5678;
    }
#endif
    
    /* Always check for SSE2 (most x86_64 CPUs have it) */
    if (__builtin_cpu_supports("sse2")) {
        /* Sequential access pattern */
        for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i++) {
            large_array[i] = large_array[i] * 2 - 1;
        }
        checksum += 0x9ABC;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_compute(large_array, sizeof(large_array)/sizeof(large_array[0]));
    nehalem_optimized_compute(large_array + 1024, sizeof(large_array)/sizeof(large_array[0]) - 1024);
    sandybridge_optimized_compute(large_array + 2048, sizeof(large_array)/sizeof(large_array[0]) - 2048);
    
    /* Execute CPUID instructions to force driver cache detection */
    uint32_t eax, ebx, ecx, edx;
    
    /* Read cache descriptors (leaf 2) */
    cpuid_leaf2(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Read deterministic cache parameters for different cache levels */
    for (uint32_t i = 0; i < 4; i++) {
        cpuid_leaf4(i, &eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Additional access patterns to hint at cache behavior */
    
    /* Pattern 1: Sequential with prefetch */
    for (size_t i = 0; i < sizeof(large_array)/sizeof(large_array[0]); i += 16) {
        __builtin_prefetch(&large_array[i + 64], 0, 3);
        large_array[i] = large_array[i] * 3;
    }
    
    /* Pattern 2: Strided access (might trigger different cache assumptions) */
    for (size_t i = 0; i < 8192; i++) {
        size_t idx = (i * 97) % (sizeof(large_array)/sizeof(large_array[0]));
        large_array[idx] = large_array[idx] + i;
    }
    
    /* Pattern 3: Block-wise processing (simulating matrix operations) */
    for (size_t block = 0; block < 64; block++) {
        size_t start = block * 16384;
        for (size_t i = 0; i < 16384 && start + i < sizeof(large_array)/sizeof(large_array[0]); i++) {
            float_array[start + i] = float_array[start + i] * 0.5f + large_array[start + i];
        }
    }
    
    /* Use checksum to prevent dead code elimination */
    __asm__ volatile ("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
