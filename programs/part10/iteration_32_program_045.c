/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in GCC's driver-i386.cc, specifically targeting the switch cases
 * for various cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * It uses multiple techniques to force the compiler driver to
 * evaluate different CPU cache configurations.
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

/* Helper function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, int size, int* result) {
    /* Large array access with stride patterns */
    int temp = 0;
    for (int i = 0; i < size; i += 8) { /* 64-byte stride for cache line */
        temp += data[i];
    }
    for (int i = 0; i < size; i += 16) { /* 128-byte stride */
        temp -= data[i];
    }
    *result = temp;
}

/* Helper function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, int size, int* result) {
    /* Different access pattern */
    int temp = 0;
    for (int i = 0; i < size; i += 4) { /* 32-byte stride */
        temp ^= data[i];
    }
    for (int i = 1; i < size; i += 8) { /* Offset stride */
        temp |= data[i];
    }
    *result = temp;
}

/* Helper function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, int size, int* result) {
    /* Matrix-style access pattern */
    int temp = 0;
    const int dim = 256;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            temp += data[i * dim + j];
        }
    }
    *result = temp;
}

/* Function to read CPUID cache descriptors */
static void read_cpuid_cache_info(uint32_t* descriptors, int* count) {
    uint32_t eax, ebx, ecx, edx;
    int desc_idx = 0;
    
    /* CPUID leaf 2 - Cache and TLB Descriptors */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2));
    
    /* Extract descriptors from eax, ebx, ecx, edx */
    uint32_t regs[4] = {eax, ebx, ecx, edx};
    for (int i = 0; i < 4; i++) {
        uint8_t* bytes = (uint8_t*)&regs[i];
        for (int j = 0; j < 4; j++) {
            if (bytes[j] != 0 && (bytes[j] & 0x80) == 0) {
                descriptors[desc_idx++] = bytes[j];
            }
        }
    }
    *count = desc_idx;
}

/* Function to read deterministic cache parameters */
static void read_deterministic_cache_params(void) {
    uint32_t eax, ebx, ecx, edx;
    int level = 0;
    
    /* Read cache parameters until CPUID leaf 4 returns 0 */
    do {
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(level));
        
        if ((eax & 0x1F) != 0) { /* Cache type field not 0 */
            /* Use the values to prevent optimization */
            volatile uint32_t dummy = eax + ebx + ecx + edx;
            (void)dummy;
        }
        
        level++;
    } while ((eax & 0x1F) != 0 && level < 16);
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (1024 * 1024) /* 1M elements = 4MB */
    static int __attribute__((aligned(64))) big_array[ARRAY_SIZE];
    
    /* Fill array with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        big_array[i] = lcg_rand() % 100;
    }
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        int result = 0;
        for (int i = 0; i < ARRAY_SIZE; i += 16) {
            result += big_array[i];
            __builtin_prefetch(&big_array[i + 64], 0, 3); /* High temporal locality */
        }
        checksum += result;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path */
        int result = 0;
        /* Access pattern that might benefit from larger caches */
        for (int i = 0; i < ARRAY_SIZE; i += 32) {
            result ^= big_array[i];
            __builtin_prefetch(&big_array[i + 128], 0, 1); /* Low temporal locality */
        }
        checksum += result;
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path with matrix transpose pattern */
        int result = 0;
        const int block = 64;
        for (int i = 0; i < ARRAY_SIZE - block; i += block) {
            for (int j = 0; j < block; j++) {
                result += big_array[i + j] * big_array[i + block - j - 1];
            }
        }
        checksum += result;
    }
    #endif
    
    /* Always execute these paths regardless of CPU features */
    {
        /* Random-ish access pattern to stress cache */
        int indices[] = {0, 7, 23, 47, 97, 203, 401, 809, 1601};
        int result = 0;
        for (int i = 0; i < ARRAY_SIZE - 1601; i += 512) {
            for (int j = 0; j < 9; j++) {
                result += big_array[i + indices[j]];
            }
        }
        checksum += result;
    }
    
    /* Call architecture-specific functions */
    {
        int result1 = 0, result2 = 0, result3 = 0;
        core2_optimized_compute(big_array, ARRAY_SIZE, &result1);
        nehalem_optimized_compute(big_array, ARRAY_SIZE, &result2);
        sandybridge_optimized_compute(big_array, ARRAY_SIZE, &result3);
        checksum += result1 + result2 + result3;
    }
    
    /* Read CPUID cache information */
    {
        uint32_t descriptors[32];
        int count = 0;
        read_cpuid_cache_info(descriptors, &count);
        
        /* Use descriptors in checksum */
        for (int i = 0; i < count; i++) {
            checksum += descriptors[i];
        }
        
        /* Also read deterministic cache parameters */
        read_deterministic_cache_params();
    }
    
    /* Additional CPUID leaf 1 for feature bits */
    {
        uint32_t eax, ebx, ecx, edx;
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(1));
        checksum += (eax >> 16) & 0xFF; /* Family */
        checksum += (eax >> 8) & 0xFF;  /* Model */
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to avoid complete optimization */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
