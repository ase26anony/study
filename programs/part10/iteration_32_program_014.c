/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Embedding inline CPUID assembly for cache descriptor leaves
 * 3. Using target attributes for different architectures
 * 4. Creating large array access patterns
 * 5. Compiling with different -march flags across multiple units
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper function to generate pseudo-random data */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* ========== Functions with target attributes ========== */

/* Core2 target - may trigger cache descriptor 0x66, 0x67, 0x68 */
__attribute__((target("arch=core2")))
void core2_compute(int* checksum) {
    /* Large aligned arrays */
    __attribute__((aligned(64))) int arr1[1024 * 16];  /* 64KB */
    __attribute__((aligned(64))) int arr2[1024 * 16];
    
    uint32_t seed = 42;
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < 1024 * 16; i++) {
        arr1[i] = lcg(&seed);
        arr2[i] = lcg(&seed);
    }
    
    /* Matrix-style multiplication with varied stride */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            arr1[i * 16 + j % 16] += arr2[j * 16 + i % 16];
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < 1024 * 16; i++) {
        *checksum += arr1[i] + arr2[i];
    }
}

/* Nehalem target - may trigger cache descriptors 0x2c, 0x41, 0x42 */
__attribute__((target("arch=nehalem")))
void nehalem_compute(int* checksum) {
    __attribute__((aligned(64))) int arr[1024 * 64];  /* 256KB */
    uint32_t seed = 123;
    
    for (int i = 0; i < 1024 * 64; i++) {
        arr[i] = lcg(&seed);
    }
    
    /* Random-ish access pattern */
    for (int i = 0; i < 100000; i++) {
        int idx = lcg(&seed) % (1024 * 64);
        arr[idx] = arr[(idx + 1024) % (1024 * 64)] * 3 + 1;
    }
    
    for (int i = 0; i < 1024 * 64; i += 8) {
        *checksum += arr[i];
    }
}

/* Sandybridge target - may trigger cache descriptors 0x3a, 0x3b, 0x3c */
__attribute__((target("arch=sandybridge")))
void sandybridge_compute(int* checksum) {
    __attribute__((aligned(64))) int arr1[1024 * 128];  /* 512KB */
    __attribute__((aligned(64))) int arr2[1024 * 128];
    uint32_t seed = 456;
    
    for (int i = 0; i < 1024 * 128; i++) {
        arr1[i] = lcg(&seed);
        arr2[i] = lcg(&seed);
    }
    
    /* Use prefetch hints */
    for (int i = 0; i < 1024 * 128 - 64; i += 64) {
        __builtin_prefetch(&arr1[i + 64], 0, 3);
        __builtin_prefetch(&arr2[i + 64], 1, 3);
        arr1[i] = arr1[i] * arr2[i + 32];
    }
    
    for (int i = 0; i < 1024 * 128; i += 16) {
        *checksum += arr1[i] + arr2[i];
    }
}

/* ========== CPUID functions ========== */

/* Execute CPUID leaf 2 (cache descriptors) */
static void cpuid_leaf2(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Execute CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_leaf4(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

/* ========== Main function ========== */

int main() {
    int checksum = 0;
    
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Execute CPUID leaf 2 - cache descriptors */
    uint32_t eax2, ebx2, ecx2, edx2;
    cpuid_leaf2(&eax2, &ebx2, &ecx2, &edx2);
    
    /* Use the results to prevent optimization */
    checksum += eax2 + ebx2 + ecx2 + edx2;
    
    /* Execute CPUID leaf 4 for cache 0 (L1) */
    uint32_t eax4, ebx4, ecx4, edx4;
    cpuid_leaf4(0, &eax4, &ebx4, &ecx4, &edx4);
    checksum += eax4 + ebx4 + ecx4 + edx4;
    
    /* Execute CPUID leaf 4 for cache 1 (L2) */
    cpuid_leaf4(1, &eax4, &ebx4, &ecx4, &edx4);
    checksum += eax4 + ebx4 + ecx4 + edx4;
    
    /* Conditional compilation based on CPU features */
    
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* Large array for SSE4.2 path */
        __attribute__((aligned(64))) float farr[1024 * 32];
        for (int i = 0; i < 1024 * 32; i++) {
            farr[i] = i * 0.1f;
        }
        for (int i = 0; i < 1024 * 32 - 4; i += 4) {
            checksum += (int)farr[i];
        }
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized computation */
        __attribute__((aligned(64))) double darr[1024 * 64];
        for (int i = 0; i < 1024 * 64; i++) {
            darr[i] = i * 0.01;
        }
        for (int i = 0; i < 1024 * 64 - 8; i += 8) {
            checksum += (int)darr[i];
        }
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* Even larger array for AVX2 */
        __attribute__((aligned(64))) int iarr[1024 * 256];  /* 1MB */
        for (int i = 0; i < 1024 * 256; i++) {
            iarr[i] = i;
        }
        for (int i = 0; i < 1024 * 256; i += 32) {
            checksum += iarr[i];
        }
    }
#endif
    
    /* Call target-specific functions */
    core2_compute(&checksum);
    nehalem_compute(&checksum);
    sandybridge_compute(&checksum);
    
    /* Additional CPUID for different leaves */
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 1 for feature bits */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    checksum += (eax >> 16) & 0xFF;  /* Model */
    
    /* CPUID leaf 0 for vendor string */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );
    checksum += ebx + ecx + edx;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
