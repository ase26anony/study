/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, architecture-specific code paths, and cache
 * hinting patterns.
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
    /* Large stride access pattern */
    for (size_t i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Small matrix-style multiplication hint */
    for (size_t i = 0; i < 64; ++i) {
        for (size_t j = 0; j < 64; ++j) {
            arr[(i * 64 + j) % size] += i * j;
        }
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* arr, size_t size) {
    /* Different access pattern */
    for (size_t i = 0; i < size; i += 8) {
        arr[i] = arr[i] / 2 + arr[size - i - 1];
    }
    /* Use prefetch hints */
    for (size_t i = 0; i < size - 64; i += 64) {
        __builtin_prefetch(&arr[i + 64], 0, 3);
        arr[i] = arr[i] ^ arr[i + 32];
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* arr, size_t size) {
    /* AVX-friendly pattern */
    for (size_t i = 0; i < size; i += 32) {
        arr[i] = arr[i] * arr[i] - arr[i + 16];
    }
}

/* Inline CPUID function to read cache descriptors */
static uint32_t read_cpuid_cache_descriptors(uint32_t leaf, uint32_t* eax, 
                                            uint32_t* ebx, uint32_t* ecx, 
                                            uint32_t* edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
    return *eax;
}

/* Function to process cache descriptor bytes */
static uint32_t process_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t checksum = 0;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple calls) */
    read_cpuid_cache_descriptors(2, &eax, &ebx, &ecx, &edx);
    
    /* Process descriptor bytes from registers */
    uint8_t* desc_bytes = (uint8_t*)&eax;
    for (int i = 0; i < 4; i++) {
        if (desc_bytes[i] && desc_bytes[i] != 0xFF) {
            checksum += desc_bytes[i];
        }
    }
    
    desc_bytes = (uint8_t*)&ebx;
    for (int i = 0; i < 4; i++) {
        if (desc_bytes[i] && desc_bytes[i] != 0xFF) {
            checksum += desc_bytes[i];
        }
    }
    
    desc_bytes = (uint8_t*)&ecx;
    for (int i = 0; i < 4; i++) {
        if (desc_bytes[i] && desc_bytes[i] != 0xFF) {
            checksum += desc_bytes[i];
        }
    }
    
    desc_bytes = (uint8_t*)&edx;
    for (int i = 0; i < 4; i++) {
        if (desc_bytes[i] && desc_bytes[i] != 0xFF) {
            checksum += desc_bytes[i];
        }
    }
    
    return checksum;
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Large aligned arrays to hint cache usage */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 512];    /* 1MB */
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = lcg_rand() % 1000;
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = lcg_rand() % 1000;
    }
    
    /* Conditional compilation paths based on CPU features */
    
    /* SSE4.2 path */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Sequential access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]) - 1; i++) {
            array1[i] = array1[i] + array1[i + 1];
        }
        checksum += array1[0];
        
        /* Call architecture-specific functions */
        core2_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    }
    
    /* AVX path */
    if (__builtin_cpu_supports("avx")) {
        /* Strided access pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 64) {
            array1[i] = array1[i] * 2 - array1[i + 32];
        }
        checksum += array1[100];
        
        nehalem_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    }
    
    /* AVX2 path */
    if (__builtin_cpu_supports("avx2")) {
        /* More complex pattern */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]) - 128; i += 128) {
            array1[i] = array1[i] ^ array1[i + 64];
            __builtin_prefetch(&array1[i + 128], 0, 1);
        }
        checksum += array1[1000];
        
        sandybridge_optimized_loop(array2, sizeof(array2)/sizeof(array2[0]));
    }
    
    /* SSE2 fallback path */
    if (__builtin_cpu_supports("sse2")) {
        /* Different stride */
        for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i += 32) {
            array1[i] = array1[i] / 3 + 1;
        }
        checksum += array1[500];
    }
    
    /* Read and process cache descriptors */
    checksum += process_cache_descriptors();
    
    /* Additional CPUID leaf 4 for deterministic cache parameters */
    #ifdef __OPTIMIZE__
    {
        uint32_t eax, ebx, ecx, edx;
        for (int i = 0; i < 4; i++) {
            eax = 4;
            ecx = i;
            __asm__ volatile (
                "cpuid"
                : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                : "a"(4), "c"(i)
            );
            checksum += eax + ebx + ecx + edx;
        }
    }
    #endif
    
    /* Matrix multiplication pattern to hint cache blocking */
    {
        #define N 128
        int matA[N][N], matB[N][N], matC[N][N];
        
        /* Initialize small matrices */
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matA[i][j] = lcg_rand() % 100;
                matB[i][j] = lcg_rand() % 100;
                matC[i][j] = 0;
            }
        }
        
        /* Cache-intensive matrix multiply */
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                for (int j = 0; j < N; j++) {
                    matC[i][j] += matA[i][k] * matB[k][j];
                }
            }
        }
        
        checksum += matC[N-1][N-1];
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(checksum));
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
