/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters matching uncovered block */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Function with architecture-specific optimization hints */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* 8KB array - matches case 0x0a */
    char buffer1[8 * 1024];
    for (int i = 0; i < sizeof(buffer1); i += 64) {
        sum += buffer1[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* 32KB array - matches case 0x2c */
    char buffer2[32 * 1024];
    for (int i = 0; i < sizeof(buffer2); i += 64) {
        sum += buffer2[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_cache_test(void) {
    volatile int sum = 0;
    /* 256KB array - matches case 0x21 */
    char buffer3[256 * 1024];
    for (int i = 0; i < sizeof(buffer3); i += 64) {
        sum += buffer3[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* 1024KB array - matches case 0x24 */
    char buffer4[1024 * 1024];
    for (int i = 0; i < sizeof(buffer4); i += 64) {
        sum += buffer4[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
    );
    
    /* Process descriptor bytes from all registers */
    uint32_t regs[4] = {eax, ebx, ecx, edx};
    
    for (i = 0; i < 4; i++) {
        uint8_t *bytes = (uint8_t *)&regs[i];
        for (j = 0; j < 4; j++) {
            descriptor = bytes[j];
            
            /* Skip invalid descriptors */
            if (descriptor == 0x00 || descriptor == 0x01) continue;
            
            /* Mirror the exact switch cases from uncovered block */
            switch (descriptor) {
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
                    /* xeon_mp check omitted for simplicity */
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
}

/* Cache-sensitive benchmark function */
void cache_sensitive_benchmark(volatile int use_sse, volatile int use_avx) {
    unsigned int target_size;
    
    /* Control flow based on CPU feature flags */
    if (use_sse && !use_avx) {
        target_size = 256 * 1024;  /* L2 size for many SSE-era CPUs */
        goto benchmark_block1;
    } else if (use_avx) {
        target_size = 1024 * 1024; /* L2 size for AVX-era CPUs */
        goto benchmark_block2;
    } else {
        target_size = 8 * 1024;    /* Small L1 size */
        goto benchmark_block3;
    }

benchmark_block1: {
    char *buffer = malloc(target_size);
    volatile int sum = 0;
    for (int i = 0; i < target_size; i += 64) {
        sum += buffer[i];
    }
    free(buffer);
    return;
}

benchmark_block2: {
    char *buffer = malloc(target_size);
    volatile int sum = 0;
    /* Different access pattern */
    for (int i = 0; i < target_size; i += 128) {
        sum += buffer[i];
    }
    free(buffer);
    return;
}

benchmark_block3: {
    char *buffer = malloc(target_size);
    volatile int sum = 0;
    for (int i = 0; i < target_size; i += 32) {
        sum += buffer[i];
    }
    free(buffer);
    return;
}
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Call architecture-specific functions */
    generic_cache_test();
    checksum += 1;
    
    if (has_sse2) {
        core2_cache_test();
        checksum += 2;
    }
    
    if (has_avx) {
        nehalem_cache_test();
        checksum += 4;
    }
    
    if (has_avx2) {
        haswell_cache_test();
        checksum += 8;
    }
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    checksum += l1_size_kb + l2_size_kb;
    
    /* Perform cache-sensitive benchmarks */
    cache_sensitive_benchmark(has_sse2, has_avx);
    
    /* Additional CPU model checks */
    if (__builtin_cpu_is("pentium4")) {
        /* Trigger case 0x2c potentially */
        volatile int p4_test = 0;
        char buffer[32 * 1024];  /* 32KB L1 */
        for (int i = 0; i < sizeof(buffer); i += 64) {
            p4_test += buffer[i];
        }
        checksum += p4_test;
    }
    
    if (__builtin_cpu_is("core2")) {
        /* Trigger cases 0x21, 0x24, etc. */
        volatile int core2_test = 0;
        char buffer[256 * 1024];  /* 256KB L2 */
        for (int i = 0; i < sizeof(buffer); i += 64) {
            core2_test += buffer[i];
        }
        checksum += core2_test;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Cache detection results:\n");
    printf("  L1: %uKB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %uKB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    printf("  Feature flags: SSE2=%d, AVX=%d, AVX2=%d\n",
           has_sse2, has_avx, has_avx2);
    printf("  Final checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate success */
}
