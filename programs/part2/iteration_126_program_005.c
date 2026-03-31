/* cache_detector.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
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
void __attribute__((optimize("-O0"))) decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
    );
    
    /* Extract descriptor bytes (eax, ebx, ecx, edx) */
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
    
    /* Process each non-zero, non-0xFF descriptor byte */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || desc == 0xFF) continue;
        
        /* Mirror the switch cases from driver-i386.cc */
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
                /* Skip if xeon_mp (simulated with SSE check) */
                if (!__builtin_cpu_supports("sse")) {
                    l2_size_kb = 4096; l2_assoc = 16; l2_line = 64;
                }
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
void __attribute__((optimize("-mtune=generic"))) benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match various cache sizes */
    char array_8k[8 * 1024];
    char array_256k[256 * 1024];
    
    for (int i = 0; i < sizeof(array_8k); i += 64) {
        array_8k[i] = i & 0xFF;
        sum += array_8k[i];
    }
    
    for (int i = 0; i < sizeof(array_256k); i += 128) {
        array_256k[i] = i & 0xFF;
        sum += array_256k[i];
    }
    
    asm volatile ("" : : "r"(sum) : "memory");
}

void __attribute__((optimize("-mtune=core2"))) benchmark_core2(void) {
    volatile int sum = 0;
    char array_16k[16 * 1024];
    char array_1mb[1024 * 1024];
    
    for (int i = 0; i < sizeof(array_16k); i += 32) {
        array_16k[i] = i & 0xFF;
        sum += array_16k[i];
    }
    
    for (int i = 0; i < sizeof(array_1mb); i += 64) {
        array_1mb[i] = i & 0xFF;
        sum += array_1mb[i];
    }
    
    asm volatile ("" : : "r"(sum) : "memory");
}

void __attribute__((optimize("-mtune=haswell"))) benchmark_haswell(void) {
    volatile int sum = 0;
    char array_32k[32 * 1024];
    char array_2mb[2 * 1024 * 1024];
    
    for (int i = 0; i < sizeof(array_32k); i += 64) {
        array_32k[i] = i & 0xFF;
        sum += array_32k[i];
    }
    
    for (int i = 0; i < sizeof(array_2mb); i += 128) {
        array_2mb[i] = i & 0xFF;
        sum += array_2mb[i];
    }
    
    asm volatile ("" : : "r"(sum) : "memory");
}

void __attribute__((optimize("-mtune=pentium4"))) benchmark_pentium4(void) {
    volatile int sum = 0;
    char array_24k[24 * 1024];
    char array_512k[512 * 1024];
    
    for (int i = 0; i < sizeof(array_24k); i += 64) {
        array_24k[i] = i & 0xFF;
        sum += array_24k[i];
    }
    
    for (int i = 0; i < sizeof(array_512k); i += 64) {
        array_512k[i] = i & 0xFF;
        sum += array_512k[i];
    }
    
    asm volatile ("" : : "r"(sum) : "memory");
}

/* Main function with complex control flow based on CPU features */
int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    int final_sum = 0;
    
    /* Complex if-else chain with goto to create interesting control flow */
    if (has_sse2) {
        benchmark_generic();
        final_sum += 1;
        
        if (has_avx) {
            benchmark_haswell();
            final_sum += 2;
            goto avx_path;
        } else {
            benchmark_core2();
            final_sum += 3;
            goto sse2_only_path;
        }
    } else {
        benchmark_pentium4();
        final_sum += 4;
        goto legacy_path;
    }

avx_path:
    if (has_avx2) {
        /* Additional AVX2-specific benchmark */
        volatile int sum = 0;
        char array_4mb[4 * 1024 * 1024];
        for (int i = 0; i < sizeof(array_4mb); i += 256) {
            array_4mb[i] = i & 0xFF;
            sum += array_4mb[i];
        }
        final_sum += sum & 0xFF;
    }
    /* Fall through */

sse2_only_path:
    /* Execute manual CPUID decoding */
    decode_cpuid_cache_descriptors();
    
    /* Use decoded cache parameters to control array sizes */
    unsigned int target_size = l1_size_kb ? l1_size_kb * 1024 : 8192;
    char* dynamic_array = malloc(target_size);
    if (dynamic_array) {
        for (unsigned int i = 0; i < target_size; i += l1_line ? l1_line : 64) {
            dynamic_array[i] = i & 0xFF;
            final_sum += dynamic_array[i];
        }
        free(dynamic_array);
    }
    
    /* Another level based on L2 cache */
    target_size = l2_size_kb ? (l2_size_kb * 1024) / 4 : 65536;
    dynamic_array = malloc(target_size);
    if (dynamic_array) {
        for (unsigned int i = 0; i < target_size; i += l2_line ? l2_line : 64) {
            dynamic_array[i] = i & 0xFF;
            final_sum += dynamic_array[i];
        }
        free(dynamic_array);
    }
    
    goto finish;

legacy_path:
    /* Legacy path for older CPUs */
    decode_cpuid_cache_descriptors();
    final_sum += l1_assoc + l2_assoc;

finish:
    /* Print results to prevent optimization */
    printf("Cache detection results:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Final checksum: %d\n", final_sum & 0xFF);
    
    return final_sum & 0xFF;
}
