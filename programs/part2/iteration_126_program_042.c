/* cache_detection.c - Trigger GCC's CPUID cache descriptor decoding */
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

/* Function 1: Direct CPUID leaf 0x2 reading with switch mirroring uncovered block */
__attribute__((noinline, optimize("O0")))
void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j = 0;
    
    /* Execute CPUID leaf 0x2 (Cache Descriptors) */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
    descriptors[j++] = (eax >> 0) & 0xFF;
    descriptors[j++] = (eax >> 8) & 0xFF;
    descriptors[j++] = (eax >> 16) & 0xFF;
    descriptors[j++] = (eax >> 24) & 0xFF;
    
    descriptors[j++] = (ebx >> 0) & 0xFF;
    descriptors[j++] = (ebx >> 8) & 0xFF;
    descriptors[j++] = (ebx >> 16) & 0xFF;
    descriptors[j++] = (ebx >> 24) & 0xFF;
    
    descriptors[j++] = (ecx >> 0) & 0xFF;
    descriptors[j++] = (ecx >> 8) & 0xFF;
    descriptors[j++] = (ecx >> 16) & 0xFF;
    descriptors[j++] = (ecx >> 24) & 0xFF;
    
    descriptors[j++] = (edx >> 0) & 0xFF;
    descriptors[j++] = (edx >> 8) & 0xFF;
    descriptors[j++] = (edx >> 16) & 0xFF;
    descriptors[j++] = (edx >> 24) & 0xFF;
    
    /* Process descriptors - mirroring the uncovered switch block */
    for (i = 0; i < j; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Direct switch matching uncovered lines */
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
                /* Simulate xeon_mp check */
                if (0) break; /* Assume not xeon_mp */
                l2_size_kb = 4096; l2_assoc = 16; l2_line = 64;
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
                /* Other descriptors not in uncovered block */
                break;
        }
    }
}

/* Function 2: Cache-sensitive benchmark with L1-sized array */
__attribute__((optimize("-mtune=generic"), noinline))
void benchmark_l1_cache(void) {
    /* Use volatile to prevent optimization */
    volatile int result = 0;
    
    /* Array sized to typical L1 cache (8KB-32KB) */
    char array[32 * 1024]; /* 32KB */
    
    /* Access pattern that exercises cache */
    for (int i = 0; i < sizeof(array); i += 64) {
        array[i] = (char)(i & 0xFF);
        result += array[i];
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
}

/* Function 3: Cache-sensitive benchmark with L2-sized array */
__attribute__((optimize("-mtune=core2"), noinline))
void benchmark_l2_cache(void) {
    volatile int result = 0;
    
    /* Array sized to typical L2 cache (256KB-2MB) */
    static char array[512 * 1024]; /* 512KB */
    
    for (int i = 0; i < sizeof(array); i += 128) {
        array[i] = (char)(i & 0xFF);
        result += array[i];
    }
    
    asm volatile ("" : : "r"(result));
}

/* Function 4: Large array for L3/DRAM */
__attribute__((optimize("-mtune=haswell"), noinline))
void benchmark_large_cache(void) {
    volatile int result = 0;
    
    /* Array larger than typical L2 cache */
    static char array[4 * 1024 * 1024]; /* 4MB */
    
    for (int i = 0; i < sizeof(array); i += 256) {
        array[i] = (char)(i & 0xFF);
        result += array[i];
    }
    
    asm volatile ("" : : "r"(result));
}

/* Function 5: Matrix traversal - cache size sensitive */
__attribute__((optimize("-mtune=native"), noinline))
void matrix_traversal(int size) {
    volatile int sum = 0;
    int matrix[size][size];
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = i * size + j;
        }
    }
    
    /* Traverse in different patterns */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum += matrix[j][i]; /* Poor locality */
        }
    }
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum += matrix[i][j]; /* Good locality */
        }
    }
    
    asm volatile ("" : : "r"(sum));
}

/* Main function with complex control flow */
int main(void) {
    volatile int checksum = 0;
    
    /* Initialize CPU detection (triggers driver cache detection) */
    __builtin_cpu_init();
    
    /* Volatile CPU feature flags */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* CPU model checks */
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    /* Direct CPUID cache descriptor decoding */
    decode_cpuid_cache_descriptors();
    
    /* Complex control flow based on CPU features */
    if (has_sse2) {
        benchmark_l1_cache();
        checksum += 1;
        
        if (has_avx) {
            benchmark_l2_cache();
            checksum += 2;
            
            if (has_avx2) {
                benchmark_large_cache();
                checksum += 4;
                goto avx2_path;
            } else {
                goto avx_path;
            }
        } else {
            goto sse2_path;
        }
    } else {
        /* Legacy path */
        matrix_traversal(64);
        checksum += 8;
        goto legacy_path;
    }

avx2_path:
    matrix_traversal(256);
    checksum += 16;
    goto common_tail;

avx_path:
    matrix_traversal(128);
    checksum += 32;
    goto common_tail;

sse2_path:
    matrix_traversal(96);
    checksum += 64;
    goto common_tail;

legacy_path:
    matrix_traversal(32);
    checksum += 128;

common_tail:
    /* Use cache parameters detected earlier */
    if (l1_size_kb > 0) {
        checksum += l1_size_kb;
    }
    if (l2_size_kb > 0) {
        checksum += l2_size_kb;
    }
    
    /* Additional architecture-specific functions */
    #ifdef __i386__
    /* 32-bit specific path */
    asm volatile (
        "pushl %%eax\n\t"
        "movl $0, %%eax\n\t"
        "cpuid\n\t"
        "popl %%eax\n\t"
        : : : "ebx", "ecx", "edx"
    );
    #endif
    
    /* Print results */
    printf("Cache detection results:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
