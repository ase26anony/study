/* cache_detection.c - Trigger GCC's CPUID cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables matching the cache structure fields */
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
        buffer[i] = i & 0xFF;
    }
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_cache_test(void) {
    volatile char buffer[32768]; /* 32KB L1 */
    volatile char l2buffer[262144]; /* 256KB L2 - matches case 0x21 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
    for (int i = 0; i < sizeof(l2buffer); i += 64) {
        l2buffer[i] = i & 0xFF;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile char buffer[32768]; /* 32KB L1 */
    volatile char l2buffer[262144]; /* 256KB L2 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
    for (int i = 0; i < sizeof(l2buffer); i += 64) {
        l2buffer[i] = i & 0xFF;
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
    
    /* Process each valid descriptor byte (non-zero, non-1) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || desc == 1) continue;
        
        /* Mirror the switch statement from driver-i386.cc */
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
                if (0) /* Placeholder for xeon_mp detection */
                    break;
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
                /* Unknown descriptor */
                break;
        }
    }
}

/* Cache-sensitive benchmark function */
unsigned long long cache_sensitive_benchmark(int cache_level) {
    const size_t sizes[] = {
        8 * 1024,    /* L1 small */
        16 * 1024,   /* L1 medium */
        32 * 1024,   /* L1 large */
        64 * 1024,   /* L1 very large */
        128 * 1024,  /* L2 small */
        256 * 1024,  /* L2 medium */
        512 * 1024,  /* L2 large */
        1024 * 1024, /* L2 very large */
        2048 * 1024, /* L3 small */
        4096 * 1024  /* L3 large */
    };
    
    volatile unsigned long long sum = 0;
    size_t selected_size = sizes[cache_level % 10];
    
    /* Dynamically allocate array based on detected cache size */
    char *buffer = (char*)malloc(selected_size);
    if (!buffer) return 0;
    
    /* Initialize with pattern */
    for (size_t i = 0; i < selected_size; i++) {
        buffer[i] = (i * 13) & 0xFF;
    }
    
    /* Access pattern that stresses cache */
    for (int iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < selected_size; i += 64) {
            sum += buffer[i];
        }
    }
    
    free(buffer);
    return sum;
}

int main(void) {
    unsigned long long checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set volatile flags based on CPU features */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Check CPU vendor */
    {
        uint32_t eax, ebx, ecx, edx;
        char vendor[13];
        
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(0)
        );
        
        *(uint32_t*)(vendor) = ebx;
        *(uint32_t*)(vendor + 4) = edx;
        *(uint32_t*)(vendor + 8) = ecx;
        vendor[12] = '\0';
        
        is_intel = (strcmp(vendor, "GenuineIntel") == 0);
        is_amd = (strcmp(vendor, "AuthenticAMD") == 0);
    }
    
    /* Complex control flow based on CPU features */
    volatile int cpu_flags = 0;
    cpu_flags |= has_sse2 ? 1 : 0;
    cpu_flags |= has_avx ? 2 : 0;
    cpu_flags |= has_avx2 ? 4 : 0;
    cpu_flags |= is_intel ? 8 : 0;
    cpu_flags |= is_amd ? 16 : 0;
    
    /* Force evaluation of all architecture-specific functions */
    if (cpu_flags & 1) { /* SSE2 available */
        generic_cache_test();
        checksum += 1;
    }
    
    if (cpu_flags & 8) { /* Intel CPU */
        core2_cache_test();
        nehalem_cache_test();
        haswell_cache_test();
        checksum += 2;
    }
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    checksum += level1_sizekb + level2_sizekb;
    
    /* Cache-sensitive benchmarks based on detected parameters */
    benchmark_start:
    if (level1_sizekb > 0) {
        checksum += cache_sensitive_benchmark(level1_sizekb / 8);
    }
    
    if (level2_sizekb > 0) {
        checksum += cache_sensitive_benchmark(level2_sizekb / 128 + 4);
    }
    
    /* Alternative path based on CPU vendor */
    if (is_intel) {
        /* Intel-specific cache patterns */
        volatile int array_8k[2048];  /* 8KB */
        volatile int array_32k[8192]; /* 32KB */
        
        for (int i = 0; i < 2048; i += 8) {
            array_8k[i] = i;
        }
        for (int i = 0; i < 8192; i += 8) {
            array_32k[i] = i;
        }
        
        checksum += array_8k[0] + array_32k[0];
    } else if (is_amd) {
        /* AMD-specific cache patterns */
        volatile int array_16k[4096];  /* 16KB */
        volatile int array_512k[131072]; /* 512KB */
        
        for (int i = 0; i < 4096; i += 8) {
            array_16k[i] = i;
        }
        for (int i = 0; i < 131072; i += 8) {
            array_512k[i] = i;
        }
        
        checksum += array_16k[0] + array_512k[0];
    }
    
    /* Prevent dead code elimination */
    volatile unsigned long long final_result = checksum;
    
    printf("Cache detection test completed. Checksum: %llu\n", final_result);
    printf("Detected L1: %uKB, %u-way, %uB line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("Detected L2: %uKB, %u-way, %uB line\n", 
           level2_sizekb, level2_assoc, level2_line);
    
    return (int)(final_result & 0x7FFFFFFF);
}
