/* driver-i386-cache-test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in GCC's driver-i386.cc, specifically targeting the switch cases
 * for cache descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * 
 * Compilation strategies:
 * 1. gcc -O2 -march=core2 -fverbose-asm -c driver-i386-cache-test.c -o test_core2.o
 * 2. gcc -O2 -march=nehalem -fverbose-asm -c driver-i386-cache-test.c -o test_nehalem.o
 * 3. gcc -O3 -march=native -mtune=generic -fdump-rtl-all -c driver-i386-cache-test.c
 * 4. gcc -O1 -m32 -march=pentium4 -fno-inline -c driver-i386-cache-test.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Helper function with target-specific attributes to force driver
 * to consider different cache configurations */
__attribute__((target("arch=core2")))
void core2_optimized_loop(int* data, int size) {
    /* Large stride access pattern */
    for (int i = 0; i < size; i += 16) {
        data[i] = data[i] * 3 + 7;
    }
}

__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, int size) {
    /* Sequential access with prefetch hints */
    for (int i = 0; i < size; i += 8) {
        __builtin_prefetch(&data[i + 32], 0, 3);
        data[i] = data[i] * 5 - 2;
    }
}

__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, int size) {
    /* Matrix-style access pattern */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            data[i * 64 + j] = data[j * 64 + i] + i - j;
        }
    }
}

/* Function to read CPUID cache descriptors directly */
static void read_cache_descriptors(uint32_t* descriptors, int max_desc) {
    uint32_t eax, ebx, ecx, edx;
    int desc_count = 0;
    
    /* CPUID leaf 2 - Cache descriptors (traditional method) */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2));
    
    /* Extract cache descriptor bytes from eax, ebx, ecx, edx */
    uint8_t* regs = (uint8_t*)&eax;
    for (int i = 0; i < 4 && desc_count < max_desc; i++) {
        if (regs[i] & 0x80) continue; /* Skip invalid descriptors */
        if (regs[i] != 0) descriptors[desc_count++] = regs[i];
    }
    
    regs = (uint8_t*)&ebx;
    for (int i = 0; i < 4 && desc_count < max_desc; i++) {
        if (regs[i] & 0x80) continue;
        if (regs[i] != 0) descriptors[desc_count++] = regs[i];
    }
    
    regs = (uint8_t*)&ecx;
    for (int i = 0; i < 4 && desc_count < max_desc; i++) {
        if (regs[i] & 0x80) continue;
        if (regs[i] != 0) descriptors[desc_count++] = regs[i];
    }
    
    regs = (uint8_t*)&edx;
    for (int i = 0; i < 4 && desc_count < max_desc; i++) {
        if (regs[i] & 0x80) continue;
        if (regs[i] != 0) descriptors[desc_count++] = regs[i];
    }
    
    /* Also try CPUID leaf 4 for deterministic cache parameters */
    for (int i = 0; i < 8 && desc_count < max_desc; i++) {
        asm volatile ("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(4), "c"(i));
        
        if ((eax & 0x1F) == 0) break; /* No more cache levels */
        
        /* Encode cache parameters into a descriptor-like value */
        uint32_t desc = 0;
        desc |= ((ecx >> 22) & 0x7FF) << 16; /* Line size */
        desc |= ((ebx >> 22) & 0x3FF) << 8;  /* Associativity */
        desc |= (eax >> 5) & 0x7FF;          /* Cache size */
        
        if (desc != 0) descriptors[desc_count++] = desc;
    }
}

/* Large array operations with different access patterns */
static void cache_sensitive_operations(void) {
    /* Aligned large arrays to exercise cache logic */
    __attribute__((aligned(64))) static int array1[1024 * 1024];  /* 4MB */
    __attribute__((aligned(64))) static int array2[512 * 512];    /* 1MB */
    __attribute__((aligned(64))) static int array3[256 * 256];    /* 256KB */
    
    /* Initialize with pseudo-random data using LCG */
    uint32_t seed = 123456789;
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed % 1000);
    }
    
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed % 1000);
    }
    
    /* Different access patterns to hint at cache usage */
    
    /* Pattern 1: Sequential access (good for prefetch) */
    int sum1 = 0;
    for (int i = 0; i < 1024 * 1024; i += 64) {
        sum1 += array1[i];
        __builtin_prefetch(&array1[i + 128], 0, 1);
    }
    
    /* Pattern 2: Strided access (cache line sized) */
    int sum2 = 0;
    for (int i = 0; i < 512 * 512; i += 16) {
        sum2 += array2[i];
    }
    
    /* Pattern 3: Random-ish access (poor locality) */
    int sum3 = 0;
    for (int i = 0; i < 256 * 256; i++) {
        int idx = (i * 97) % (256 * 256);  /* Pseudo-random index */
        sum3 += array3[idx];
    }
    
    /* Use results to prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3));
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t cache_descriptors[32] = {0};
    int checksum = 0;
    
    /* Read cache descriptors - forces driver to handle CPUID */
    read_cache_descriptors(cache_descriptors, 32);
    
    /* Use descriptors in computation */
    for (int i = 0; i < 32 && cache_descriptors[i] != 0; i++) {
        checksum += (int)cache_descriptors[i];
    }
    
    /* Conditional compilation paths based on CPU features */
    /* Each path may cause driver to evaluate different cache configurations */
    
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        __attribute__((aligned(16))) float sse_array[1024];
        for (int i = 0; i < 1024; i++) {
            sse_array[i] = i * 0.5f;
        }
        /* Dummy SSE operations */
        for (int i = 0; i < 1024 - 4; i += 4) {
            sse_array[i] = sse_array[i] + sse_array[i + 1];
        }
        checksum += (int)sse_array[0];
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        __attribute__((aligned(32))) double avx_array[2048];
        for (int i = 0; i < 2048; i++) {
            avx_array[i] = i * 0.25;
        }
        /* Large stride to exercise cache */
        for (int i = 0; i < 2048; i += 32) {
            avx_array[i] = avx_array[i] * 2.0;
        }
        checksum += (int)avx_array[0];
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        __attribute__((aligned(64))) int avx2_array[4096];
        for (int i = 0; i < 4096; i++) {
            avx2_array[i] = i * 3;
        }
        /* Gather-like access pattern */
        for (int i = 0; i < 4096; i += 8) {
            avx2_array[i] = avx2_array[i * 2 % 4096];
        }
        checksum += avx2_array[0];
    }
#endif
    
    /* Call target-specific functions */
    __attribute__((aligned(64))) int data[8192];
    for (int i = 0; i < 8192; i++) data[i] = i;
    
    /* These calls may cause driver to analyze cache for different arch targets */
    core2_optimized_loop(data, 8192);
    nehalem_optimized_loop(data, 8192);
    sandybridge_optimized_loop(data, 8192);
    
    /* Perform cache-sensitive operations */
    cache_sensitive_operations();
    
    /* Final checksum to prevent optimization */
    for (int i = 0; i < 8192; i += 128) {
        checksum += data[i];
    }
    
    /* Print result to ensure side effects */
    printf("Cache test checksum: %d\n", checksum);
    
    return 0;
}
