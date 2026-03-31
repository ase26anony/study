/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
struct cache_params {
    int sizekb;
    int assoc;
    int line;
};

struct cache_params level1 = {0, 0, 0};
struct cache_params level2 = {0, 0, 0};
volatile int xeon_mp = 0;

/* Force compiler to consider different microarchitectures */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache */
    char buffer1[8 * 1024];  /* 8KB - matches case 0x0a */
    for (int i = 0; i < sizeof(buffer1); i += 64) {
        sum += buffer1[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache for Core 2 */
    char buffer2[256 * 1024];  /* 256KB - matches case 0x21 */
    for (int i = 0; i < sizeof(buffer2); i += 64) {
        sum += buffer2[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to L2 cache for Nehalem */
    char buffer3[256 * 1024];  /* 256KB */
    for (int i = 0; i < sizeof(buffer3); i += 64) {
        sum += buffer3[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to L2 cache for Haswell */
    char buffer4[256 * 1024];  /* 256KB */
    for (int i = 0; i < sizeof(buffer4); i += 64) {
        sum += buffer4[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
    descriptors[0] = eax & 0xFF;
    descriptors[1] = (eax >> 8) & 0xFF;
    descriptors[2] = (eax >> 16) & 0xFF;
    descriptors[3] = (eax >> 24) & 0xFF;
    descriptors[4] = ebx & 0xFF;
    descriptors[5] = (ebx >> 8) & 0xFF;
    descriptors[6] = (ebx >> 16) & 0xFF;
    descriptors[7] = (ebx >> 24) & 0xFF;
    descriptors[8] = ecx & 0xFF;
    descriptors[9] = (ecx >> 8) & 0xFF;
    descriptors[10] = (ecx >> 16) & 0xFF;
    descriptors[11] = (ecx >> 24) & 0xFF;
    descriptors[12] = edx & 0xFF;
    descriptors[13] = (edx >> 8) & 0xFF;
    descriptors[14] = (edx >> 16) & 0xFF;
    descriptors[15] = (edx >> 24) & 0xFF;
    
    /* Process descriptors - mirroring driver-i386.cc switch */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0xFF)
            continue;
        
        /* Direct mapping of uncovered switch cases */
        switch (desc) {
            case 0x0a:
                level1.sizekb = 8; level1.assoc = 2; level1.line = 32;
                break;
            case 0x0c:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 32;
                break;
            case 0x0d:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
                break;
            case 0x0e:
                level1.sizekb = 24; level1.assoc = 6; level1.line = 64;
                break;
            case 0x21:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
                break;
            case 0x24:
                level2.sizekb = 1024; level2.assoc = 16; level2.line = 64;
                break;
            case 0x2c:
                level1.sizekb = 32; level1.assoc = 8; level1.line = 64;
                break;
            case 0x39:
                level2.sizekb = 128; level2.assoc = 4; level2.line = 64;
                break;
            case 0x3a:
                level2.sizekb = 192; level2.assoc = 6; level2.line = 64;
                break;
            case 0x3b:
                level2.sizekb = 128; level2.assoc = 2; level2.line = 64;
                break;
            case 0x3c:
                level2.sizekb = 256; level2.assoc = 4; level2.line = 64;
                break;
            case 0x3d:
                level2.sizekb = 384; level2.assoc = 6; level2.line = 64;
                break;
            case 0x3e:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
                break;
            case 0x41:
                level2.sizekb = 128; level2.assoc = 4; level2.line = 32;
                break;
            case 0x42:
                level2.sizekb = 256; level2.assoc = 4; level2.line = 32;
                break;
            case 0x43:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 32;
                break;
            case 0x44:
                level2.sizekb = 1024; level2.assoc = 4; level2.line = 32;
                break;
            case 0x45:
                level2.sizekb = 2048; level2.assoc = 4; level2.line = 32;
                break;
            case 0x48:
                level2.sizekb = 3072; level2.assoc = 12; level2.line = 64;
                break;
            case 0x49:
                if (!xeon_mp) {
                    level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
                }
                break;
            case 0x4e:
                level2.sizekb = 6144; level2.assoc = 24; level2.line = 64;
                break;
            case 0x60:
                level1.sizekb = 16; level1.assoc = 8; level1.line = 64;
                break;
            case 0x66:
                level1.sizekb = 8; level1.assoc = 4; level1.line = 64;
                break;
            case 0x67:
                level1.sizekb = 16; level1.assoc = 4; level1.line = 64;
                break;
            case 0x68:
                level1.sizekb = 32; level1.assoc = 4; level1.line = 64;
                break;
            case 0x78:
                level2.sizekb = 1024; level2.assoc = 4; level2.line = 64;
                break;
            case 0x79:
                level2.sizekb = 128; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7a:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7b:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7c:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7d:
                level2.sizekb = 2048; level2.assoc = 8; level2.line = 64;
                break;
            case 0x7f:
                level2.sizekb = 512; level2.assoc = 2; level2.line = 64;
                break;
            case 0x80:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 64;
                break;
            case 0x82:
                level2.sizekb = 256; level2.assoc = 8; level2.line = 32;
                break;
            case 0x83:
                level2.sizekb = 512; level2.assoc = 8; level2.line = 32;
                break;
            case 0x84:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 32;
                break;
            case 0x85:
                level2.sizekb = 2048; level2.assoc = 8; level2.line = 32;
                break;
            case 0x86:
                level2.sizekb = 512; level2.assoc = 4; level2.line = 64;
                break;
            case 0x87:
                level2.sizekb = 1024; level2.assoc = 8; level2.line = 64;
                break;
            default:
                /* Other descriptors not in uncovered block */
                break;
        }
    }
}

/* Cache-sensitive benchmark based on detected parameters */
volatile int benchmark_cache_sizes(void) {
    volatile int result = 0;
    volatile int use_sse2 = __builtin_cpu_supports("sse2");
    volatile int use_avx = __builtin_cpu_supports("avx");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    
    /* Force complex control flow based on CPU features */
    if (use_sse2) {
        goto sse2_block;
    } else {
        goto generic_block;
    }
    
sse2_block:
    if (use_avx) {
        /* Allocate arrays matching various cache sizes from uncovered block */
        char* l1_small = malloc(8 * 1024);      /* 0x0a */
        char* l1_medium = malloc(16 * 1024);    /* 0x0c, 0x0d */
        char* l1_large = malloc(32 * 1024);     /* 0x2c */
        
        for (int i = 0; i < 8 * 1024; i += 32) {
            result += l1_small[i];
        }
        for (int i = 0; i < 16 * 1024; i += 64) {
            result += l1_medium[i];
        }
        
        free(l1_small);
        free(l1_medium);
        free(l1_large);
        
        if (is_nehalem) {
            goto nehalem_block;
        }
    } else {
        char* l2_small = malloc(128 * 1024);    /* 0x39, 0x3b, 0x41 */
        char* l2_medium = malloc(256 * 1024);   /* 0x21, 0x3c, 0x42 */
        
        for (int i = 0; i < 128 * 1024; i += 64) {
            result += l2_small[i];
        }
        
        free(l2_small);
        free(l2_medium);
    }
    
    if (is_core2) {
        goto core2_block;
    }
    
    goto finish;
    
generic_block:
    /* Generic cache test with multiple sizes */
    for (int size_idx = 0; size_idx < 5; size_idx++) {
        int sizes[] = {8, 16, 32, 64, 128};
        int size_kb = sizes[size_idx];
        char* buffer = malloc(size_kb * 1024);
        
        for (int i = 0; i < size_kb * 1024; i += 32) {
            result += buffer[i];
        }
        
        free(buffer);
    }
    goto finish;
    
core2_block:
    /* Core2-specific pattern */
    char* core2_l2 = malloc(256 * 1024);  /* 0x21 */
    for (int i = 0; i < 256 * 1024; i += 64) {
        result += core2_l2[i];
    }
    free(core2_l2);
    goto finish;
    
nehalem_block:
    /* Nehalem-specific pattern */
    char* nehalem_l2 = malloc(256 * 1024);
    for (int i = 0; i < 256 * 1024; i += 64) {
        result += nehalem_l2[i];
    }
    free(nehalem_l2);
    goto finish;
    
finish:
    return result;
}

int main(void) {
    /* Initialize CPU detection - triggers GCC's internal cache detection */
    __builtin_cpu_init();
    
    printf("CPU Detection Initialized\n");
    
    /* Use volatile to prevent optimization of CPU checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    printf("SSE2: %d, SSE3: %d, SSSE3: %d, SSE4.1: %d, SSE4.2: %d, AVX: %d, AVX2: %d\n",
           has_sse2, has_sse3, has_ssse3, has_sse4_1, has_sse4_2, has_avx, has_avx2);
    
    /* Call architecture-specific functions to trigger different -mtune paths */
    generic_cache_test();
    core2_cache_test();
    nehalem_cache_test();
    haswell_cache_test();
    
    /* Direct CPUID reading to trigger descriptor decoding */
    read_cpuid_cache_descriptors();
    
    printf("Detected L1 Cache: %dKB, %d-way, %d-byte line\n",
           level1.sizekb, level1.assoc, level1.line);
    printf("Detected L2 Cache: %dKB, %d-way, %d-byte line\n",
           level2.sizekb, level2.assoc, level2.line);
    
    /* Run cache-sensitive benchmark */
    volatile int benchmark_result = benchmark_cache_sizes();
    
    /* Matrix traversal to hint at cache optimization */
    const int MATRIX_SIZE = 512;  /* Fits in L2 cache for many cases */
    volatile int matrix_sum = 0;
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    /* Initialize matrix */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = i % 256;
    }
    
    /* Traverse in different patterns to exercise cache */
    for (int iter = 0; iter < 10; iter++) {
        /* Row-major traversal */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_sum += matrix[i * MATRIX_SIZE + j];
            }
        }
        
        /* Column-major traversal (cache-inefficient) */
        for (int j = 0; j < MATRIX_SIZE; j++) {
            for (int i = 0; i < MATRIX_SIZE; i++) {
                matrix_sum += matrix[i * MATRIX_SIZE + j];
            }
        }
    }
    
    free(matrix);
    
    /* Final checksum to prevent dead code elimination */
    volatile int final_result = benchmark_result + matrix_sum;
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
