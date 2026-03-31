/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPU feature detection mechanisms, inline assembly, and architecture-
 * specific code paths.
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

/* Helper functions with different target attributes */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* arr, int size) {
    /* Sequential access pattern */
    for (int i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, int size) {
    /* Strided access pattern (stride 64 bytes) */
    for (int i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 5 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, int size) {
    /* Reverse sequential access */
    for (int i = size - 1; i >= 0; i -= 16) {
        arr[i] = arr[i] * 11 % 1024;
    }
}

__attribute__((target("arch=skylake")))
void skylake_optimized_loop(int* arr, int size) {
    /* Random-ish access using LCG */
    uint32_t idx = 0;
    for (int i = 0; i < size / 4; i++) {
        idx = (idx * 1103515245 + 12345) % size;
        arr[idx] = arr[idx] + i;
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_descriptors(uint32_t* eax, uint32_t* ebx, 
                                    uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Function to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_deterministic_cache(uint32_t leaf, uint32_t* eax, 
                                      uint32_t* ebx, uint32_t* ecx, 
                                      uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    #define ARRAY_SIZE (1024 * 1024) /* 1M elements */
    static int __attribute__((aligned(64))) large_array[ARRAY_SIZE];
    static int __attribute__((aligned(64))) second_array[ARRAY_SIZE / 2];
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        large_array[i] = lcg_rand() % 1000;
    }
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        second_array[i] = lcg_rand() % 1000;
    }
    
    /* Conditional code paths based on CPU features */
    if (__builtin_cpu_supports("sse2")) {
        /* Matrix-style multiplication (simplified) */
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 1024; j++) {
                int idx = (i * 32 + j) % ARRAY_SIZE;
                large_array[idx] = large_array[idx] * 3 + 7;
            }
        }
        checksum += 1;
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* Different access pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            __builtin_prefetch(&large_array[i + 128], 0, 3);
            large_array[i] = large_array[i] * 2 - large_array[i + 32];
        }
        checksum += 2;
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* More intensive computation */
        for (int i = 0; i < ARRAY_SIZE / 2; i++) {
            second_array[i] = (second_array[i] * second_array[i]) % 1000;
        }
        
        /* Call AVX-targeted function */
        #ifdef __AVX__
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            large_array[i] = large_array[i] * 7 + 11;
        }
        #endif
        checksum += 4;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        /* Another pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 128) {
            large_array[i] = large_array[i] ^ large_array[i + 64];
        }
        checksum += 8;
    }
    
    /* Call architecture-specific functions */
    core2_optimized_loop(large_array, ARRAY_SIZE);
    nehalem_optimized_loop(second_array, ARRAY_SIZE / 2);
    sandybridge_optimized_loop(large_array, ARRAY_SIZE);
    skylake_optimized_loop(second_array, ARRAY_SIZE / 2);
    
    /* Execute CPUID instructions to generate cache descriptors */
    uint32_t eax, ebx, ecx, edx;
    
    /* Read CPUID leaf 2 (cache descriptors) */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Read deterministic cache parameters for first few levels */
    for (int i = 0; i < 3; i++) {
        cpuid_deterministic_cache(i, &eax, &ebx, &ecx, &edx);
        checksum += eax + ebx + ecx + edx;
    }
    
    /* Additional optimization barriers and computations */
    volatile int sink = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 256) {
        sink += large_array[i];
    }
    
    for (int i = 0; i < ARRAY_SIZE / 2; i += 512) {
        sink -= second_array[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Sink: %d\n", sink);
    
    return 0;
}
