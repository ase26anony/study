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
    /* Large array access with stride patterns */
    int temp = 0;
    for (size_t i = 0; i < size; i += 8) {
        temp += data[i];
        /* Prefetch hint for next cache line */
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
    *checksum += temp;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Different access pattern */
    int temp = 0;
    for (size_t i = 0; i < size; i += 16) {
        temp += data[i] - data[i + 4];
        __builtin_prefetch(&data[i + 128], 0, 2);
    }
    *checksum += temp;
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int* data, size_t size, uint32_t* checksum) {
    /* Matrix-style access pattern */
    int temp = 0;
    const size_t stride = 32;
    for (size_t i = 0; i < size; i += stride) {
        for (size_t j = 0; j < 8 && (i + j) < size; j++) {
            temp += data[i + j] * j;
        }
        __builtin_prefetch(&data[i + 256], 0, 1);
    }
    *checksum += temp;
}

/* Function to read CPUID cache descriptors */
static void read_cpuid_cache_info(uint32_t* descriptors, int max_descriptors) {
    uint32_t eax, ebx, ecx, edx;
    int desc_count = 0;
    
    /* CPUID leaf 2 - Cache and TLB Descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract cache descriptors from registers */
    uint8_t* regs = (uint8_t*)&eax;
    for (int i = 0; i < 4 && desc_count < max_descriptors; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[desc_count++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ebx;
    for (int i = 0; i < 4 && desc_count < max_descriptors; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[desc_count++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&ecx;
    for (int i = 0; i < 4 && desc_count < max_descriptors; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[desc_count++] = regs[i];
        }
    }
    
    regs = (uint8_t*)&edx;
    for (int i = 0; i < 4 && desc_count < max_descriptors; i++) {
        if (regs[i] != 0 && regs[i] != 0xFF) {
            descriptors[desc_count++] = regs[i];
        }
    }
    
    /* Also read deterministic cache parameters (leaf 4) */
    for (int level = 0; level < 3 && desc_count < max_descriptors; level++) {
        ecx = level;
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(ecx)
        );
        
        if ((eax & 0x1F) != 0) { /* Cache type field not null */
            descriptors[desc_count++] = (eax & 0xFF); /* Cache type/level */
            descriptors[desc_count++] = ((eax >> 5) & 0x7FF) + 1; /* Ways */
            descriptors[desc_count++] = ((ebx >> 22) & 0x3FF) + 1; /* Lines */
            descriptors[desc_count++] = (ecx & 0xFFF) + 1; /* Partitions */
        }
    }
}

int main(void) {
    /* Initialize CPU feature detection */
    __builtin_cpu_init();
    
    uint32_t checksum = 0;
    uint32_t lcg_state = 42;
    
    /* Large aligned arrays to hint cache usage */
#define ARRAY_SIZE (2 * 1024 * 1024) /* 2MB */
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
        data1[i] = (int)lcg(&lcg_state);
        data2[i] = (int)lcg(&lcg_state);
    }
    
    /* Conditional compilation based on CPU features */
#ifdef __SSE4_2__
    if (__builtin_cpu_supports("sse4.2")) {
        /* SSE4.2 optimized path */
        for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
            checksum += data1[i] ^ data2[i];
        }
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        /* AVX optimized path with larger stride */
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            checksum += data1[i] | data2[i];
            __builtin_prefetch(&data1[i + 64], 0, 3);
        }
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path with different access pattern */
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            checksum += data1[i] & data2[i + 8];
            __builtin_prefetch(&data2[i + 128], 0, 2);
        }
    }
#endif
    
    /* Always execute SSE2 path */
    if (__builtin_cpu_supports("sse2")) {
        for (size_t i = 0; i < ARRAY_SIZE; i += 2) {
            checksum += data1[i] * data2[i];
        }
    }
    
    /* Call architecture-specific functions */
    core2_optimized_compute(data1, ARRAY_SIZE / 2, &checksum);
    nehalem_optimized_compute(data2, ARRAY_SIZE / 2, &checksum);
    sandybridge_optimized_compute(data1, ARRAY_SIZE / 4, &checksum);
    
    /* Read CPUID cache descriptors */
    uint32_t cache_descriptors[32] = {0};
    read_cpuid_cache_info(cache_descriptors, 32);
    
    /* Use descriptors in checksum to prevent elimination */
    for (int i = 0; i < 32 && cache_descriptors[i] != 0; i++) {
        checksum += cache_descriptors[i];
    }
    
    /* Matrix multiplication-like pattern for cache pressure */
    const size_t N = 256;
    int matrix_a[N][N] __attribute__((aligned(64)));
    int matrix_b[N][N] __attribute__((aligned(64)));
    int matrix_c[N][N] __attribute__((aligned(64)));
    
    /* Initialize matrices */
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            matrix_a[i][j] = (int)lcg(&lcg_state) % 100;
            matrix_b[i][j] = (int)lcg(&lcg_state) % 100;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Perform matrix multiplication with cache-aware blocking */
    const size_t BLOCK = 16; /* Block size for cache optimization */
    for (size_t i0 = 0; i0 < N; i0 += BLOCK) {
        for (size_t j0 = 0; j0 < N; j0 += BLOCK) {
            for (size_t k0 = 0; k0 < N; k0 += BLOCK) {
                for (size_t i = i0; i < i0 + BLOCK && i < N; i++) {
                    for (size_t j = j0; j < j0 + BLOCK && j < N; j++) {
                        int sum = 0;
                        for (size_t k = k0; k < k0 + BLOCK && k < N; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                            /* Prefetch next elements */
                            if (k % 8 == 0) {
                                __builtin_prefetch(&matrix_a[i][k + 8], 0, 1);
                                __builtin_prefetch(&matrix_b[k + 8][j], 0, 1);
                            }
                        }
                        matrix_c[i][j] += sum;
                    }
                }
            }
        }
    }
    
    /* Add matrix result to checksum */
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            checksum += matrix_c[i][j];
        }
    }
    
    /* Prevent dead code elimination */
    __asm__ volatile("" : : "r"(checksum));
    
    /* Print result to ensure side effects */
    printf("Checksum: %u\n", checksum);
    
    free(data1);
    free(data2);
    
    return 0;
}
