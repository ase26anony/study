/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC compiler driver (driver-i386.cc lines 127-244).
 * It uses multiple techniques to force the driver to evaluate
 * different cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.) through
 * CPUID interrogation during compilation.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ============================================
   CPUID inline assembly helpers
   ============================================ */

static inline void cpuid(uint32_t leaf, uint32_t subleaf,
                         uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(leaf), "c"(subleaf));
}

/* Read CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_cache_descriptors(uint8_t descriptors[16]) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t bytes_read = 0;
    
    cpuid(2, 0, &eax, &ebx, &ecx, &edx);
    
    /* First byte of eax tells how many descriptor bytes follow */
    uint8_t count = eax & 0xFF;
    descriptors[bytes_read++] = (eax >> 8) & 0xFF;
    descriptors[bytes_read++] = (eax >> 16) & 0xFF;
    descriptors[bytes_read++] = (eax >> 24) & 0xFF;
    
    descriptors[bytes_read++] = ebx & 0xFF;
    descriptors[bytes_read++] = (ebx >> 8) & 0xFF;
    descriptors[bytes_read++] = (ebx >> 16) & 0xFF;
    descriptors[bytes_read++] = (ebx >> 24) & 0xFF;
    
    descriptors[bytes_read++] = ecx & 0xFF;
    descriptors[bytes_read++] = (ecx >> 8) & 0xFF;
    descriptors[bytes_read++] = (ecx >> 16) & 0xFF;
    descriptors[bytes_read++] = (ecx >> 24) & 0xFF;
    
    descriptors[bytes_read++] = edx & 0xFF;
    descriptors[bytes_read++] = (edx >> 8) & 0xFF;
    descriptors[bytes_read++] = (edx >> 16) & 0xFF;
    descriptors[bytes_read++] = (edx >> 24) & 0xFF;
    
    return count;
}

/* ============================================
   Target-specific functions with different attributes
   ============================================ */

/* Core2 target - should trigger different cache detection */
__attribute__((target("arch=core2")))
void core2_computation(int *checksum) {
    /* Large aligned array */
    __attribute__((aligned(64))) static int arr[256 * 1024]; /* 1MB */
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < 256 * 1024; i++) {
        arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Access with varying strides to hint at cache usage */
    int sum = 0;
    for (int stride = 1; stride <= 64; stride *= 2) {
        for (int i = 0; i < 256 * 1024; i += stride) {
            sum += arr[i];
            /* Prefetch hint */
            if (i + stride * 16 < 256 * 1024) {
                __builtin_prefetch(&arr[i + stride * 16], 0, 3);
            }
        }
    }
    
    *checksum += sum;
}

/* Nehalem target */
__attribute__((target("arch=nehalem")))
void nehalem_computation(int *checksum) {
    __attribute__((aligned(64))) static float farr[512 * 1024]; /* 2MB */
    
    for (int i = 0; i < 512 * 1024; i++) {
        farr[i] = (i * 0.001f) - 256.0f;
    }
    
    float sum = 0.0f;
    /* Matrix-style access pattern */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 512; j++) {
            sum += farr[i * 512 + j];
        }
    }
    
    *checksum += (int)sum;
}

/* Sandy Bridge target with AVX */
__attribute__((target("arch=sandybridge")))
void sandybridge_computation(int *checksum) {
    __attribute__((aligned(64))) static double darr[256 * 1024]; /* 2MB */
    
    for (int i = 0; i < 256 * 1024; i++) {
        darr[i] = i * 0.0001;
    }
    
    double sum = 0.0;
    /* Large stride to miss caches */
    for (int i = 0; i < 256 * 1024; i += 128) {
        sum += darr[i];
    }
    
    *checksum += (int)sum;
}

/* Generic function that will be compiled with different -march flags */
void generic_computation(int *checksum) {
    /* Very large stack array (may trigger stack cache heuristics) */
    __attribute__((aligned(64))) int local_arr[64 * 1024]; /* 256KB */
    
    for (int i = 0; i < 64 * 1024; i++) {
        local_arr[i] = i ^ 0x5555;
    }
    
    int sum = 0;
    /* Random-ish access pattern */
    for (int i = 0; i < 1000000; i++) {
        int idx = (i * 97) & (64 * 1024 - 1);
        sum += local_arr[idx];
    }
    
    *checksum += sum;
}

/* ============================================
   Main function with CPU feature detection
   ============================================ */

int main() {
    int checksum = 0;
    
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Force evaluation of various CPUID leaves during compilation */
    uint8_t descriptors[16];
    uint32_t count = read_cpuid_cache_descriptors(descriptors);
    
    /* Use descriptors to influence checksum (prevents dead code elimination) */
    for (uint32_t i = 0; i < count && i < 16; i++) {
        checksum += descriptors[i];
    }
    
    /* Conditional compilation paths based on CPU features */
    if (__builtin_cpu_supports("avx")) {
        /* This path may trigger different cache detection for AVX-capable CPUs */
        __attribute__((aligned(64))) static int avx_arr[1024 * 1024]; /* 4MB */
        
        for (int i = 0; i < 1024 * 1024; i++) {
            avx_arr[i] = i * 3;
        }
        
        int sum = 0;
        for (int i = 0; i < 1024 * 1024; i += 8) {
            sum += avx_arr[i];
        }
        checksum += sum;
        
        /* Call AVX-targeted computation */
        sandybridge_computation(&checksum);
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        __attribute__((aligned(16))) static short sse_arr[512 * 1024]; /* 1MB */
        
        for (int i = 0; i < 512 * 1024; i++) {
            sse_arr[i] = (i & 0xFF) - 128;
        }
        
        int sum = 0;
        for (int i = 0; i < 512 * 1024; i += 4) {
            sum += sse_arr[i];
        }
        checksum += sum;
    }
    
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path - most x86_64 CPUs have this */
        nehalem_computation(&checksum);
    }
    
    /* Always execute these */
    core2_computation(&checksum);
    generic_computation(&checksum);
    
    /* Additional CPUID leaf 4 (deterministic cache parameters) */
    /* This provides detailed cache info and may influence driver */
    uint32_t eax, ebx, ecx, edx;
    for (int i = 0; i < 4; i++) {
        cpuid(4, i, &eax, &ebx, &ecx, &edx);
        checksum += (eax & 0xFF) + (ebx & 0xFF) + (ecx & 0xFF) + (edx & 0xFF);
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
