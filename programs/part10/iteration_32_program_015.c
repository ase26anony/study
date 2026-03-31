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
    /* Large array access pattern that might hint at L1/L2 cache usage */
    int temp = 0;
    for (size_t i = 0; i < size; i += 8) {
        temp += data[i];
        __builtin_prefetch(&data[i + 64], 0, 3); /* Medium locality hint */
    }
    *checksum += temp;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Different stride pattern */
    int temp = 0;
    for (size_t i = 0; i < size; i += 16) {
        temp += data[i];
        __builtin_prefetch(&data[i + 128], 0, 1); /* Low locality hint */
    }
    *checksum += temp;
}

/* Function with target attribute for Sandy Bridge architecture */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Yet another access pattern */
    int temp = 0;
    for (size_t i = 0; i < size; i += 32) {
        temp += data[i];
        __builtin_prefetch(&data[i + 256], 0, 2); /* High locality hint */
    }
    *checksum += temp;
}

/* Inline assembly to read CPUID leaf 2 (cache descriptors) */
static void cpuid_cache_descriptors(uint32_t* eax, uint32_t* ebx, 
                                   uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(2)
    );
}

/* Inline assembly to read CPUID leaf 4 (deterministic cache parameters) */
static void cpuid_deterministic_cache(uint32_t leaf, uint32_t* eax, 
                                     uint32_t* ebx, uint32_t* ecx, 
                                     uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(4), "c"(leaf)
    );
}

int main() {
    uint32_t checksum = 0;
    uint32_t lcg_state = 42;
    
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays to exercise cache detection */
    #define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB to exceed typical L1/L2 */
    int* data1 __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* data2 __attribute__((aligned(64))) = 
        (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = lcg(&lcg_state);
        data2[i] = lcg(&lcg_state);
    }
    
    /* Conditional compilation paths based on CPU features */
    #ifdef __OPTIMIZE__
    checksum += 1;
    #endif
    
    /* Different code paths based on CPU support */
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            checksum += data1[i] + data2[ARRAY_SIZE - i - 1];
        }
    } else if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
            checksum += data1[i] * data2[i];
        }
    } else if (__builtin_cpu_supports("sse2")) {
        /* SSE2 path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 2) {
            checksum += data1[i] - data2[i];
        }
    }
    
    /* Call architecture-specific functions */
    core2_optimized_compute(data1, ARRAY_SIZE, &checksum);
    nehalem_optimized_compute(data2, ARRAY_SIZE, &checksum);
    sandybridge_optimized_compute(data1, ARRAY_SIZE, &checksum);
    
    /* Execute CPUID instructions to generate cache descriptor handling */
    uint32_t eax, ebx, ecx, edx;
    
    /* Read cache descriptors (leaf 2) */
    cpuid_cache_descriptors(&eax, &ebx, &ecx, &edx);
    checksum += eax + ebx + ecx + edx;
    
    /* Read deterministic cache parameters for L1 data cache (leaf 4, ECX=0) */
    cpuid_deterministic_cache(0, &eax, &ebx, &ecx, &edx);
    checksum += eax ^ ebx ^ ecx ^ edx;
    
    /* Read deterministic cache parameters for L2 cache (leaf 4, ECX=1) */
    cpuid_deterministic_cache(1, &eax, &ebx, &ecx, &edx);
    checksum += (eax & 0xFF) + (ebx & 0xFF) + (ecx & 0xFF) + (edx & 0xFF);
    
    /* Matrix multiplication-like pattern to exercise cache */
    #define MATRIX_SIZE 512
    int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
    int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
    int matrix_c[MATRIX_SIZE][MATRIX_SIZE];
    
    /* Initialize matrices */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = lcg(&lcg_state) % 100;
            matrix_b[i][j] = lcg(&lcg_state) % 100;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Simple matrix multiplication with cache-unfriendly access */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            for (int k = 0; k < MATRIX_SIZE; k++) {
                matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
            }
            checksum += matrix_c[i][j];
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to avoid complete optimization */
    printf("Checksum: %u\n", checksum);
    
    free(data1);
    free(data2);
    
    return 0;
}
