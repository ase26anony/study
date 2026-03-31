/* driver_cache_test.c
 * 
 * This program is designed to trigger CPU cache detection logic in GCC's
 * driver-i386.cc by exercising various CPUID-based cache descriptor paths.
 * It uses __builtin_cpu_init/supports, inline assembly with CPUID,
 * target attributes, and large array access patterns to cover the
 * switch cases for cache descriptor bytes 0x0a through 0x87.
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

/* Fill array with pseudo-random data */
static void fill_array(int *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int)(lcg_rand() % 256);
    }
}

/* Compute simple checksum to prevent dead code elimination */
static int compute_checksum(const int *arr, size_t n) {
    int sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum = (sum * 31 + arr[i]) & 0x7FFFFFFF;
    }
    return sum;
}

/* Matrix multiplication with different strides to hint at cache usage */
static void matrix_multiply(const int *A, const int *B, int *C, 
                           int size, int stride) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k += stride) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C[i * size + j] = sum;
        }
    }
}

/* Inline assembly to read CPUID leaf 2 (cache descriptors) */
static void cpuid_leaf2(uint32_t *eax, uint32_t *ebx, 
                       uint32_t *ecx, uint32_t *edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Inline assembly to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_leaf4(uint32_t leaf, uint32_t *eax, uint32_t *ebx,
                       uint32_t *ecx, uint32_t *edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

/* Functions with target attributes for different architectures */
#ifdef __GNUC__
/* Core2 target - may trigger cache descriptor 0x66, 0x67, etc. */
__attribute__((target("arch=core2")))
static int core2_optimized_compute(void) {
    /* Large aligned arrays */
    __attribute__((aligned(64))) int arr1[1024 * 16];
    __attribute__((aligned(64))) int arr2[1024 * 16];
    
    fill_array(arr1, 1024 * 16);
    fill_array(arr2, 1024 * 16);
    
    /* Matrix multiply with stride 2 */
    __attribute__((aligned(64))) int result[256 * 256];
    matrix_multiply(arr1, arr2, result, 256, 2);
    
    return compute_checksum(result, 256 * 256);
}

/* Nehalem target - may trigger cache descriptor 0x2c, 0x21, etc. */
__attribute__((target("arch=nehalem")))
static int nehalem_optimized_compute(void) {
    __attribute__((aligned(64))) int arr[1024 * 64];  /* 256KB */
    fill_array(arr, 1024 * 64);
    
    /* Random-ish access pattern */
    int sum = 0;
    for (int i = 0; i < 1000000; i++) {
        int idx = lcg_rand() % (1024 * 64);
        sum += arr[idx];
        /* Prefetch hint */
        __builtin_prefetch(&arr[(idx + 64) % (1024 * 64)], 0, 3);
    }
    
    return sum & 0x7FFFFFFF;
}

/* Sandybridge target - may trigger cache descriptor 0x3a, 0x3b, etc. */
__attribute__((target("arch=sandybridge")))
static int sandybridge_optimized_compute(void) {
    /* Very large array to potentially trigger L2 cache cases */
    __attribute__((aligned(64))) int big_arr[1024 * 512];  /* 2MB */
    fill_array(big_arr, 1024 * 512);
    
    /* Sequential and strided access */
    int checksum = 0;
    for (int stride = 1; stride <= 64; stride *= 2) {
        for (int i = 0; i < 1024 * 512; i += stride) {
            checksum = (checksum * 31 + big_arr[i]) & 0x7FFFFFFF;
        }
    }
    
    return checksum;
}

/* Skylake target - may trigger cache descriptor 0x55, 0x56 (not in list) */
__attribute__((target("arch=skylake")))
static int skylake_optimized_compute(void) {
    __attribute__((aligned(64))) int arr1[1024 * 32];
    __attribute__((aligned(64))) int arr2[1024 * 32];
    
    fill_array(arr1, 1024 * 32);
    fill_array(arr2, 1024 * 32);
    
    /* Histogram computation */
    int histogram[256] = {0};
    for (int i = 0; i < 1024 * 32; i++) {
        int val = (arr1[i] ^ arr2[i]) & 0xFF;
        histogram[val]++;
    }
    
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += histogram[i];
    }
    
    return sum;
}
#endif

/* Main function with CPU detection and multiple code paths */
int main(void) {
    int total_checksum = 0;
    
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Collect cache descriptor information via CPUID leaf 2 */
    uint32_t eax2, ebx2, ecx2, edx2;
    cpuid_leaf2(&eax2, &ebx2, &ecx2, &edx2);
    
    /* Use the cache descriptor bytes in trivial ways */
    total_checksum += (eax2 & 0xFF) + ((ebx2 >> 8) & 0xFF) + 
                     ((ecx2 >> 16) & 0xFF) + ((edx2 >> 24) & 0xFF);
    
    /* Read deterministic cache parameters (leaf 4) for all cache levels */
    for (uint32_t leaf = 0; leaf < 16; leaf++) {
        uint32_t eax4, ebx4, ecx4, edx4;
        cpuid_leaf4(leaf, &eax4, &ebx4, &ecx4, &edx4);
        
        /* Only use if this is a valid cache level */
        if ((eax4 & 0x1F) != 0) {
            total_checksum += (eax4 & 0xFF) + (ebx4 & 0xFF) + 
                             (ecx4 & 0xFF) + (edx4 & 0xFF);
        }
    }
    
    /* Conditional code paths based on CPU support */
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path - may trigger older cache descriptors */
        __attribute__((aligned(16))) int sse_arr[1024 * 8];
        fill_array(sse_arr, 1024 * 8);
        
        /* Simple vector-like computation */
        for (int i = 0; i < 1024 * 8 - 4; i += 4) {
            sse_arr[i] = sse_arr[i] + sse_arr[i+1];
            sse_arr[i+1] = sse_arr[i+2] * sse_arr[i+3];
        }
        
        total_checksum += compute_checksum(sse_arr, 1024 * 8);
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        __attribute__((aligned(16))) int sse42_arr[1024 * 32];
        fill_array(sse42_arr, 1024 * 32);
        
        /* More intensive computation */
        for (int i = 0; i < 1024 * 32 - 8; i += 8) {
            for (int j = 0; j < 8; j++) {
                sse42_arr[i+j] = (sse42_arr[i+j] * 1103515245 + 12345) & 0x7FFFFFFF;
            }
        }
        
        total_checksum += compute_checksum(sse42_arr, 1024 * 32);
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* AVX path - may trigger newer cache descriptors */
        __attribute__((aligned(32))) int avx_arr[1024 * 64];
        fill_array(avx_arr, 1024 * 64);
        
        /* Strided access pattern */
        for (int stride = 1; stride <= 32; stride *= 2) {
            for (int i = 0; i < 1024 * 64; i += stride) {
                avx_arr[i] = avx_arr[i] ^ 0xAAAAAAAA;
            }
        }
        
        total_checksum += compute_checksum(avx_arr, 1024 * 64);
    }
    
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path */
        __attribute__((aligned(32))) int avx2_arr[1024 * 128];
        fill_array(avx2_arr, 1024 * 128);
        
        /* Matrix transpose simulation */
        for (int i = 0; i < 1024; i++) {
            for (int j = i+1; j < 1024; j++) {
                int tmp = avx2_arr[i * 128 + j];
                avx2_arr[i * 128 + j] = avx2_arr[j * 128 + i];
                avx2_arr[j * 128 + i] = tmp;
            }
        }
        
        total_checksum += compute_checksum(avx2_arr, 1024 * 128);
    }
    
#ifdef __GNUC__
    /* Call architecture-specific functions */
    total_checksum += core2_optimized_compute();
    total_checksum += nehalem_optimized_compute();
    total_checksum += sandybridge_optimized_compute();
    total_checksum += skylake_optimized_compute();
#endif
    
    /* Final array operation with prefetch hints */
    __attribute__((aligned(64))) int final_arr[1024 * 256];  /* 1MB */
    fill_array(final_arr, 1024 * 256);
    
    /* Mixed access patterns */
    for (int i = 0; i < 1024 * 256; i += 64) {
        __builtin_prefetch(&final_arr[i + 128], 0, 1);
        final_arr[i] = final_arr[i] * 3 + 7;
    }
    
    total_checksum += compute_checksum(final_arr, 1024 * 256);
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(total_checksum));
    
    /* Print result to avoid complete optimization */
    printf("Checksum: %d\n", total_checksum & 0xFF);
    
    return (total_checksum & 0xFF) == 0 ? 0 : 1;
}
