/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Creating architecture-dependent code paths
 * 3. Embedding CPUID inline assembly
 * 4. Using target attributes for different architectures
 * 5. Using large array access patterns
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
void core2_optimized_compute(int* data, int size, int* result) {
    /* Large array access with stride patterns */
    int temp = 0;
    for (int i = 0; i < size; i += 8) { /* 64-byte stride for cache lines */
        temp += data[i];
    }
    /* Matrix-style access pattern */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            temp += data[(i * 1024 + j) % size];
        }
    }
    *result = temp;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, int size, int* result) {
    int temp = 0;
    /* Different access pattern */
    for (int i = 0; i < size; i += 16) { /* 128-byte stride */
        temp += data[i];
    }
    /* Reverse access pattern */
    for (int i = size - 1; i >= 0; i -= 4) {
        temp += data[i];
    }
    *result = temp;
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, int size, int* result) {
    int temp = 0;
    /* Random-ish access to stress cache */
    for (int i = 0; i < 100000; i++) {
        uint32_t idx = lcg_rand() % size;
        temp += data[idx];
        __builtin_prefetch(&data[(idx + 64) % size], 0, 3); /* High temporal locality */
    }
    *result = temp;
}

/* Function to read CPUID cache descriptors */
static void read_cpuid_cache_info(uint32_t* descriptors, int* count) {
    uint32_t eax, ebx, ecx, edx;
    int desc_idx = 0;
    
    /* CPUID leaf 2 - Cache descriptors (legacy method) */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2));
    
    /* Extract cache descriptors from eax, ebx, ecx, edx */
    uint8_t* regs = (uint8_t*)&eax;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
            descriptors[desc_idx++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
            descriptors[desc_idx++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
            descriptors[desc_idx++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (regs[i] != 0 && (regs[i] & 0x80) == 0) {
            descriptors[desc_idx++] = regs[i];
        }
    }
    
    *count = desc_idx;
}

/* Function to read deterministic cache parameters */
static void read_deterministic_cache_params(uint32_t* params) {
    uint32_t eax, ebx, ecx, edx;
    int cache_level = 0;
    
    for (int i = 0; i < 4; i++) { /* Check up to 4 cache levels */
        asm volatile ("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(4), "c"(i));
        
        if ((eax & 0x1F) == 0) break; /* No more caches */
        
        params[cache_level * 4 + 0] = eax;
        params[cache_level * 4 + 1] = ebx;
        params[cache_level * 4 + 2] = ecx;
        params[cache_level * 4 + 3] = edx;
        cache_level++;
    }
}

int main(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
    int* __attribute__((aligned(64))) data_array = 
        (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (!data_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = lcg_rand() % 100;
    }
    
    /* CPU feature detection - creates different code paths */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        int result = 0;
        for (int i = 0; i < ARRAY_SIZE; i += 4) {
            result += data_array[i];
        }
        checksum += result;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        int result = 0;
        /* AVX-optimized access pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            result += data_array[i];
        }
        checksum += result;
        
        /* Call AVX-targeted function */
        sandybridge_optimized_compute(data_array, ARRAY_SIZE, &result);
        checksum += result;
    }
    #endif
    
    #ifdef __SSE2__
    if (__builtin_cpu_supports("sse2")) {
        int result = 0;
        /* SSE2-optimized pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 16) {
            result += data_array[i];
            __builtin_prefetch(&data_array[i + 256], 0, 0); /* Low temporal locality */
        }
        checksum += result;
    }
    #endif
    
    /* Always execute these for broader coverage */
    {
        int result1 = 0, result2 = 0;
        core2_optimized_compute(data_array, ARRAY_SIZE, &result1);
        nehalem_optimized_compute(data_array, ARRAY_SIZE, &result2);
        checksum += result1 + result2;
    }
    
    /* Read CPUID cache information */
    uint32_t cache_descriptors[16] = {0};
    int desc_count = 0;
    read_cpuid_cache_info(cache_descriptors, &desc_count);
    
    /* Use descriptors in checksum */
    for (int i = 0; i < desc_count; i++) {
        checksum += cache_descriptors[i];
    }
    
    /* Read deterministic cache parameters */
    uint32_t cache_params[16] = {0};
    read_deterministic_cache_params(cache_params);
    
    for (int i = 0; i < 16; i++) {
        checksum += cache_params[i];
    }
    
    /* Complex access patterns to hint cache usage */
    {
        /* Matrix multiplication pattern */
        #define MATRIX_SIZE 512
        int temp = 0;
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                for (int k = 0; k < MATRIX_SIZE; k++) {
                    temp += data_array[(i * MATRIX_SIZE + k) % ARRAY_SIZE] *
                            data_array[(k * MATRIX_SIZE + j) % ARRAY_SIZE];
                }
            }
        }
        checksum += temp;
        
        /* Histogram pattern */
        int histogram[256] = {0};
        for (int i = 0; i < ARRAY_SIZE; i++) {
            histogram[data_array[i] % 256]++;
        }
        for (int i = 0; i < 256; i++) {
            checksum += histogram[i];
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to avoid optimization */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    free(data_array);
    return 0;
}
