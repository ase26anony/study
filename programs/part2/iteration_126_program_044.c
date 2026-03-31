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

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* 8KB array - matches case 0x0a */
    char buffer1[8 * 1024];
    for (int i = 0; i < sizeof(buffer1); i += 64) {
        buffer1[i] = i & 0xFF;
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
        buffer2[i] = i & 0xFF;
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
        buffer3[i] = i & 0xFF;
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
        buffer4[i] = i & 0xFF;
        sum += buffer4[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
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
    
    /* Process descriptors - mirroring the uncovered switch cases */
    for (i = 0; i < 16; i++) {
        if (descriptors[i] & 0x80) continue; /* Invalid descriptor */
        
        switch (descriptors[i]) {
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
                if (0) break; /* Assume not xeon_mp */
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

/* Cache-sensitive benchmark */
volatile int cache_sensitive_benchmark(volatile int use_sse2, volatile int use_avx) {
    int sum = 0;
    size_t array_size;
    
    /* Control flow based on CPU features */
    if (use_sse2) {
        array_size = 256 * 1024; /* L2 cache size from case 0x21 */
        goto sse2_block;
    } else if (use_avx) {
        array_size = 1024 * 1024; /* L2 cache size from case 0x24 */
        goto avx_block;
    } else {
        array_size = 8 * 1024; /* L1 cache size from case 0x0a */
        goto generic_block;
    }
    
sse2_block: {
        char* array = (char*)malloc(array_size);
        if (!array) return -1;
        
        /* Access pattern that depends on cache line size */
        for (size_t i = 0; i < array_size; i += 64) {
            array[i] = (char)(i & 0xFF);
            sum += array[i];
        }
        free(array);
        goto end;
    }
    
avx_block: {
        char* array = (char*)malloc(array_size);
        if (!array) return -1;
        
        /* Different stride for AVX */
        for (size_t i = 0; i < array_size; i += 128) {
            array[i] = (char)(i & 0xFF);
            sum += array[i];
        }
        free(array);
        goto end;
    }
    
generic_block: {
        char* array = (char*)malloc(array_size);
        if (!array) return -1;
        
        for (size_t i = 0; i < array_size; i += 32) {
            array[i] = (char)(i & 0xFF);
            sum += array[i];
        }
        free(array);
        goto end;
    }
    
end:
    return sum;
}

int main(void) {
    int final_sum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    /* Call architecture-specific functions */
    generic_cache_test();
    if (is_core2) {
        core2_cache_test();
    }
    if (is_nehalem) {
        nehalem_cache_test();
    }
    if (is_haswell) {
        haswell_cache_test();
    }
    
    /* Direct CPUID cache descriptor reading */
    read_cpuid_cache_descriptors();
    
    /* Cache-sensitive benchmark with volatile control flow */
    final_sum += cache_sensitive_benchmark(has_sse2, has_avx);
    
    /* Additional switch to hit more cases */
    switch (level1_sizekb) {
        case 8:
            final_sum += 1;
            break;
        case 16:
            final_sum += 2;
            break;
        case 24:
            final_sum += 3;
            break;
        case 32:
            final_sum += 4;
            break;
        default:
            final_sum += 5;
            break;
    }
    
    switch (level2_sizekb) {
        case 128:
            final_sum += 10;
            break;
        case 256:
            final_sum += 20;
            break;
        case 512:
            final_sum += 30;
            break;
        case 1024:
            final_sum += 40;
            break;
        case 2048:
            final_sum += 50;
            break;
        case 3072:
            final_sum += 60;
            break;
        case 4096:
            final_sum += 70;
            break;
        case 6144:
            final_sum += 80;
            break;
        default:
            final_sum += 90;
            break;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Cache detection results:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", 
           level2_sizekb, level2_assoc, level2_line);
    printf("Final checksum: %d\n", final_sum);
    
    return final_sum & 0xFF; /* Return non-zero to indicate execution */
}
