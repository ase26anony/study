/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC compiler driver (driver-i386.cc lines 127-244).
 * It uses multiple techniques to force the driver to evaluate
 * various CPU cache configurations through CPUID and builtins.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ========== CPUID INLINE ASSEMBLY ========== */
static inline void cpuid(uint32_t leaf, uint32_t subleaf,
                         uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(leaf), "c"(subleaf));
}

/* Read cache descriptors via CPUID leaf 2 */
static uint32_t read_cache_descriptors(uint32_t descriptors[4]) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(2, 0, &eax, &ebx, &ecx, &edx);
    descriptors[0] = eax;
    descriptors[1] = ebx;
    descriptors[2] = ecx;
    descriptors[3] = edx;
    return eax & 0xFF; /* Number of iterations */
}

/* Read deterministic cache parameters via CPUID leaf 4 */
static void read_deterministic_cache(uint32_t level, uint32_t *eax,
                                     uint32_t *ebx, uint32_t *ecx,
                                     uint32_t *edx) {
    cpuid(4, level, eax, ebx, ecx, edx);
}

/* ========== TARGET-SPECIFIC FUNCTIONS ========== */
/* Each function targets a different microarchitecture,
 * potentially triggering different cache descriptor cases.
 */

__attribute__((target("arch=core2")))
void core2_optimized_compute(int *arr, int size) {
    /* May trigger cases: 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, etc. */
    for (int i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 3 + 1;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int *arr, int size) {
    /* May trigger cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, etc. */
    for (int i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 5 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int *arr, int size) {
    /* May trigger cases: 0x2c, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, etc. */
    for (int i = 0; i < size; i += 32) {
        arr[i] = arr[i] * 7 + 3;
    }
}

__attribute__((target("arch=skylake")))
void skylake_optimized_compute(int *arr, int size) {
    /* May trigger cases: 0x48, 0x49, 0x4e, 0x60, etc. */
    for (int i = 0; i < size; i += 64) {
        arr[i] = arr[i] * 11 - 5;
    }
}

/* ========== LARGE ARRAY ACCESS PATTERNS ========== */
#define L1_SIZE (32 * 1024 / sizeof(int))   /* ~32KB */
#define L2_SIZE (256 * 1024 / sizeof(int))  /* ~256KB */
#define L3_SIZE (8192 * 1024 / sizeof(int)) /* ~8MB */

__attribute__((aligned(64))) static int large_array[L3_SIZE * 2];

/* Different access patterns to hint at cache usage */
static int sequential_access(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        __builtin_prefetch(&arr[i + 64], 0, 3); /* Medium locality */
    }
    return sum;
}

static int strided_access(int *arr, int size, int stride) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += arr[i];
        if (i + stride * 16 < size) {
            __builtin_prefetch(&arr[i + stride * 16], 0, 1); /* Low locality */
        }
    }
    return sum;
}

static int randomish_access(int *arr, int size) {
    /* Pseudo-random but deterministic pattern */
    int sum = 0;
    uint32_t idx = 0;
    for (int i = 0; i < size / 4; i++) {
        idx = (idx * 1103515245 + 12345) % size;
        sum += arr[idx];
        if (i % 8 == 0) {
            __builtin_prefetch(&arr[(idx * 16807) % size], 0, 0); /* No locality */
        }
    }
    return sum;
}

/* ========== MAIN DETECTION AND COMPUTATION ========== */
int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Fill array with pseudo-random data */
    uint32_t seed = 42;
    for (int i = 0; i < L3_SIZE * 2; i++) {
        seed = seed * 1103515245 + 12345;
        large_array[i] = (int)(seed % 1000);
    }
    
    /* ===== CPUID CACHE DESCRIPTOR READING ===== */
    uint32_t descriptors[4];
    uint32_t iterations = read_cache_descriptors(descriptors);
    checksum += iterations;
    
    /* Process all descriptor bytes */
    for (int i = 0; i < 4; i++) {
        uint32_t val = descriptors[i];
        for (int byte = 0; byte < 4; byte++) {
            uint8_t desc_byte = (val >> (byte * 8)) & 0xFF;
            if (desc_byte != 0 && desc_byte != 1) {
                checksum += desc_byte; /* Use descriptor bytes */
            }
        }
    }
    
    /* Read deterministic cache parameters for levels 1-3 */
    for (uint32_t level = 0; level <= 2; level++) {
        uint32_t eax, ebx, ecx, edx;
        read_deterministic_cache(level, &eax, &ebx, &ecx, &edx);
        checksum += (eax & 0x1F); /* Cache type */
        checksum += ((eax >> 14) & 0xFFF); /* Number of threads */
    }
    
    /* ===== CONDITIONAL CODE PATHS BASED ON CPU FEATURES ===== */
    /* Each path may cause driver to consider different cache configurations */
    
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += sequential_access(large_array, L2_SIZE);
        checksum += strided_access(large_array + L2_SIZE, L2_SIZE, 4);
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        checksum += randomish_access(large_array + L3_SIZE, L3_SIZE);
        
        /* Matrix-style multiplication pattern */
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 1024; j++) {
                int idx = (i * 1024 + j) % (L3_SIZE * 2);
                large_array[idx] = large_array[idx] * 3 / 2;
            }
        }
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* Different stride pattern */
        checksum += strided_access(large_array, L3_SIZE, 8);
        
        /* Call target-specific functions */
        core2_optimized_compute(large_array, 1024);
        nehalem_optimized_compute(large_array + 1024, 1024);
        sandybridge_optimized_compute(large_array + 2048, 1024);
        skylake_optimized_compute(large_array + 3072, 1024);
    }
#endif
    
    /* Always execute some baseline computation */
    checksum += sequential_access(large_array, L1_SIZE);
    
    /* ===== HISTOGRAM COMPUTATION (CACHE INTENSIVE) ===== */
    int histogram[256] = {0};
    for (int i = 0; i < L2_SIZE; i++) {
        uint8_t val = (large_array[i] & 0xFF);
        histogram[val]++;
        if (i % 32 == 0) {
            __builtin_prefetch(&large_array[i + 128], 0, 2);
        }
    }
    
    for (int i = 0; i < 256; i++) {
        checksum += histogram[i];
    }
    
    /* ===== FINAL OUTPUT ===== */
    /* Use checksum in a way that prevents dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %u\n", checksum);
    return 0;
}
