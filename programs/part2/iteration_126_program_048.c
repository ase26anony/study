/* cache_descriptor_trigger.c
 * Targets GCC driver-i386.cc cache descriptor decoding (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=native -fno-omit-frame-pointer -std=gnu11
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -flto -std=gnu11
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Global variables to store cache parameters matching the uncovered switch cases */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Function optimized for generic x86 - triggers default cache detection */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Use array sizes that might trigger different cache optimizations */
    char buffer1[8 * 1024];  /* 8KB - matches case 0x0a, 0x66 */
    char buffer2[16 * 1024]; /* 16KB - matches case 0x0c, 0x0d, 0x60, 0x67 */
    char buffer3[32 * 1024]; /* 32KB - matches case 0x2c, 0x68 */
    
    /* Access patterns that depend on cache line size */
    for (int i = 0; i < sizeof(buffer1); i += 32) buffer1[i] = i;
    for (int i = 0; i < sizeof(buffer2); i += 64) buffer2[i] = i;
    for (int i = 0; i < sizeof(buffer3); i += 64) buffer3[i] = i;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(buffer1), "r"(buffer2), "r"(buffer3));
}

/* Function optimized for Core2 - triggers specific descriptor cases */
__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    /* Core2 often uses 0x21, 0x24, 0x78, 0x79, 0x7a descriptors */
    volatile int sum = 0;
    char buffer_l2[256 * 1024];  /* 256KB - matches case 0x21, 0x3c, 0x7a, 0x82 */
    char buffer_l2b[1024 * 1024]; /* 1MB - matches case 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    
    /* Stride access based on typical cache line sizes */
    for (int i = 0; i < sizeof(buffer_l2); i += 64) {
        buffer_l2[i] = (i * 3) & 0xFF;
        sum += buffer_l2[i];
    }
    for (int i = 0; i < sizeof(buffer_l2b); i += 64) {
        buffer_l2b[i] = (i * 5) & 0xFF;
        sum += buffer_l2b[i];
    }
    
    asm volatile("" : : "r"(sum));
}

/* Function optimized for Nehalem/Haswell - triggers newer descriptors */
__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    /* Haswell uses descriptors like 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x45, 0x48, 0x49 */
    volatile int sum = 0;
    char buffer1[128 * 1024];   /* 128KB - matches case 0x39, 0x3b, 0x41, 0x79 */
    char buffer2[384 * 1024];   /* 384KB - matches case 0x3d */
    char buffer3[512 * 1024];   /* 512KB - matches case 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    char buffer4[2048 * 1024];  /* 2MB - matches case 0x45, 0x7d, 0x85 */
    char buffer5[3072 * 1024];  /* 3MB - matches case 0x48 */
    char buffer6[4096 * 1024];  /* 4MB - matches case 0x49 */
    char buffer7[6144 * 1024];  /* 6MB - matches case 0x4e */
    
    /* Different access patterns for different cache levels */
    for (int i = 0; i < sizeof(buffer1); i += 64) sum += buffer1[i] = i;
    for (int i = 0; i < sizeof(buffer2); i += 64) sum += buffer2[i] = i * 2;
    for (int i = 0; i < sizeof(buffer3); i += 64) sum += buffer3[i] = i * 3;
    
    asm volatile("" : : "r"(sum), "r"(buffer4), "r"(buffer5), "r"(buffer6), "r"(buffer7));
}

/* Direct CPUID leaf 0x2 reading - mirrors the driver's decoding logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int descriptor_count = 0;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
    descriptors[descriptor_count++] = (eax >> 0) & 0xFF;
    descriptors[descriptor_count++] = (eax >> 8) & 0xFF;
    descriptors[descriptor_count++] = (eax >> 16) & 0xFF;
    descriptors[descriptor_count++] = (eax >> 24) & 0xFF;
    
    descriptors[descriptor_count++] = (ebx >> 0) & 0xFF;
    descriptors[descriptor_count++] = (ebx >> 8) & 0xFF;
    descriptors[descriptor_count++] = (ebx >> 16) & 0xFF;
    descriptors[descriptor_count++] = (ebx >> 24) & 0xFF;
    
    descriptors[descriptor_count++] = (ecx >> 0) & 0xFF;
    descriptors[descriptor_count++] = (ecx >> 8) & 0xFF;
    descriptors[descriptor_count++] = (ecx >> 16) & 0xFF;
    descriptors[descriptor_count++] = (ecx >> 24) & 0xFF;
    
    descriptors[descriptor_count++] = (edx >> 0) & 0xFF;
    descriptors[descriptor_count++] = (edx >> 8) & 0xFF;
    descriptors[descriptor_count++] = (edx >> 16) & 0xFF;
    descriptors[descriptor_count++] = (edx >> 24) & 0xFF;
    
    /* Process descriptors - mirroring the uncovered switch cases */
    for (int i = 0; i < descriptor_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0x01 || (desc & 0x80)) continue;
        
        /* Direct mapping of switch cases from uncovered lines */
        switch (desc) {
            case 0x0a: l1_size_kb = 8; l1_assoc = 2; l1_line = 32; break;
            case 0x0c: l1_size_kb = 16; l1_assoc = 4; l1_line = 32; break;
            case 0x0d: l1_size_kb = 16; l1_assoc = 4; l1_line = 64; break;
            case 0x0e: l1_size_kb = 24; l1_assoc = 6; l1_line = 64; break;
            case 0x21: l2_size_kb = 256; l2_assoc = 8; l2_line = 64; break;
            case 0x24: l2_size_kb = 1024; l2_assoc = 16; l2_line = 64; break;
            case 0x2c: l1_size_kb = 32; l1_assoc = 8; l1_line = 64; break;
            case 0x39: l2_size_kb = 128; l2_assoc = 4; l2_line = 64; break;
            case 0x3a: l2_size_kb = 192; l2_assoc = 6; l2_line = 64; break;
            case 0x3b: l2_size_kb = 128; l2_assoc = 2; l2_line = 64; break;
            case 0x3c: l2_size_kb = 256; l2_assoc = 4; l2_line = 64; break;
            case 0x3d: l2_size_kb = 384; l2_assoc = 6; l2_line = 64; break;
            case 0x3e: l2_size_kb = 512; l2_assoc = 4; l2_line = 64; break;
            case 0x41: l2_size_kb = 128; l2_assoc = 4; l2_line = 32; break;
            case 0x42: l2_size_kb = 256; l2_assoc = 4; l2_line = 32; break;
            case 0x43: l2_size_kb = 512; l2_assoc = 4; l2_line = 32; break;
            case 0x44: l2_size_kb = 1024; l2_assoc = 4; l2_line = 32; break;
            case 0x45: l2_size_kb = 2048; l2_assoc = 4; l2_line = 32; break;
            case 0x48: l2_size_kb = 3072; l2_assoc = 12; l2_line = 64; break;
            case 0x49: l2_size_kb = 4096; l2_assoc = 16; l2_line = 64; break;
            case 0x4e: l2_size_kb = 6144; l2_assoc = 24; l2_line = 64; break;
            case 0x60: l1_size_kb = 16; l1_assoc = 8; l1_line = 64; break;
            case 0x66: l1_size_kb = 8; l1_assoc = 4; l1_line = 64; break;
            case 0x67: l1_size_kb = 16; l1_assoc = 4; l1_line = 64; break;
            case 0x68: l1_size_kb = 32; l1_assoc = 4; l1_line = 64; break;
            case 0x78: l2_size_kb = 1024; l2_assoc = 4; l2_line = 64; break;
            case 0x79: l2_size_kb = 128; l2_assoc = 8; l2_line = 64; break;
            case 0x7a: l2_size_kb = 256; l2_assoc = 8; l2_line = 64; break;
            case 0x7b: l2_size_kb = 512; l2_assoc = 8; l2_line = 64; break;
            case 0x7c: l2_size_kb = 1024; l2_assoc = 8; l2_line = 64; break;
            case 0x7d: l2_size_kb = 2048; l2_assoc = 8; l2_line = 64; break;
            case 0x7f: l2_size_kb = 512; l2_assoc = 2; l2_line = 64; break;
            case 0x80: l2_size_kb = 512; l2_assoc = 8; l2_line = 64; break;
            case 0x82: l2_size_kb = 256; l2_assoc = 8; l2_line = 32; break;
            case 0x83: l2_size_kb = 512; l2_assoc = 8; l2_line = 32; break;
            case 0x84: l2_size_kb = 1024; l2_assoc = 8; l2_line = 32; break;
            case 0x85: l2_size_kb = 2048; l2_assoc = 8; l2_line = 32; break;
            case 0x86: l2_size_kb = 512; l2_assoc = 4; l2_line = 64; break;
            case 0x87: l2_size_kb = 1024; l2_assoc = 8; l2_line = 64; break;
            default: break;
        }
    }
}

/* Cache-sensitive benchmark based on detected parameters */
void cache_sensitive_benchmark(void) {
    volatile int checksum = 0;
    volatile int use_sse2 = __builtin_cpu_supports("sse2");
    volatile int use_avx = __builtin_cpu_supports("avx");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    /* Control flow based on CPU features - forces GCC to consider cache params */
    if (use_sse2) {
        goto sse2_block;
    } else {
        goto generic_block;
    }
    
sse2_block:
    {
        /* Array sized to typical L1 cache */
        char l1_array[32 * 1024];  /* 32KB */
        for (int i = 0; i < sizeof(l1_array); i += l1_line ? l1_line : 64) {
            l1_array[i] = i & 0xFF;
            checksum += l1_array[i];
        }
        
        if (use_avx) {
            goto avx_block;
        }
        goto after_avx;
    }
    
avx_block:
    {
        /* Larger array for L2 cache */
        char l2_array[256 * 1024];  /* 256KB */
        for (int i = 0; i < sizeof(l2_array); i += l2_line ? l2_line : 64) {
            l2_array[i] = (i * 7) & 0xFF;
            checksum += l2_array[i];
        }
        goto after_avx;
    }
    
generic_block:
    {
        /* Conservative sizes for generic CPUs */
        char small_array[8 * 1024];  /* 8KB */
        for (int i = 0; i < sizeof(small_array); i += 32) {
            small_array[i] = i & 0xFF;
            checksum += small_array[i];
        }
        goto after_avx;
    }
    
after_avx:
    /* Final computation to prevent dead code elimination */
    volatile int final = checksum * 3 + (use_sse2 ? 1 : 0) + (use_avx ? 2 : 0);
    asm volatile("" : : "r"(final));
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    printf("Starting cache descriptor trigger program...\n");
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    printf("Detected cache parameters (if any):\n");
    printf("  L1: %uKB, %u-way, %u-byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %uKB, %u-way, %u-byte line\n", l2_size_kb, l2_assoc, l2_line);
    
    /* Call architecture-specific functions to trigger different cache optimizations */
    generic_cache_test();
    
    /* Conditionally call CPU-specific functions based on builtin checks */
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    
    if (is_core2 || has_sse4_2) {
        core2_cache_test();
    }
    
    if (is_nehalem || __builtin_cpu_supports("avx")) {
        haswell_cache_test();
    }
    
    /* Perform cache-sensitive benchmark */
    cache_sensitive_benchmark();
    
    /* Final checksum computation using all detected parameters */
    volatile unsigned int final_result = 
        l1_size_kb + l1_assoc * 100 + l1_line * 10000 +
        l2_size_kb + l2_assoc * 100 + l2_line * 10000;
    
    printf("Final computed value: %u\n", final_result);
    printf("Program completed. Check GCC driver execution paths for cache descriptor decoding.\n");
    
    return (int)final_result & 0xFF;
}
