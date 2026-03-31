/* driver_cache_test.c
 * 
 * This program is designed to trigger the CPU cache detection logic
 * in the GCC driver (driver-i386.cc lines 127-244) by using various
 * CPUID-based techniques, architecture-specific code paths, and
 * cache-aware programming patterns.
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
void core2_optimized_loop(int* data, size_t size) {
    /* Sequential access pattern */
    for (size_t i = 0; i < size; i += 64/sizeof(int)) {
        data[i] = data[i] * 3 + 7;
    }
    /* Some stride-2 access */
    for (size_t i = 0; i < size/2; i++) {
        data[i*2] += data[i*2 + 1];
    }
}

/* Helper function with Nehalem target attribute */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_loop(int* data, size_t size) {
    /* More complex access pattern */
    for (size_t i = 0; i < size; i += 32) {
        data[i] = data[i] ^ 0xAAAAAAAA;
    }
    /* Reverse access */
    for (size_t i = size-1; i > 0; i -= 16) {
        data[i] = data[i] * 2;
    }
}

/* Helper function with Sandybridge target attribute */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_loop(int* data, size_t size) {
    /* Blocked access pattern */
    const size_t block = 256;
    for (size_t i = 0; i < size; i += block) {
        size_t end = (i + block < size) ? i + block : size;
        for (size_t j = i; j < end; j++) {
            data[j] = data[j] + (j % 256);
        }
    }
}

/* Function to read CPUID leaf 2 (cache descriptors) */
static uint32_t read_cpuid_cache_descriptors(uint32_t* descriptors) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t count = 0;
    
    /* CPUID leaf 2 - Cache and TLB Descriptor */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Store the descriptors from eax, ebx, ecx, edx */
    descriptors[0] = eax;
    descriptors[1] = ebx;
    descriptors[2] = ecx;
    descriptors[3] = edx;
    
    /* Count non-zero bytes (cache descriptors) */
    uint8_t* bytes = (uint8_t*)descriptors;
    for (int i = 0; i < 16; i++) {
        if (bytes[i] && (bytes[i] & 0x80) == 0) { /* Valid cache descriptor */
            count++;
        }
    }
    
    return count;
}

/* Function to read deterministic cache parameters (CPUID leaf 4) */
static void read_deterministic_cache_params(int level) {
    uint32_t eax, ebx, ecx, edx;
    
    for (int i = 0; ; i++) {
        ecx = i; /* Sub-leaf index */
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(ecx)
        );
        
        /* Check if this cache level exists */
        if ((eax & 0x1F) == 0) {
            break;
        }
        
        /* Use the values to prevent optimization */
        volatile uint32_t dummy = eax + ebx + ecx + edx;
        (void)dummy;
    }
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint64_t checksum = 0;
    
    /* Large aligned arrays to suggest cache usage */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
    static int __attribute__((aligned(64))) large_array[ARRAY_SIZE];
    
    /* Fill array with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        large_array[i] = lcg_rand();
    }
    
    /* Read CPUID cache descriptors */
    uint32_t cache_descriptors[4];
    uint32_t desc_count = read_cpuid_cache_descriptors(cache_descriptors);
    checksum += desc_count;
    
    /* Read deterministic cache parameters */
    read_deterministic_cache_params(1); /* L1 */
    read_deterministic_cache_params(2); /* L2 */
    read_deterministic_cache_params(3); /* L3 */
    
    /* Conditional compilation based on CPU features */
    #ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
            large_array[i] = large_array[i] >> 1;
        }
        checksum += 0x1234;
    }
    #endif
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            large_array[i] = large_array[i] * 3;
        }
        checksum += 0x5678;
        
        /* Use prefetch hints */
        for (size_t i = 0; i < ARRAY_SIZE; i += 64) {
            __builtin_prefetch(&large_array[i + 128], 0, 3);
            large_array[i] = large_array[i] + 1;
        }
    }
    #endif
    
    #ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 optimized path with more complex pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            large_array[i] = large_array[i] ^ large_array[i + 8];
        }
        checksum += 0x9ABC;
    }
    #endif
    
    /* Call architecture-specific functions */
    core2_optimized_loop(large_array, ARRAY_SIZE / 4);
    nehalem_optimized_loop(large_array + ARRAY_SIZE/4, ARRAY_SIZE / 4);
    sandybridge_optimized_loop(large_array + ARRAY_SIZE/2, ARRAY_SIZE / 4);
    
    /* Matrix multiplication-like pattern (cache blocking) */
    #define MAT_SIZE 512
    static int __attribute__((aligned(64))) mat_a[MAT_SIZE][MAT_SIZE];
    static int __attribute__((aligned(64))) mat_b[MAT_SIZE][MAT_SIZE];
    static int __attribute__((aligned(64))) mat_c[MAT_SIZE][MAT_SIZE];
    
    /* Initialize matrices */
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            mat_a[i][j] = lcg_rand() % 100;
            mat_b[i][j] = lcg_rand() % 100;
            mat_c[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication (cache-aware) */
    const int BLOCK = 64; /* Try different block sizes */
    for (int i = 0; i < MAT_SIZE; i += BLOCK) {
        for (int j = 0; j < MAT_SIZE; j += BLOCK) {
            for (int k = 0; k < MAT_SIZE; k += BLOCK) {
                for (int ii = i; ii < i + BLOCK && ii < MAT_SIZE; ii++) {
                    for (int jj = j; jj < j + BLOCK && jj < MAT_SIZE; jj++) {
                        int sum = mat_c[ii][jj];
                        for (int kk = k; kk < k + BLOCK && kk < MAT_SIZE; kk++) {
                            sum += mat_a[ii][kk] * mat_b[kk][jj];
                        }
                        mat_c[ii][jj] = sum;
                    }
                }
            }
        }
    }
    
    /* Compute final checksum */
    for (size_t i = 0; i < ARRAY_SIZE; i += 97) { /* Prime stride */
        checksum += large_array[i];
    }
    
    for (int i = 0; i < MAT_SIZE; i += 73) {
        for (int j = 0; j < MAT_SIZE; j += 73) {
            checksum += mat_c[i][j];
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
