/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables matching the uncovered structure fields */
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
void generic_tuned_function(int* data, int size) {
    /* Simple memory access pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i * 2;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int* data, int size) {
    /* Different access pattern */
    for (int i = 0; i < size; i += 2) {
        data[i] = i * 3;
        if (i + 1 < size) {
            data[i + 1] = (i + 1) * 3;
        }
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int* data, int size) {
    /* Strided access pattern */
    for (int i = 0; i < size; i += 4) {
        data[i] = i * 4;
        if (i + 1 < size) data[i + 1] = (i + 1) * 4;
        if (i + 2 < size) data[i + 2] = (i + 2) * 4;
        if (i + 3 < size) data[i + 3] = (i + 3) * 4;
    }
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j, bytes_read = 0;
    
    /* Read CPUID leaf 0x2 - Cache descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
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
    
    /* Process descriptors - this switch mirrors the uncovered code */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0xFF) continue;
        
        /* Force compiler to generate switch for each case */
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
                if (!is_intel) {
                    level2_sizekb = 4096; level2_assoc = 16; level2_line = 64;
                }
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
                /* Unknown descriptor */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions */
void benchmark_l1_cache(int size_kb) {
    int elements = (size_kb * 1024) / sizeof(int);
    int* data = (int*)malloc(elements * sizeof(int));
    
    if (!data) return;
    
    /* Access pattern that should fit in L1 */
    for (int i = 0; i < elements; i++) {
        data[i] = i;
    }
    
    /* Compute checksum */
    volatile int sum = 0;
    for (int i = 0; i < elements; i++) {
        sum += data[i];
    }
    
    free(data);
}

void benchmark_l2_cache(int size_kb) {
    int elements = (size_kb * 1024) / sizeof(int);
    int* data = (int*)malloc(elements * sizeof(int));
    
    if (!data) return;
    
    /* Strided access pattern for L2 */
    for (int i = 0; i < elements; i += 16) {
        data[i] = i * 2;
    }
    
    /* Random-like access */
    volatile int sum = 0;
    for (int i = 0; i < elements; i = (i * 13 + 7) % elements) {
        sum += data[i];
    }
    
    free(data);
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
    
    /* Read cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Complex control flow based on CPU features */
    volatile int checksum = 0;
    
    if (has_sse2) {
        goto sse2_block;
    } else {
        goto legacy_block;
    }
    
sse2_block:
    {
        /* Allocate arrays matching cache sizes from uncovered cases */
        int* data8k = (int*)malloc(8 * 1024);      /* 0x0a: 8KB L1 */
        int* data16k = (int*)malloc(16 * 1024);    /* 0x0c: 16KB L1 */
        int* data256k = (int*)malloc(256 * 1024);  /* 0x21: 256KB L2 */
        int* data1m = (int*)malloc(1024 * 1024);   /* 0x24: 1024KB L2 */
        
        if (data8k) {
            generic_tuned_function(data8k, (8 * 1024) / sizeof(int));
            for (int i = 0; i < (8 * 1024) / sizeof(int); i++) {
                checksum += data8k[i];
            }
            free(data8k);
        }
        
        if (data16k) {
            core2_tuned_function(data16k, (16 * 1024) / sizeof(int));
            for (int i = 0; i < (16 * 1024) / sizeof(int); i++) {
                checksum += data16k[i];
            }
            free(data16k);
        }
        
        if (has_avx) {
            goto avx_block;
        }
        
        if (data256k) {
            haswell_tuned_function(data256k, (256 * 1024) / sizeof(int));
            for (int i = 0; i < (256 * 1024) / sizeof(int); i++) {
                checksum += data256k[i];
            }
            free(data256k);
        }
        
        if (data1m) {
            generic_tuned_function(data1m, (1024 * 1024) / sizeof(int));
            for (int i = 0; i < (1024 * 1024) / sizeof(int); i++) {
                checksum += data1m[i];
            }
            free(data1m);
        }
        
        goto benchmark_section;
    }
    
avx_block:
    {
        /* Additional AVX-specific paths */
        if (has_avx2) {
            benchmark_l1_cache(32);  /* 0x2c: 32KB L1 */
            benchmark_l2_cache(512); /* 0x7b: 512KB L2 */
        } else {
            benchmark_l1_cache(16);  /* 0x0c: 16KB L1 */
            benchmark_l2_cache(256); /* 0x21: 256KB L2 */
        }
        goto benchmark_section;
    }
    
legacy_block:
    {
        /* Legacy CPU paths */
        benchmark_l1_cache(8);   /* 0x0a: 8KB L1 */
        benchmark_l2_cache(128); /* 0x39: 128KB L2 */
        goto benchmark_section;
    }
    
benchmark_section:
    /* Execute benchmarks based on detected cache sizes */
    if (level1_sizekb > 0) {
        benchmark_l1_cache(level1_sizekb);
    }
    
    if (level2_sizekb > 0) {
        benchmark_l2_cache(level2_sizekb);
    }
    
    /* Force use of all volatile variables to prevent optimization */
    checksum += has_sse2 + has_avx + has_avx2 + is_intel + is_amd;
    checksum += level1_sizekb + level1_assoc + level1_line;
    checksum += level2_sizekb + level2_assoc + level2_line;
    
    printf("Cache detection complete. Checksum: %d\n", checksum);
    printf("L1: %dKB, %d-way, %dB line\n", level1_sizekb, level1_assoc, level1_line);
    printf("L2: %dKB, %d-way, %dB line\n", level2_sizekb, level2_assoc, level2_line);
    
    return 0;
}
