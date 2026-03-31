/* cache_detection.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
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

/* Function to manually decode CPUID leaf 0x2 descriptors */
__attribute__((noinline, optimize("O0")))
void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x2)
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
    
    /* Process each descriptor byte (mirroring driver-i386.cc logic) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Manual switch over descriptor values */
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
                /* Simulating xeon_mp check */
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

/* Architecture-specific functions with different -mtune optimizations */
__attribute__((optimize("-mtune=generic")))
void benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match cache sizes from uncovered block */
    char array_8k[8 * 1024];
    char array_256k[256 * 1024];
    
    for (int i = 0; i < sizeof(array_8k); i++) {
        array_8k[i] = (char)(i % 256);
        sum += array_8k[i];
    }
    
    for (int i = 0; i < sizeof(array_256k); i += 64) {
        array_256k[i] = (char)(i % 256);
        sum += array_256k[i];
    }
    
    l1_size_kb += (sum & 1); /* Prevent optimization */
}

__attribute__((optimize("-mtune=core2")))
void benchmark_core2(void) {
    volatile int sum = 0;
    char array_16k[16 * 1024];
    char array_1024k[1024 * 1024];
    
    for (int i = 0; i < sizeof(array_16k); i++) {
        array_16k[i] = (char)(i % 256);
        sum += array_16k[i];
    }
    
    for (int i = 0; i < sizeof(array_1024k); i += 64) {
        array_1024k[i] = (char)(i % 256);
        sum += array_1024k[i];
    }
    
    l2_size_kb += (sum & 1);
}

__attribute__((optimize("-mtune=haswell")))
void benchmark_haswell(void) {
    volatile int sum = 0;
    char array_32k[32 * 1024];
    char array_2048k[2048 * 1024];
    
    for (int i = 0; i < sizeof(array_32k); i++) {
        array_32k[i] = (char)(i % 256);
        sum += array_32k[i];
    }
    
    for (int i = 0; i < sizeof(array_2048k); i += 64) {
        array_2048k[i] = (char)(i % 256);
        sum += array_2048k[i];
    }
    
    l1_line += (sum & 1);
}

__attribute__((optimize("-mtune=pentium4")))
void benchmark_pentium4(void) {
    volatile int sum = 0;
    char array_24k[24 * 1024];
    char array_512k[512 * 1024];
    
    for (int i = 0; i < sizeof(array_24k); i++) {
        array_24k[i] = (char)(i % 256);
        sum += array_24k[i];
    }
    
    for (int i = 0; i < sizeof(array_512k); i += 64) {
        array_512k[i] = (char)(i % 256);
        sum += array_512k[i];
    }
    
    l2_assoc += (sum & 1);
}

/* Main function with complex control flow based on CPU features */
int main(void) {
    volatile int checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    /* Decode CPUID cache descriptors directly */
    decode_cpuid_cache_descriptors();
    
    /* Complex control flow with goto to prevent optimization */
    if (has_sse2) {
        benchmark_generic();
        checksum += 1;
        if (is_core2) goto core2_path;
    }
    
    if (has_avx) {
        benchmark_haswell();
        checksum += 2;
        if (is_haswell) goto haswell_path;
    }
    
    if (has_avx2) {
        benchmark_haswell();
        checksum += 4;
    }
    
    /* Always execute these paths */
    benchmark_pentium4();
    checksum += 8;
    
core2_path:
    benchmark_core2();
    checksum += 16;
    
haswell_path:
    benchmark_haswell();
    checksum += 32;
    
    /* Final checksum computation using cache parameters */
    checksum += l1_size_kb + l1_assoc + l1_line;
    checksum += l2_size_kb + l2_assoc + l2_line;
    
    /* Print results */
    printf("Cache Parameters Detected:\n");
    printf("  L1: %u KB, %u-way, %u byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    
    /* Additional CPUID leaf 0x2 reading for good measure */
    {
        uint32_t eax, ebx, ecx, edx;
        asm volatile (
            "mov $0x2, %%eax\n\t"
            "cpuid\n\t"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            :
            : "cc"
        );
        printf("CPUID Leaf 0x2: EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n",
               eax, ebx, ecx, edx);
    }
    
    return checksum & 255; /* Return non-zero to indicate execution */
}
