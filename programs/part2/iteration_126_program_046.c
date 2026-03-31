/* 
 * cache_detection.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection.c -o cache_detection
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 cache_detection.c -o cache_detection
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Function declarations with architecture-specific optimizations */
__attribute__((optimize("-mtune=generic"))) void generic_cache_test(void);
__attribute__((optimize("-mtune=core2"))) void core2_cache_test(void);
__attribute__((optimize("-mtune=nehalem"))) void nehalem_cache_test(void);
__attribute__((optimize("-mtune=haswell"))) void haswell_cache_test(void);
__attribute__((optimize("-mtune=skylake"))) void skylake_cache_test(void);

/* Manual CPUID leaf 0x2 decoding (mirrors driver-i386.cc logic) */
__attribute__((noinline)) void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t *descriptors;
    int i, j;
    volatile int xeon_mp = 0; /* Simulated flag */
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
    uint8_t desc_bytes[16];
    desc_bytes[0] = (eax >> 0) & 0xFF;
    desc_bytes[1] = (eax >> 8) & 0xFF;
    desc_bytes[2] = (eax >> 16) & 0xFF;
    desc_bytes[3] = (eax >> 24) & 0xFF;
    desc_bytes[4] = (ebx >> 0) & 0xFF;
    desc_bytes[5] = (ebx >> 8) & 0xFF;
    desc_bytes[6] = (ebx >> 16) & 0xFF;
    desc_bytes[7] = (ebx >> 24) & 0xFF;
    desc_bytes[8] = (ecx >> 0) & 0xFF;
    desc_bytes[9] = (ecx >> 8) & 0xFF;
    desc_bytes[10] = (ecx >> 16) & 0xFF;
    desc_bytes[11] = (ecx >> 24) & 0xFF;
    desc_bytes[12] = (edx >> 0) & 0xFF;
    desc_bytes[13] = (edx >> 8) & 0xFF;
    desc_bytes[14] = (edx >> 16) & 0xFF;
    desc_bytes[15] = (edx >> 24) & 0xFF;
    
    /* Process each valid descriptor byte (0x00 is invalid, 0xFF is reserved) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = desc_bytes[i];
        if (desc == 0x00 || desc == 0xFF) continue;
        
        /* Mirror the exact switch cases from driver-i386.cc */
        switch (desc) {
            case 0x0a:
                l1_size_kb = 8; l1_assoc = 2; l1_line = 32;
                break;
            case 0x0c:
                l1_size_kb = 16; l1_assoc = 4; l1_line = 32;
                break;
            case 0x0d:
                l1_size_kb = 16; l1_assoc = 4; l1_line = 64;
                break;
            case 0x0e:
                l1_size_kb = 24; l1_assoc = 6; l1_line = 64;
                break;
            case 0x21:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 64;
                break;
            case 0x24:
                l2_size_kb = 1024; l2_assoc = 16; l2_line = 64;
                break;
            case 0x2c:
                l1_size_kb = 32; l1_assoc = 8; l1_line = 64;
                break;
            case 0x39:
                l2_size_kb = 128; l2_assoc = 4; l2_line = 64;
                break;
            case 0x3a:
                l2_size_kb = 192; l2_assoc = 6; l2_line = 64;
                break;
            case 0x3b:
                l2_size_kb = 128; l2_assoc = 2; l2_line = 64;
                break;
            case 0x3c:
                l2_size_kb = 256; l2_assoc = 4; l2_line = 64;
                break;
            case 0x3d:
                l2_size_kb = 384; l2_assoc = 6; l2_line = 64;
                break;
            case 0x3e:
                l2_size_kb = 512; l2_assoc = 4; l2_line = 64;
                break;
            case 0x41:
                l2_size_kb = 128; l2_assoc = 4; l2_line = 32;
                break;
            case 0x42:
                l2_size_kb = 256; l2_assoc = 4; l2_line = 32;
                break;
            case 0x43:
                l2_size_kb = 512; l2_assoc = 4; l2_line = 32;
                break;
            case 0x44:
                l2_size_kb = 1024; l2_assoc = 4; l2_line = 32;
                break;
            case 0x45:
                l2_size_kb = 2048; l2_assoc = 4; l2_line = 32;
                break;
            case 0x48:
                l2_size_kb = 3072; l2_assoc = 12; l2_line = 64;
                break;
            case 0x49:
                if (!xeon_mp) {
                    l2_size_kb = 4096; l2_assoc = 16; l2_line = 64;
                }
                break;
            case 0x4e:
                l2_size_kb = 6144; l2_assoc = 24; l2_line = 64;
                break;
            case 0x60:
                l1_size_kb = 16; l1_assoc = 8; l1_line = 64;
                break;
            case 0x66:
                l1_size_kb = 8; l1_assoc = 4; l1_line = 64;
                break;
            case 0x67:
                l1_size_kb = 16; l1_assoc = 4; l1_line = 64;
                break;
            case 0x68:
                l1_size_kb = 32; l1_assoc = 4; l1_line = 64;
                break;
            case 0x78:
                l2_size_kb = 1024; l2_assoc = 4; l2_line = 64;
                break;
            case 0x79:
                l2_size_kb = 128; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7a:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7b:
                l2_size_kb = 512; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7c:
                l2_size_kb = 1024; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7d:
                l2_size_kb = 2048; l2_assoc = 8; l2_line = 64;
                break;
            case 0x7f:
                l2_size_kb = 512; l2_assoc = 2; l2_line = 64;
                break;
            case 0x80:
                l2_size_kb = 512; l2_assoc = 8; l2_line = 64;
                break;
            case 0x82:
                l2_size_kb = 256; l2_assoc = 8; l2_line = 32;
                break;
            case 0x83:
                l2_size_kb = 512; l2_assoc = 8; l2_line = 32;
                break;
            case 0x84:
                l2_size_kb = 1024; l2_assoc = 8; l2_line = 32;
                break;
            case 0x85:
                l2_size_kb = 2048; l2_assoc = 8; l2_line = 32;
                break;
            case 0x86:
                l2_size_kb = 512; l2_assoc = 4; l2_line = 64;
                break;
            case 0x87:
                l2_size_kb = 1024; l2_assoc = 8; l2_line = 64;
                break;
            default:
                /* Other descriptors not in uncovered lines */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions */
__attribute__((noinline)) void benchmark_cache_size(unsigned int size_kb) {
    volatile unsigned long long sum = 0;
    unsigned int elements = (size_kb * 1024) / sizeof(int);
    int *array = (int*)malloc(elements * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (unsigned int i = 0; i < elements; i++) {
        array[i] = i;
    }
    
    /* Access pattern that stresses cache */
    for (unsigned int iter = 0; iter < 100; iter++) {
        for (unsigned int i = 0; i < elements; i += 64) {
            sum += array[i];
        }
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(sum));
    
    free(array);
}

/* Architecture-specific test functions */
__attribute__((optimize("-mtune=generic"))) 
void generic_cache_test(void) {
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    
    if (has_sse2) {
        benchmark_cache_size(8);   /* Potential L1 */
        benchmark_cache_size(256); /* Potential L2 */
    }
    
    decode_cpuid_cache_descriptors();
}

__attribute__((optimize("-mtune=core2"))) 
void core2_cache_test(void) {
    volatile int is_core2 = __builtin_cpu_is("core2");
    
    if (is_core2) {
        /* Core 2 typically has 32KB L1, up to 6MB L2 */
        benchmark_cache_size(32);
        benchmark_cache_size(2048);
        benchmark_cache_size(6144);
    }
    
    decode_cpuid_cache_descriptors();
}

__attribute__((optimize("-mtune=nehalem"))) 
void nehalem_cache_test(void) {
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    
    if (has_sse4_2) {
        /* Nehalem has 32KB L1, 256KB L2 per core */
        benchmark_cache_size(32);
        benchmark_cache_size(256);
    }
    
    decode_cpuid_cache_descriptors();
}

__attribute__((optimize("-mtune=haswell"))) 
void haswell_cache_test(void) {
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    if (has_avx2) {
        /* Haswell has 32KB L1, 256KB L2 */
        benchmark_cache_size(32);
        benchmark_cache_size(256);
        benchmark_cache_size(8192); /* L3 */
    }
    
    decode_cpuid_cache_descriptors();
}

__attribute__((optimize("-mtune=skylake"))) 
void skylake_cache_test(void) {
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    if (has_avx512f) {
        /* Skylake-X has up to 32KB L1, 1MB L2 */
        benchmark_cache_size(32);
        benchmark_cache_size(1024);
    }
    
    decode_cpuid_cache_descriptors();
}

/* Main function with complex control flow */
int main(void) {
    volatile unsigned long long checksum = 0;
    
    /* Initialize CPU detection - triggers driver cache detection */
    __builtin_cpu_init();
    
    /* Volatile CPU feature flags */
    volatile int has_mmx = __builtin_cpu_supports("mmx");
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Complex if-else chain based on CPU features */
    if (has_mmx) {
        checksum += 1;
        benchmark_cache_size(8);  /* Early CPUs had 8KB L1 */
        goto mmx_block;
    } else {
        goto legacy_block;
    }
    
mmx_block:
    if (has_sse) {
        checksum += 2;
        benchmark_cache_size(16); /* Pentium III had 16KB L1 */
        generic_cache_test();
    }
    
    if (has_sse2) {
        checksum += 4;
        benchmark_cache_size(32); /* Pentium 4 had 32KB L1 */
        
        /* Call architecture-specific functions */
        core2_cache_test();
        nehalem_cache_test();
    }
    
    if (has_sse3) {
        checksum += 8;
        benchmark_cache_size(64);
    }
    
    if (has_ssse3) {
        checksum += 16;
        benchmark_cache_size(128);
    }
    
    if (has_sse4_1 || has_sse4_2) {
        checksum += 32;
        benchmark_cache_size(256);
        haswell_cache_test();
    }
    
    if (has_avx) {
        checksum += 64;
        benchmark_cache_size(512);
    }
    
    if (has_avx2) {
        checksum += 128;
        benchmark_cache_size(1024);
        skylake_cache_test();
    }
    
    goto final_block;
    
legacy_block:
    /* Legacy path for very old CPUs */
    benchmark_cache_size(4);
    checksum += 256;
    
final_block:
    /* Execute CPUID leaf 0x2 directly */
    decode_cpuid_cache_descriptors();
    
    /* Use the detected cache parameters */
    if (l1_size_kb > 0) {
        checksum += l1_size_kb;
    }
    if (l2_size_kb > 0) {
        checksum += l2_size_kb;
    }
    
    /* Matrix traversal - cache size sensitive */
    {
        /* Use sizes from uncovered block */
        unsigned int matrix_sizes[] = {8, 16, 32, 128, 256, 512, 1024, 2048, 4096};
        volatile int matrix_sum = 0;
        
        for (unsigned int s = 0; s < sizeof(matrix_sizes)/sizeof(matrix_sizes[0]); s++) {
            unsigned int size_kb = matrix_sizes[s];
            unsigned int dim = (size_kb * 256); /* Approximate sqrt(size) */
            if (dim > 2048) dim = 2048;
            
            int *matrix = (int*)malloc(dim * dim * sizeof(int));
            if (!matrix) continue;
            
            /* Initialize */
            for (unsigned int i = 0; i < dim * dim; i++) {
                matrix[i] = i % 256;
            }
            
            /* Traverse - different patterns */
            for (unsigned int i = 0; i < dim; i++) {
                for (unsigned int j = 0; j < dim; j++) {
                    matrix_sum += matrix[i * dim + j];
                }
            }
            
            /* Another pattern */
            for (unsigned int j = 0; j < dim; j++) {
                for (unsigned int i = 0; i < dim; i++) {
                    matrix_sum += matrix[i * dim + j];
                }
            }
            
            checksum += matrix_sum;
            free(matrix);
        }
    }
    
    /* Print results */
    printf("Cache detection results:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
