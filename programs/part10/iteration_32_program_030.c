/* driver_cache_test.c
 * 
 * This program is designed to trigger CPU cache detection logic in GCC's
 * driver-i386.cc, specifically targeting the switch cases for cache
 * descriptor bytes (0x0a, 0x0c, 0x0d, etc.).
 * 
 * Compilation strategies:
 * 1. gcc -O2 -march=core2 -fverbose-asm -c driver_cache_test.c -o test_core2.o
 * 2. gcc -O2 -march=nehalem -fverbose-asm -c driver_cache_test.c -o test_nehalem.o
 * 3. gcc -O3 -march=native -mtune=generic -c driver_cache_test.c
 * 4. gcc -O1 -m32 -march=pentium4 -fno-inline -c driver_cache_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Helper functions with different target attributes to force driver
 * to consider multiple cache configurations */
#ifdef __SSE2__
__attribute__((target("arch=core2")))
static void core2_optimized_loop(int* data, int size) {
    /* Core2-specific optimization path */
    int sum = 0;
    for (int i = 0; i < size; i += 8) {
        sum += data[i];
        __builtin_prefetch(&data[i + 32], 0, 3); /* Medium locality */
    }
    __asm__ volatile("" : : "r"(sum)); /* Prevent elimination */
}

__attribute__((target("arch=nehalem")))
static void nehalem_optimized_loop(int* data, int size) {
    /* Nehalem-specific optimization path */
    int sum = 0;
    for (int i = 0; i < size; i += 16) {
        sum += data[i];
        __builtin_prefetch(&data[i + 64], 0, 1); /* High locality */
    }
    __asm__ volatile("" : : "r"(sum));
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_optimized_loop(int* data, int size) {
    /* Sandy Bridge-specific optimization path */
    int sum = 0;
    for (int i = 0; i < size; i += 32) {
        sum += data[i];
        __builtin_prefetch(&data[i + 128], 0, 2); /* Low locality */
    }
    __asm__ volatile("" : : "r"(sum));
}
#endif

/* Function to read CPUID cache descriptors directly */
static uint32_t read_cpuid_cache_descriptors() {
    uint32_t eax, ebx, ecx, edx;
    uint32_t descriptor_sum = 0;
    
    /* CPUID leaf 2 - Cache descriptors (may return multiple calls) */
    for (int i = 0; i < 3; i++) {
        __asm__ volatile (
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(2)
        );
        
        /* Sum all descriptor bytes to prevent elimination */
        descriptor_sum += (eax & 0xFF) + ((eax >> 8) & 0xFF) + 
                         ((eax >> 16) & 0xFF) + ((eax >> 24) & 0xFF);
        descriptor_sum += (ebx & 0xFF) + ((ebx >> 8) & 0xFF) + 
                         ((ebx >> 16) & 0xFF) + ((ebx >> 24) & 0xFF);
        descriptor_sum += (ecx & 0xFF) + ((ecx >> 8) & 0xFF) + 
                         ((ecx >> 16) & 0xFF) + ((ecx >> 24) & 0xFF);
        descriptor_sum += (edx & 0xFF) + ((edx >> 8) & 0xFF) + 
                         ((edx >> 16) & 0xFF) + ((edx >> 24) & 0xFF);
    }
    
    return descriptor_sum;
}

/* Function to read deterministic cache parameters */
static uint32_t read_deterministic_cache_params() {
    uint32_t eax, ebx, ecx, edx;
    uint32_t cache_info = 0;
    int level = 0;
    
    /* CPUID leaf 4 - Deterministic Cache Parameters */
    do {
        __asm__ volatile (
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(level)
        );
        
        cache_info += eax + ebx + ecx + edx;
        level++;
    } while ((eax & 0x1F) != 0); /* Continue until cache type = 0 */
    
    return cache_info;
}

/* Large array operations with different access patterns */
static void cache_sensitive_operations() {
    /* Different sized arrays to potentially trigger different cache behaviors */
    #define LARGE_SIZE (1024 * 1024)  /* 1MB */
    #define MEDIUM_SIZE (256 * 1024)  /* 256KB */
    #define SMALL_SIZE (32 * 1024)    /* 32KB */
    
    /* Aligned arrays to ensure cache line alignment */
    static int large_array[LARGE_SIZE] __attribute__((aligned(64)));
    static int medium_array[MEDIUM_SIZE] __attribute__((aligned(64)));
    static int small_array[SMALL_SIZE] __attribute__((aligned(64)));
    
    /* Initialize with pseudo-random data using LCG */
    uint32_t seed = 123456789;
    for (int i = 0; i < LARGE_SIZE; i++) {
        seed = (1103515245 * seed + 12345) & 0x7FFFFFFF;
        large_array[i] = (int)seed;
    }
    
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        seed = (1103515245 * seed + 12345) & 0x7FFFFFFF;
        medium_array[i] = (int)seed;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        seed = (1103515245 * seed + 12345) & 0x7FFFFFFF;
        small_array[i] = (int)seed;
    }
    
    /* Pattern 1: Sequential access (good for prefetching) */
    int sum1 = 0;
    for (int i = 0; i < LARGE_SIZE; i += 1) {
        sum1 += large_array[i];
    }
    
    /* Pattern 2: Strided access (cache line sized) */
    int sum2 = 0;
    for (int i = 0; i < MEDIUM_SIZE; i += 16) { /* 64 bytes / 4 bytes per int */
        sum2 += medium_array[i];
        __builtin_prefetch(&medium_array[i + 64], 0, 3);
    }
    
    /* Pattern 3: Random-ish access pattern */
    int sum3 = 0;
    for (int i = 0; i < SMALL_SIZE; i++) {
        int idx = (i * 97) % SMALL_SIZE; /* Pseudo-random index */
        sum3 += small_array[idx];
        if (i % 8 == 0) {
            __builtin_prefetch(&small_array[(idx + 32) % SMALL_SIZE], 0, 1);
        }
    }
    
    /* Use results to prevent elimination */
    __asm__ volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3));
    
    #ifdef __SSE2__
    /* Call architecture-specific functions */
    core2_optimized_loop(large_array, LARGE_SIZE / 4);
    nehalem_optimized_loop(medium_array, MEDIUM_SIZE / 4);
    sandybridge_optimized_loop(small_array, SMALL_SIZE / 4);
    #endif
}

int main() {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    
    /* Read CPUID information - this may influence driver's cache detection */
    checksum += read_cpuid_cache_descriptors();
    checksum += read_deterministic_cache_params();
    
    /* Conditional compilation paths based on CPU features */
    #ifdef __OPTIMIZE__
    if (__builtin_cpu_supports("avx")) {
        /* AVX path - may trigger different cache assumptions */
        checksum += 0xAVX;
        cache_sensitive_operations();
    } else if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        checksum += 0xSSE42;
        cache_sensitive_operations();
    } else if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path */
        checksum += 0xSSE2;
        cache_sensitive_operations();
    }
    #endif
    
    /* Additional conditional paths for different optimization levels */
    #if defined(__SSE4_2__)
    {
        int temp_array[1024] __attribute__((aligned(64)));
        for (int i = 0; i < 1024; i++) {
            temp_array[i] = i;
        }
        int sse42_sum = 0;
        for (int i = 0; i < 1024; i += 4) {
            sse42_sum += temp_array[i];
        }
        checksum += sse42_sum;
    }
    #endif
    
    #if defined(__AVX__)
    {
        int avx_array[2048] __attribute__((aligned(64)));
        for (int i = 0; i < 2048; i++) {
            avx_array[i] = i * 2;
        }
        int avx_sum = 0;
        for (int i = 0; i < 2048; i += 8) {
            avx_sum += avx_array[i];
        }
        checksum += avx_sum;
    }
    #endif
    
    /* Matrix multiplication pattern - cache intensive */
    #define MATRIX_SIZE 256
    static int matrix_a[MATRIX_SIZE][MATRIX_SIZE] __attribute__((aligned(64)));
    static int matrix_b[MATRIX_SIZE][MATRIX_SIZE] __attribute__((aligned(64)));
    static int matrix_c[MATRIX_SIZE][MATRIX_SIZE] __attribute__((aligned(64)));
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = i + j;
            matrix_b[i][j] = i - j;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Blocked matrix multiplication for better cache utilization */
    int block_size = 32;
    for (int ii = 0; ii < MATRIX_SIZE; ii += block_size) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += block_size) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += block_size) {
                for (int i = ii; i < ii + block_size && i < MATRIX_SIZE; i++) {
                    for (int j = jj; j < jj + block_size && j < MATRIX_SIZE; j++) {
                        int sum = 0;
                        for (int k = kk; k < kk + block_size && k < MATRIX_SIZE; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                        }
                        matrix_c[i][j] += sum;
                    }
                }
            }
        }
    }
    
    /* Compute final checksum from matrix result */
    int matrix_sum = 0;
    for (int i = 0; i < MATRIX_SIZE; i += 8) {
        for (int j = 0; j < MATRIX_SIZE; j += 8) {
            matrix_sum += matrix_c[i][j];
        }
    }
    checksum += matrix_sum;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
