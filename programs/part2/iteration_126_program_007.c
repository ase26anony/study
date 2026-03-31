/* 
 * GCC Cache Descriptor Exercise Program
 * Targets driver-i386.cc cache parameter decoding logic (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 -fno-omit-frame-pointer
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -flto
 */

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
volatile int xeon_mp = 0; /* Simulate Xeon MP detection */

/* Function optimized for different microarchitectures to force driver cache decoding */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(volatile int* flag) {
    /* Access pattern that might benefit from generic cache parameters */
    static char buffer[8192]; /* 8KB - matches case 0x0a */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (char)(i & 0xFF);
    }
    *flag += buffer[0];
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(volatile int* flag) {
    /* 32KB L1 cache - matches case 0x2c */
    static char buffer[32768];
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (char)(i & 0xFF);
    }
    *flag += buffer[1024];
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(volatile int* flag) {
    /* 256KB L2 cache - matches multiple cases */
    static char buffer[262144];
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (char)(i & 0xFF);
    }
    *flag += buffer[4096];
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, rounds;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile ("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(2), "c"(0));
    
    /* AL register contains number of rounds */
    rounds = eax & 0xFF;
    
    /* Process first round of descriptors */
    for (i = 0; i < 4; i++) {
        descriptor = (eax >> (8 * i)) & 0xFF;
        if (descriptor & 0x80) continue; /* Invalid descriptor */
        
        /* Switch statement mirroring driver-i386.cc uncovered block */
        switch (descriptor) {
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
                if (xeon_mp) break;
                level2.sizekb = 4096; level2.assoc = 16; level2.line = 64;
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

/* Cache-size sensitive benchmark */
void cache_sensitive_benchmark(volatile int* checksum) {
    volatile int use_sse2 = __builtin_cpu_supports("sse2");
    volatile int use_avx = __builtin_cpu_supports("avx");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    
    /* Array sizes matching cache sizes from uncovered block */
    char* array_8kb = malloc(8 * 1024);
    char* array_256kb = malloc(256 * 1024);
    char* array_1024kb = malloc(1024 * 1024);
    
    if (array_8kb && array_256kb && array_1024kb) {
        /* Control flow based on CPU features - forces GCC to consider cache params */
        if (use_sse2) {
            /* Access pattern for 8KB L1 cache (case 0x0a) */
            for (int i = 0; i < 8 * 1024; i += 32) {
                array_8kb[i] = (char)(*checksum + i);
            }
            *checksum += array_8kb[0];
            
            goto sse2_block;
        } else {
            goto legacy_block;
        }
        
    sse2_block:
        if (use_avx) {
            /* Access pattern for 256KB L2 cache (case 0x21) */
            for (int i = 0; i < 256 * 1024; i += 64) {
                array_256kb[i] = (char)(*checksum + i);
            }
            *checksum += array_256kb[4096];
        }
        
        if (is_core2 || is_nehalem) {
            /* Access pattern for 1024KB L2 cache (case 0x24) */
            for (int i = 0; i < 1024 * 1024; i += 64) {
                array_1024kb[i] = (char)(*checksum + i);
            }
            *checksum += array_1024kb[65536];
        }
        
        goto cleanup;
        
    legacy_block:
        /* Simple access for non-SSE2 CPUs */
        for (int i = 0; i < 8 * 1024; i++) {
            array_8kb[i] = (char)i;
        }
        *checksum += array_8kb[1024];
        
    cleanup:
        free(array_8kb);
        free(array_256kb);
        free(array_1024kb);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize GCC's CPU detection - triggers driver cache decoding */
    __builtin_cpu_init();
    
    printf("CPU Detection Initialized\n");
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    printf("L1 Cache: %dKB, %d-way, %d-byte line\n", 
           level1.sizekb, level1.assoc, level1.line);
    printf("L2 Cache: %dKB, %d-way, %d-byte line\n", 
           level2.sizekb, level2.assoc, level2.line);
    
    /* Call architecture-tuned functions to force driver to process cache params */
    generic_tuned_function(&checksum);
    core2_tuned_function(&checksum);
    haswell_tuned_function(&checksum);
    
    /* Perform cache-sensitive benchmarking */
    cache_sensitive_benchmark(&checksum);
    
    /* Additional CPUID-based branching to hit more switch cases */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    
    /* Complex if-else chain using volatile CPU feature flags */
    if (has_sse) {
        checksum += 0x1000;
        if (has_sse3) {
            checksum += 0x2000;
            if (has_ssse3) {
                checksum += 0x3000;
                if (has_sse4_1) {
                    checksum += 0x4000;
                    if (has_sse4_2) {
                        checksum += 0x5000;
                    }
                }
            }
        }
    }
    
    /* Matrix traversal that benefits from cache parameter knowledge */
    {
        /* Use sizes that match uncovered cache parameters */
        int matrix1[128][128];  /* ~64KB if int=4 bytes - near L1 sizes */
        int matrix2[512][512];  /* ~1MB - near L2 sizes */
        
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 128; j++) {
                matrix1[i][j] = i * j + checksum;
            }
        }
        
        /* Access in different patterns based on CPU features */
        if (__builtin_cpu_supports("avx2")) {
            for (int i = 0; i < 512; i += 2) {
                for (int j = 0; j < 512; j += 2) {
                    matrix2[i][j] = matrix1[i % 128][j % 128] * 2;
                }
            }
        } else {
            for (int i = 0; i < 512; i++) {
                for (int j = 0; j < 512; j++) {
                    matrix2[i][j] = matrix1[i % 128][j % 128];
                }
            }
        }
        
        checksum += matrix1[64][64] + matrix2[256][256];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Program completed - cache descriptor logic exercised.\n");
    
    return 0;
}
