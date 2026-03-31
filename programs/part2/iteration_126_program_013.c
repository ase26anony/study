/* cache_detection.c - Trigger GCC driver-i386.cc cache descriptor decoding */
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

/* Function optimized for different microarchitectures */
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

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile char buffer[32768]; /* 32KB L1 */
    volatile char l2_buffer[262144]; /* 256KB L2 - matches case 0x21 */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
    for (int i = 0; i < sizeof(l2_buffer); i += 64) {
        l2_buffer[i] = i & 0xFF;
    }
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
        : "a"(0x2)
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
    
    /* Process descriptors (mirroring driver-i386.cc switch) */
    for (int i = 0; i < descriptor_count; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0xFF) continue;
        
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

/* Cache-sensitive benchmark */
unsigned long long cache_sensitive_benchmark(volatile int use_sse, volatile int use_avx) {
    unsigned long long checksum = 0;
    
    /* Allocate arrays matching cache sizes from uncovered block */
    size_t sizes[] = {8*1024, 16*1024, 32*1024, 64*1024, 128*1024, 
                      256*1024, 512*1024, 1024*1024, 2048*1024};
    
    for (size_t s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        size_t array_size = sizes[s] / sizeof(int);
        int *array = (int*)malloc(array_size * sizeof(int));
        
        if (!array) continue;
        
        /* Initialize array */
        for (size_t i = 0; i < array_size; i++) {
            array[i] = (i * 3) & 0xFF;
        }
        
        /* Access pattern sensitive to cache size */
        for (size_t i = 0; i < array_size; i += 64/sizeof(int)) {
            checksum += array[i];
        }
        
        free(array);
    }
    
    /* Conditional jumps based on CPU features */
    if (use_sse) {
        goto sse_block;
    } else if (use_avx) {
        goto avx_block;
    } else {
        goto generic_block;
    }
    
sse_block:
    checksum ^= 0x12345678;
    goto end_block;
    
avx_block:
    checksum ^= 0x87654321;
    goto end_block;
    
generic_block:
    checksum ^= 0xABCDEF01;
    
end_block:
    return checksum;
}

int main(void) {
    /* Initialize CPU detection (triggers driver cache detection) */
    __builtin_cpu_init();
    
    /* Volatile flags to force CPU feature checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", 
           has_sse2, has_avx, has_avx2);
    printf("CPU Model: core2=%d, nehalem=%d, haswell=%d\n",
           is_core2, is_nehalem, is_haswell);
    
    /* Call architecture-specific functions */
    generic_cache_test();
    
    if (is_core2) {
        core2_cache_test();
    }
    
    if (is_haswell) {
        haswell_cache_test();
    }
    
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    printf("Detected Cache: L1=%dKB/%d-way/%dB, L2=%dKB/%d-way/%dB\n",
           l1_size_kb, l1_assoc, l1_line,
           l2_size_kb, l2_assoc, l2_line);
    
    /* Run cache-sensitive benchmark */
    unsigned long long checksum = cache_sensitive_benchmark(has_sse2, has_avx);
    
    /* Additional CPUID-based branching */
    uint32_t eax, ebx, ecx, edx;
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    volatile uint32_t cpuid_family = (eax >> 8) & 0xF;
    volatile uint32_t cpuid_model = (eax >> 4) & 0xF;
    
    /* Switch on CPU family/model (triggers different optimization paths) */
    switch (cpuid_family) {
        case 0x6: /* Intel */
            switch (cpuid_model) {
                case 0xF: /* Pentium 4 */
                    checksum += 0xF00D;
                    break;
                case 0x17: /* Core 2 */
                    checksum += 0xC0DE;
                    break;
                case 0x2A: /* Sandy Bridge */
                    checksum += 0x5ANDY;
                    break;
                case 0x3C: /* Haswell */
                    checksum += 0xHA5W;
                    break;
            }
            break;
        case 0xF: /* AMD */
            checksum += 0xAMD;
            break;
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
