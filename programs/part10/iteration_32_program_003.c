/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by:
 * 1. Using __builtin_cpu_init() and __builtin_cpu_supports()
 * 2. Multiple compilation units with different -march flags
 * 3. Inline assembly with CPUID instruction
 * 4. Compiler pragmas and target attributes
 * 5. Large array access patterns with prefetch hints
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper function to generate pseudo-random data */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Large array access pattern that might hint at L1 cache usage */
    int temp[8192] __attribute__((aligned(64)));  /* 8KB aligned array */
    uint32_t state = 42;
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < sizeof(temp)/sizeof(temp[0]); i++) {
        temp[i] = lcg(&state) & 0xFF;
    }
    
    /* Access with stride patterns */
    for (size_t i = 0; i < sizeof(temp)/sizeof(temp[0]); i += 32) {
        *checksum += temp[i];
        __builtin_prefetch(&temp[i + 64], 0, 3);  /* High temporal locality */
    }
    
    /* Matrix-style access */
    for (size_t i = 0; i < 64; i++) {
        for (size_t j = 0; j < 64; j++) {
            *checksum += temp[i * 128 + j];
        }
    }
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Larger array to potentially trigger L2 cache detection */
    int temp[32768] __attribute__((aligned(64)));  /* 128KB aligned array */
    uint32_t state = 123;
    
    for (size_t i = 0; i < sizeof(temp)/sizeof(temp[0]); i++) {
        temp[i] = lcg(&state) & 0xFF;
    }
    
    /* Different access pattern */
    for (size_t i = 0; i < sizeof(temp)/sizeof(temp[0]); i += 64) {
        *checksum += temp[i];
        __builtin_prefetch(&temp[i + 128], 0, 2);  /* Medium temporal locality */
    }
    
    /* Reverse access pattern */
    for (size_t i = sizeof(temp)/sizeof(temp[0]) - 1; i > 0; i -= 97) {
        *checksum += temp[i];
    }
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Even larger array */
    int temp[131072] __attribute__((aligned(64)));  /* 512KB aligned array */
    uint32_t state = 456;
    
    for (size_t i = 0; i < sizeof(temp)/sizeof(temp[0]); i++) {
        temp[i] = lcg(&state) & 0xFF;
    }
    
    /* Random-ish access pattern using LCG */
    uint32_t idx_state = 789;
    for (int i = 0; i < 10000; i++) {
        uint32_t idx = lcg(&idx_state) % (sizeof(temp)/sizeof(temp[0]));
        *checksum += temp[idx];
        if (i % 32 == 0) {
            __builtin_prefetch(&temp[(idx + 256) % (sizeof(temp)/sizeof(temp[0]))], 0, 1);
        }
    }
}

/* Function to read CPUID cache descriptors */
static void read_cpuid_cache_info(uint32_t* checksum) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple calls) */
    asm volatile (
        "mov $2, %%eax\n\t"
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        :
        : 
    );
    
    *checksum += eax + ebx + ecx + edx;
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; i < 8; i++) {  /* Check up to 8 cache levels */
        asm volatile (
            "mov $4, %%eax\n\t"
            "mov %0, %%ecx\n\t"
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "r"(i)
            : 
        );
        
        if ((eax & 0x1F) == 0)  /* No more caches */
            break;
            
        *checksum += eax + ebx + ecx + edx;
    }
}

/* Main computation function with ISA-specific paths */
void perform_computations(uint32_t* checksum) {
    /* Large data array */
    static int data[1024 * 1024] __attribute__((aligned(64)));
    uint32_t state = 0xDEADBEEF;
    
    /* Initialize with pseudo-random data */
    for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
        data[i] = lcg(&state) & 0xFFF;
    }
    
    /* Use different code paths based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i += 4) {
            *checksum += data[i] + data[i+1] + data[i+2] + data[i+3];
        }
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path - different access pattern */
        for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i += 8) {
            *checksum += data[i] ^ data[i+4];
        }
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path - matrix transpose simulation */
        for (size_t i = 0; i < 1024; i++) {
            for (size_t j = 0; j < 1024; j++) {
                *checksum += data[i * 1024 + j] * data[j * 1024 + i];
            }
        }
    }
    #endif
    
    /* Call architecture-specific functions */
    core2_optimized_compute(data, sizeof(data)/sizeof(data[0]), checksum);
    nehalem_optimized_compute(data, sizeof(data)/sizeof(data[0]), checksum);
    sandybridge_optimized_compute(data, sizeof(data)/sizeof(data[0]), checksum);
}

int main(void) {
    uint32_t checksum = 0;
    
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    /* Read CPUID cache information */
    read_cpuid_cache_info(&checksum);
    
    /* Perform computations with different optimization paths */
    perform_computations(&checksum);
    
    /* Additional CPU feature checks to trigger driver logic */
    if (__builtin_cpu_supports("sse")) {
        checksum += 0x1000;
    }
    if (__builtin_cpu_supports("sse2")) {
        checksum += 0x2000;
    }
    if (__builtin_cpu_supports("sse3")) {
        checksum += 0x3000;
    }
    if (__builtin_cpu_supports("ssse3")) {
        checksum += 0x4000;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        checksum += 0x5000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 0x6000;
    }
    if (__builtin_cpu_supports("avx")) {
        checksum += 0x7000;
    }
    if (__builtin_cpu_supports("avx2")) {
        checksum += 0x8000;
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to ensure side effects */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
