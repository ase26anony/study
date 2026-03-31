/* 
 * cache_detection.c - Program to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
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

/* Volatile CPU feature flags to force runtime evaluation */
volatile int has_sse2 = 0;
volatile int has_avx = 0;
volatile int has_avx2 = 0;
volatile int is_intel = 0;
volatile int is_amd = 0;

/* Function to manually decode CPUID leaf 0x2 cache descriptors */
__attribute__((noinline, optimize("O0")))
void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j, bytes_valid;
    
    /* Execute CPUID leaf 0x2 to get cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
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
    
    /* Check if eax[7:0] gives number of valid descriptor bytes */
    bytes_valid = eax & 0xFF;
    if (bytes_valid > 15) bytes_valid = 1; /* Only byte 0 is valid */
    
    /* Process each valid descriptor byte */
    for (i = 0; i < bytes_valid; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip null descriptors and descriptor type 0 */
        if (desc == 0x00 || desc == 0xFF) continue;
        
        /* Manual switch statement mirroring driver-i386.cc logic */
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
                /* Check for Xeon MP - simplified */
                if (0) { /* xeon_mp check would go here */
                    break;
                }
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
                /* Unknown descriptor */
                break;
        }
    }
}

/* Architecture-specific functions with different mtune optimizations */

__attribute__((optimize("-mtune=generic"), noinline))
void benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match common cache sizes */
    int array8k[2048];  /* ~8KB */
    int array16k[4096]; /* ~16KB */
    
    for (int i = 0; i < 2048; i++) {
        array8k[i] = i;
        sum += array8k[i];
    }
    
    for (int i = 0; i < 4096; i++) {
        array16k[i] = i * 2;
        sum += array16k[i];
    }
    
    l1_size_kb += (sum & 1); /* Prevent optimization */
}

__attribute__((optimize("-mtune=core2"), noinline))
void benchmark_core2(void) {
    volatile int sum = 0;
    /* Match Core2 cache sizes: 32KB L1, larger L2 */
    int array32k[8192];   /* ~32KB */
    int array256k[65536]; /* ~256KB */
    
    for (int i = 0; i < 8192; i++) {
        array32k[i] = i * 3;
        sum += array32k[i];
    }
    
    for (int i = 0; i < 65536; i += 64) { /* Strided access */
        array256k[i] = i * 5;
        sum += array256k[i];
    }
    
    l2_size_kb += (sum & 1);
}

__attribute__((optimize("-mtune=haswell"), noinline))
void benchmark_haswell(void) {
    volatile int sum = 0;
    /* Haswell typically has 32KB L1, 256KB L2 */
    int array32k[8192];
    int array256k[65536];
    int array1m[262144]; /* 1MB */
    
    for (int i = 0; i < 8192; i++) {
        array32k[i] = i * 7;
        sum += array32k[i];
    }
    
    for (int i = 0; i < 65536; i += 32) {
        array256k[i] = i * 11;
        sum += array256k[i];
    }
    
    for (int i = 0; i < 262144; i += 128) {
        array1m[i] = i * 13;
        sum += array1m[i];
    }
    
    l2_assoc += (sum & 1);
}

__attribute__((optimize("-mtune=pentium4"), noinline))
void benchmark_pentium4(void) {
    volatile int sum = 0;
    /* Pentium 4 cache characteristics */
    int array8k[2048];   /* 8KB trace cache */
    int array512k[131072]; /* 512KB L2 */
    
    for (int i = 0; i < 2048; i++) {
        array8k[i] = i * 17;
        sum += array8k[i];
    }
    
    for (int i = 0; i < 131072; i += 64) {
        array512k[i] = i * 19;
        sum += array512k[i];
    }
    
    l1_line += (sum & 1);
}

/* Main function with complex control flow based on CPU features */
int main(void) {
    volatile int checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile flags based on CPU features */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Check vendor */
    uint32_t eax, ebx, ecx, edx;
    asm volatile ("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0) : );
    char vendor[13];
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    
    is_intel = (strcmp(vendor, "GenuineIntel") == 0);
    is_amd = (strcmp(vendor, "AuthenticAMD") == 0);
    
    /* Complex control flow with goto to prevent optimization */
    if (has_sse2) {
        goto sse2_block;
    } else {
        goto legacy_block;
    }

sse2_block:
    /* Call architecture-specific benchmarks */
    benchmark_generic();
    checksum += 1;
    
    if (has_avx) {
        benchmark_haswell();
        checksum += 2;
        
        if (has_avx2) {
            benchmark_core2(); /* Some AVX2 CPUs might be Core2-like */
            checksum += 4;
        }
    }
    
    if (is_intel) {
        /* Intel CPUs often use specific cache descriptor patterns */
        benchmark_pentium4();
        checksum += 8;
    }
    
    goto decode_block;

legacy_block:
    /* Fallback for older CPUs */
    benchmark_generic();
    checksum += 16;
    goto decode_block;

decode_block:
    /* Manually decode CPUID cache descriptors */
    decode_cpuid_cache_descriptors();
    
    /* Use decoded cache parameters to control array sizes */
    volatile size_t array_size = 0;
    
    if (l1_size_kb > 0) {
        array_size = l1_size_kb * 256; /* Convert KB to number of ints */
        int* l1_array = (int*)malloc(array_size * sizeof(int));
        if (l1_array) {
            for (size_t i = 0; i < array_size; i++) {
                l1_array[i] = i * 23;
                checksum += l1_array[i];
            }
            free(l1_array);
        }
    }
    
    if (l2_size_kb > 0) {
        array_size = l2_size_kb * 256; /* Larger array for L2 */
        int* l2_array = (int*)malloc(array_size * sizeof(int));
        if (l2_array) {
            /* Strided access pattern */
            for (size_t i = 0; i < array_size; i += l2_line / sizeof(int)) {
                l2_array[i] = i * 29;
                checksum += l2_array[i];
            }
            free(l2_array);
        }
    }
    
    /* Matrix traversal - cache size sensitive */
    if (l1_size_kb > 0 && l2_size_kb > 0) {
        size_t matrix_dim = (l1_size_kb * 1024) / sizeof(int);
        if (matrix_dim > 1000) matrix_dim = 1000;
        
        int* matrix = (int*)malloc(matrix_dim * matrix_dim * sizeof(int));
        if (matrix) {
            /* Row-major traversal */
            for (size_t i = 0; i < matrix_dim; i++) {
                for (size_t j = 0; j < matrix_dim; j++) {
                    matrix[i * matrix_dim + j] = i * j;
                    checksum += matrix[i * matrix_dim + j];
                }
            }
            
            /* Column-major traversal (cache-unfriendly) */
            for (size_t j = 0; j < matrix_dim; j++) {
                for (size_t i = 0; i < matrix_dim; i++) {
                    checksum -= matrix[i * matrix_dim + j];
                }
            }
            free(matrix);
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Cache Parameters:\n");
    printf("  L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", has_sse2, has_avx, has_avx2);
    printf("Vendor: %s\n", vendor);
    printf("Final checksum: %d\n", checksum & 0xFF);
    
    return (checksum & 0xFF);
}
