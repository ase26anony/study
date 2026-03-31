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
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Helper function with Core2 target attribute */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int *arr, size_t size) {
    /* Sequential access pattern */
    for (size_t i = 0; i < size; i += 64/sizeof(int)) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Some stride-2 access */
    for (size_t i = 0; i < size/2; i++) {
        arr[i*2] += arr[i];
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int *arr, size_t size) {
    /* More complex access pattern */
    for (size_t i = 0; i < size; i += 128/sizeof(int)) {
        arr[i] = arr[i] ^ 0x55555555;
    }
    /* Reverse loop */
    for (size_t i = size-1; i > 0; i -= 3) {
        arr[i] = arr[i-1] + arr[i-2];
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int *arr, size_t size) {
    /* Blocked access pattern */
    const size_t block = 256/sizeof(int);
    for (size_t i = 0; i < size; i += block) {
        size_t end = (i + block < size) ? i + block : size;
        for (size_t j = i; j < end; j++) {
            arr[j] = arr[j] * 2 - 1;
        }
    }
}

/* Function that uses inline CPUID to read cache descriptors */
static uint32_t read_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t checksum = 0;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple calls) */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2));
    
    checksum += eax + ebx + ecx + edx;
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 4; i++) { /* Check up to 4 cache levels */
        ecx = i;
        asm volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(4), "c"(i));
        
        if ((eax & 0x1F) == 0) /* No more caches */
            break;
            
        checksum += eax + ebx + ecx + edx;
    }
    
    return checksum;
}

/* Large array operations with different access patterns */
static void cache_size_hinting_operations(void) {
    /* Different sized arrays to hint at various cache levels */
    #define L1_SIZE (32 * 1024 / sizeof(int))  /* ~L1 size */
    #define L2_SIZE (256 * 1024 / sizeof(int)) /* ~L2 size */
    #define L3_SIZE (8192 * 1024 / sizeof(int)) /* ~L3 size */
    
    /* Aligned arrays */
    int __attribute__((aligned(64))) l1_array[L1_SIZE];
    int __attribute__((aligned(64))) l2_array[L2_SIZE];
    static int __attribute__((aligned(64))) l3_array[L3_SIZE];
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < L1_SIZE; i++) l1_array[i] = lcg_rand();
    for (size_t i = 0; i < L2_SIZE; i++) l2_array[i] = lcg_rand();
    for (size_t i = 0; i < L3_SIZE; i++) l3_array[i] = lcg_rand();
    
    /* Pattern 1: Sequential L1 access */
    for (size_t i = 0; i < L1_SIZE; i++) {
        l1_array[i] = l1_array[i] * 3 + 1;
    }
    
    /* Pattern 2: Strided L2 access (every 16th element) */
    for (size_t i = 0; i < L2_SIZE; i += 16) {
        l2_array[i] = l2_array[i] ^ l2_array[i+8];
    }
    
    /* Pattern 3: Random-ish L3 access using LCG */
    uint32_t idx = 0;
    for (size_t i = 0; i < L3_SIZE/4; i++) {
        idx = (idx * 1103515245 + 12345) % L3_SIZE;
        l3_array[idx] += i;
        
        /* Prefetch hint */
        if (i % 32 == 0) {
            __builtin_prefetch(&l3_array[(idx + 64) % L3_SIZE], 0, 3);
        }
    }
    
    /* Pattern 4: Matrix-style blocked access */
    const size_t block = 64;
    for (size_t i = 0; i < L2_SIZE; i += block) {
        for (size_t j = 0; j < block && (i+j) < L2_SIZE; j++) {
            l2_array[i+j] = l2_array[i+j] * l2_array[i+(j^1)];
        }
    }
}

/* Main function with conditional compilation paths */
int main(void) {
    uint64_t checksum = 0;
    
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Conditional code paths based on CPU features */
    #ifdef __SSE2__
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2-optimized path */
        float __attribute__((aligned(16))) sse_array[1024];
        for (int i = 0; i < 1024; i++) sse_array[i] = i * 0.5f;
        for (int i = 0; i < 1024-1; i++) sse_array[i] += sse_array[i+1];
        checksum += (uint64_t)sse_array[0];
    }
    #endif
    
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2-optimized path */
        int __attribute__((aligned(16))) sse42_array[2048];
        for (int i = 0; i < 2048; i++) sse42_array[i] = i ^ 0xAAAA;
        for (int i = 0; i < 2048-4; i += 4) {
            sse42_array[i] = __builtin_popcount(sse42_array[i+1]);
        }
        checksum += sse42_array[512];
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized path */
        double __attribute__((aligned(32))) avx_array[4096];
        for (int i = 0; i < 4096; i++) avx_array[i] = i * 0.25;
        for (int i = 0; i < 4096-8; i += 8) {
            avx_array[i] = avx_array[i+4] * avx_array[i+7];
        }
        checksum += (uint64_t)avx_array[1024];
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2-optimized path */
        long __attribute__((aligned(32))) avx2_array[8192];
        for (int i = 0; i < 8192; i++) avx2_array[i] = i * 3L;
        for (int i = 0; i < 8192-16; i += 16) {
            avx2_array[i] = avx2_array[i] << (avx2_array[i+8] & 7);
        }
        checksum += avx2_array[2048];
    }
    #endif
    
    /* Read CPUID cache information */
    checksum += read_cpuid_cache_info();
    
    /* Perform cache-hinting operations */
    cache_size_hinting_operations();
    
    /* Call architecture-specific functions */
    {
        int test_array[1024];
        for (int i = 0; i < 1024; i++) test_array[i] = i;
        
        #if defined(__i386__) || defined(__x86_64__)
        core2_optimized_loop(test_array, 1024);
        nehalem_optimized_loop(test_array, 1024);
        sandybridge_optimized_loop(test_array, 1024);
        #endif
        
        for (int i = 0; i < 1024; i++) {
            checksum += test_array[i];
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
