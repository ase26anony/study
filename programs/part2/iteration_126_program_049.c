/* cache_detector.c - Trigger GCC driver-i386.cc cache descriptor decoding */
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

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache */
    volatile char array[8 * 1024];
    for (int i = 0; i < sizeof(array); i += 64) {
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache for Core 2 */
    volatile char array[256 * 1024];
    for (int i = 0; i < sizeof(array); i += 64) {
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache for Nehalem */
    volatile char array[256 * 1024];
    for (int i = 0; i < sizeof(array); i += 64) {
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to typical L3 cache */
    volatile char array[8192 * 1024];
    for (int i = 0; i < sizeof(array); i += 64) {
        sum += array[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading (mirrors driver logic) */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int descriptor_count = 0;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
    );
    
    /* Extract descriptor bytes (Intel CPUID leaf 0x2 format) */
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
    
    /* Process descriptors (mirroring driver-i386.cc switch logic) */
    for (int i = 0; i < descriptor_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip null and reserved descriptors */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Direct mapping of uncovered switch cases */
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
                /* Note: xeon_mp check omitted for simplicity */
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
                /* Other descriptors not in uncovered lines */
                break;
        }
    }
}

/* Cache-sensitive benchmark */
volatile int perform_cache_benchmark(unsigned int cache_size_kb) {
    volatile int result = 0;
    size_t array_size = cache_size_kb * 1024;
    
    /* Dynamically allocate array of appropriate size */
    volatile char* array = (volatile char*)malloc(array_size);
    if (!array) return 0;
    
    /* Initialize with pattern */
    for (size_t i = 0; i < array_size; i++) {
        array[i] = (i % 256);
    }
    
    /* Access with stride to test cache effects */
    for (size_t i = 0; i < array_size; i += 64) {
        result += array[i];
    }
    
    free((void*)array);
    return result;
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force CPU feature checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    int checksum = 0;
    
    /* Force compilation with different -mtune values */
    generic_cache_test();
    checksum += 1;
    
    core2_cache_test();
    checksum += 2;
    
    nehalem_cache_test();
    checksum += 3;
    
    haswell_cache_test();
    checksum += 4;
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Control flow based on CPU features (forces driver to consider cache params) */
    volatile int cpu_flags = 0;
    if (has_sse2) cpu_flags |= 1;
    if (has_avx) cpu_flags |= 2;
    if (has_avx2) cpu_flags |= 4;
    
    /* Goto-based control flow to create complex branching */
    if (cpu_flags & 1) {
        goto sse2_path;
    } else {
        goto legacy_path;
    }
    
sse2_path:
    checksum += perform_cache_benchmark(256);  /* Typical L2 size */
    if (cpu_flags & 2) {
        goto avx_path;
    }
    goto done;
    
avx_path:
    checksum += perform_cache_benchmark(1024); /* Larger cache */
    if (cpu_flags & 4) {
        checksum += perform_cache_benchmark(8192); /* L3 cache */
    }
    goto done;
    
legacy_path:
    checksum += perform_cache_benchmark(8);    /* Small L1 */
    goto done;
    
done:
    /* Use detected cache parameters */
    checksum += l1_size_kb + l1_assoc + l1_line;
    checksum += l2_size_kb + l2_assoc + l2_line;
    
    /* Print results to prevent optimization */
    printf("Cache detection results:\n");
    printf("L1: %u KB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
