/* cache_detection.c - Trigger GCC's CPUID cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables matching the uncovered structure pattern */
volatile unsigned int level1_sizekb = 0;
volatile unsigned int level1_assoc = 0;
volatile unsigned int level1_line = 0;
volatile unsigned int level2_sizekb = 0;
volatile unsigned int level2_assoc = 0;
volatile unsigned int level2_line = 0;

/* Volatile CPU feature flags to force runtime evaluation */
volatile int has_sse2 = 0;
volatile int has_avx = 0;
volatile int has_avx2 = 0;
volatile int is_intel = 0;
volatile int is_amd = 0;

/* Function with architecture-specific optimization hints */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile char buffer[8192]; /* 8KB - matches case 0x0a */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile char buffer[32768]; /* 32KB - matches case 0x2c */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (i * 3) & 0xFF;
    }
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_cache_test(void) {
    volatile char buffer[262144]; /* 256KB - matches case 0x21 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (i * 5) & 0xFF;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile char buffer[1048576]; /* 1024KB - matches case 0x24 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = (i * 7) & 0xFF;
    }
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
    
    /* Extract descriptor bytes (eax contains number of iterations) */
    descriptors[0] = (eax >> 0) & 0xFF;
    descriptors[1] = (eax >> 8) & 0xFF;
    descriptors[2] = (eax >> 16) & 0xFF;
    descriptors[3] = (eax >> 24) & 0xFF;
    descriptors[4] = (ebx >> 0) & 0xFF;
    descriptors[5] = (ebx >> 8) & 0xFF;
    descriptors[6] = (ebx >> 16) & 0xFF;
    descriptors[7] = (ebx >> 24) & 0xFF;
    descriptors[8] = (ecx >> 0) & 0xFF;
    descriptors[9] = (ecx >> 8) & 0xFF;
    descriptors[10] = (ecx >> 16) & 0xFF;
    descriptors[11] = (ecx >> 24) & 0xFF;
    descriptors[12] = (edx >> 0) & 0xFF;
    descriptors[13] = (edx >> 8) & 0xFF;
    descriptors[14] = (edx >> 16) & 0xFF;
    descriptors[15] = (edx >> 24) & 0xFF;
    
    /* Process descriptors - mirroring the uncovered switch cases */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0xFF) continue;
        
        /* Direct mapping of descriptor values to cache parameters */
        switch (desc) {
            case 0x0a:
                level1_sizekb = 8; level1_assoc = 2; level1_line = 32;
                break;
            case 0x0c:
                level1_sizekb = 16; level1_assoc = 4; level1_line = 32;
                break;
            case 0x0d:
                level1_sizekb = 16; level1_assoc = 4; level1_line = 64;
                break;
            case 0x0e:
                level1_sizekb = 24; level1_assoc = 6; level1_line = 64;
                break;
            case 0x21:
                level2_sizekb = 256; level2_assoc = 8; level2_line = 64;
                break;
            case 0x24:
                level2_sizekb = 1024; level2_assoc = 16; level2_line = 64;
                break;
            case 0x2c:
                level1_sizekb = 32; level1_assoc = 8; level1_line = 64;
                break;
            case 0x39:
                level2_sizekb = 128; level2_assoc = 4; level2_line = 64;
                break;
            case 0x3a:
                level2_sizekb = 192; level2_assoc = 6; level2_line = 64;
                break;
            case 0x3b:
                level2_sizekb = 128; level2_assoc = 2; level2_line = 64;
                break;
            case 0x3c:
                level2_sizekb = 256; level2_assoc = 4; level2_line = 64;
                break;
            case 0x3d:
                level2_sizekb = 384; level2_assoc = 6; level2_line = 64;
                break;
            case 0x3e:
                level2_sizekb = 512; level2_assoc = 4; level2_line = 64;
                break;
            case 0x41:
                level2_sizekb = 128; level2_assoc = 4; level2_line = 32;
                break;
            case 0x42:
                level2_sizekb = 256; level2_assoc = 4; level2_line = 32;
                break;
            case 0x43:
                level2_sizekb = 512; level2_assoc = 4; level2_line = 32;
                break;
            case 0x44:
                level2_sizekb = 1024; level2_assoc = 4; level2_line = 32;
                break;
            case 0x45:
                level2_sizekb = 2048; level2_assoc = 4; level2_line = 32;
                break;
            case 0x48:
                level2_sizekb = 3072; level2_assoc = 12; level2_line = 64;
                break;
            case 0x49:
                /* Simulating xeon_mp check */
                if (is_intel && has_avx) {
                    /* xeon_mp would be true, so break */
                    break;
                }
                level2_sizekb = 4096; level2_assoc = 16; level2_line = 64;
                break;
            case 0x4e:
                level2_sizekb = 6144; level2_assoc = 24; level2_line = 64;
                break;
            case 0x60:
                level1_sizekb = 16; level1_assoc = 8; level1_line = 64;
                break;
            case 0x66:
                level1_sizekb = 8; level1_assoc = 4; level1_line = 64;
                break;
            case 0x67:
                level1_sizekb = 16; level1_assoc = 4; level1_line = 64;
                break;
            case 0x68:
                level1_sizekb = 32; level1_assoc = 4; level1_line = 64;
                break;
            case 0x78:
                level2_sizekb = 1024; level2_assoc = 4; level2_line = 64;
                break;
            case 0x79:
                level2_sizekb = 128; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7a:
                level2_sizekb = 256; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7b:
                level2_sizekb = 512; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7c:
                level2_sizekb = 1024; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7d:
                level2_sizekb = 2048; level2_assoc = 8; level2_line = 64;
                break;
            case 0x7f:
                level2_sizekb = 512; level2_assoc = 2; level2_line = 64;
                break;
            case 0x80:
                level2_sizekb = 512; level2_assoc = 8; level2_line = 64;
                break;
            case 0x82:
                level2_sizekb = 256; level2_assoc = 8; level2_line = 32;
                break;
            case 0x83:
                level2_sizekb = 512; level2_assoc = 8; level2_line = 32;
                break;
            case 0x84:
                level2_sizekb = 1024; level2_assoc = 8; level2_line = 32;
                break;
            case 0x85:
                level2_sizekb = 2048; level2_assoc = 8; level2_line = 32;
                break;
            case 0x86:
                level2_sizekb = 512; level2_assoc = 4; level2_line = 64;
                break;
            case 0x87:
                level2_sizekb = 1024; level2_assoc = 8; level2_line = 64;
                break;
            default:
                /* Other descriptors not in uncovered block */
                break;
        }
    }
}

/* Cache-sensitive benchmark function */
unsigned long long cache_sensitive_benchmark(size_t buffer_size) {
    volatile char *buffer = malloc(buffer_size);
    unsigned long long sum = 0;
    
    if (!buffer) return 0;
    
    /* Initialize with pattern */
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = (i * 13) & 0xFF;
    }
    
    /* Access pattern that depends on cache line size */
    for (size_t i = 0; i < buffer_size; i += 64) {
        sum += buffer[i];
    }
    
    free((void*)buffer);
    return sum;
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile flags based on CPU features */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Detect vendor */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );
    
    char vendor[13];
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    
    is_intel = (strcmp(vendor, "GenuineIntel") == 0);
    is_amd = (strcmp(vendor, "AuthenticAMD") == 0);
    
    /* Force execution of different optimization paths */
    generic_cache_test();
    core2_cache_test();
    nehalem_cache_test();
    haswell_cache_test();
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Complex control flow based on volatile CPU flags */
    unsigned long long total_sum = 0;
    
    if (has_sse2) {
        goto sse2_block;
    } else {
        goto legacy_block;
    }
    
sse2_block:
    total_sum += cache_sensitive_benchmark(8192);    /* 8KB */
    if (has_avx) {
        goto avx_block;
    }
    goto after_avx;
    
avx_block:
    total_sum += cache_sensitive_benchmark(262144);  /* 256KB */
    if (has_avx2) {
        total_sum += cache_sensitive_benchmark(1048576); /* 1024KB */
    }
    goto after_avx;
    
legacy_block:
    total_sum += cache_sensitive_benchmark(16384);   /* 16KB */
    goto after_avx;
    
after_avx:
    /* More cache-size-specific tests */
    if (level1_sizekb > 0) {
        total_sum += cache_sensitive_benchmark(level1_sizekb * 1024);
    }
    if (level2_sizekb > 0) {
        total_sum += cache_sensitive_benchmark(level2_sizekb * 1024);
    }
    
    /* Matrix traversal - cache size sensitive */
    {
        const int N = 256; /* 256x256 int matrix = 256KB */
        volatile int matrix[N][N];
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrix[i][j] = i * j;
                total_sum += matrix[i][j];
            }
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Cache detection results:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", 
           level2_sizekb, level2_assoc, level2_line);
    printf("Total checksum: %llu\n", total_sum);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", 
           has_sse2, has_avx, has_avx2);
    printf("Vendor: %s\n", vendor);
    
    return 0;
}
