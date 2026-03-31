/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID and cache-related constructs across multiple compilation units
 * and architectures.
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

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, int size) {
    /* Large stride access pattern */
    for (int i = 0; i < size; i += 16) {
        data[i] = data[i] * 3 + 7;
    }
    /* Prefetch hints */
    for (int i = 0; i < size; i += 32) {
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 8) {
        data[i] = data[i] * 5 - 11;
    }
    for (int i = 0; i < size; i += 64) {
        __builtin_prefetch(&data[i + 128], 1, 2);
    }
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, int size) {
    /* Matrix-style access */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            data[i * 1024 + j] = data[i * 1024 + j] + data[j * 1024 + i];
        }
    }
}

/* Inline assembly to read CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_leaf2(void) {
    uint32_t eax, ebx, ecx, edx;
    /* CPUID leaf 2 - Cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    return eax; /* First descriptor byte in AL */
}

/* Inline assembly to read CPUID leaf 4 (deterministic cache parameters) */
static void read_deterministic_cache_params(int level, uint32_t* eax, uint32_t* ebx, 
                                           uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(level)
    );
}

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    __attribute__((aligned(64))) static int large_array[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int medium_array[256 * 1024];  /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < 1024 * 1024; i++) {
        large_array[i] = lcg_rand() % 1000;
    }
    for (int i = 0; i < 256 * 1024; i++) {
        medium_array[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation based on CPU features */
#if defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__)
    /* SSE4.2/AVX path - different cache behavior */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Sequential access with medium stride */
        for (int i = 0; i < 1024 * 1024; i += 4) {
            large_array[i] = large_array[i] * 2;
        }
        checksum += 0x1234;
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* Larger stride for AVX */
        for (int i = 0; i < 1024 * 1024; i += 8) {
            large_array[i] = large_array[i] + 1;
        }
        checksum += 0x5678;
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* Even larger stride */
        for (int i = 0; i < 1024 * 1024; i += 16) {
            large_array[i] = large_array[i] - 5;
        }
        checksum += 0x9ABC;
    }
#endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(medium_array, 256 * 1024);
    nehalem_optimized_loop(medium_array, 256 * 1024);
    sandybridge_optimized_loop(large_array, 1024 * 1024);
    
    /* Read CPUID cache information */
    uint32_t cache_desc = read_cpuid_leaf2();
    checksum += cache_desc;
    
    /* Read deterministic cache parameters for L1 and L2 */
    uint32_t eax, ebx, ecx, edx;
    read_deterministic_cache_params(0, &eax, &ebx, &ecx, &edx); /* L1 */
    checksum += eax + ebx + ecx + edx;
    
    read_deterministic_cache_params(1, &eax, &ebx, &ecx, &edx); /* L2 */
    checksum += eax + ebx + ecx + edx;
    
    /* Additional optimization-sensitive code */
#ifdef __OPTIMIZE__
    /* Different access pattern when optimizing */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            int idx = (i * 17 + j * 13) % (1024 * 1024);
            large_array[idx] = large_array[idx] * 3 / 2;
        }
    }
    checksum += 0xDEF0;
#endif
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    /* Compute final checksum from array data */
    for (int i = 0; i < 10000; i += 97) {
        checksum += large_array[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
