/* cache_detection.c - Trigger GCC's CPUID leaf 0x2 cache descriptor decoding */
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

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    /* 8KB array - matches case 0x0a */
    char array8k[8 * 1024];
    for (int i = 0; i < sizeof(array8k); i += 32) {
        array8k[i] = i & 0xFF;
        sum += array8k[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    /* 32KB array - matches case 0x2c */
    char array32k[32 * 1024];
    for (int i = 0; i < sizeof(array32k); i += 64) {
        array32k[i] = i & 0xFF;
        sum += array32k[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    /* 256KB array - matches case 0x3c */
    char array256k[256 * 1024];
    for (int i = 0; i < sizeof(array256k); i += 64) {
        array256k[i] = i & 0xFF;
        sum += array256k[i];
    }
    (void)sum;
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
        : "a"(2), "c"(0)
    );
    
    /* Extract descriptor bytes from registers */
    descriptors[0] = eax & 0xFF;
    descriptors[1] = (eax >> 8) & 0xFF;
    descriptors[2] = (eax >> 16) & 0xFF;
    descriptors[3] = (eax >> 24) & 0xFF;
    descriptors[4] = ebx & 0xFF;
    descriptors[5] = (ebx >> 8) & 0xFF;
    descriptors[6] = (ebx >> 16) & 0xFF;
    descriptors[7] = (ebx >> 24) & 0xFF;
    descriptors[8] = ecx & 0xFF;
    descriptors[9] = (ecx >> 8) & 0xFF;
    descriptors[10] = (ecx >> 16) & 0xFF;
    descriptors[11] = (ecx >> 24) & 0xFF;
    descriptors[12] = edx & 0xFF;
    descriptors[13] = (edx >> 8) & 0xFF;
    descriptors[14] = (edx >> 16) & 0xFF;
    descriptors[15] = (edx >> 24) & 0xFF;
    
    /* Process descriptor bytes - mirroring uncovered switch cases */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors */
        if (desc == 0 || (desc & 0x80))
            continue;
            
        /* Switch over descriptor values - matches uncovered block */
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

/* Cache-sensitive benchmark based on detected parameters */
void cache_sensitive_benchmark(void) {
    volatile int sum = 0;
    volatile int use_sse2 = __builtin_cpu_supports("sse2");
    volatile int use_avx = __builtin_cpu_supports("avx");
    
    /* Control flow based on CPU features */
    if (use_sse2) {
        goto sse2_path;
    } else if (use_avx) {
        goto avx_path;
    } else {
        goto generic_path;
    }
    
sse2_path:
    {
        /* 16KB array - matches case 0x0c */
        char array16k[16 * 1024];
        for (int i = 0; i < sizeof(array16k); i += 32) {
            array16k[i] = i & 0xFF;
            sum += array16k[i];
        }
    }
    goto benchmark_done;
    
avx_path:
    {
        /* 32KB array - matches case 0x2c */
        char array32k[32 * 1024];
        for (int i = 0; i < sizeof(array32k); i += 64) {
            array32k[i] = i & 0xFF;
            sum += array32k[i];
        }
    }
    goto benchmark_done;
    
generic_path:
    {
        /* 8KB array - matches case 0x0a */
        char array8k[8 * 1024];
        for (int i = 0; i < sizeof(array8k); i += 32) {
            array8k[i] = i & 0xFF;
            sum += array8k[i];
        }
    }
    
benchmark_done:
    /* Use sum to prevent optimization */
    printf("Benchmark checksum: %d\n", sum);
}

int main(void) {
    /* Initialize CPU detection - triggers GCC's internal cache detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force runtime evaluation */
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
    
    /* Direct CPUID reading */
    read_cpuid_cache_descriptors();
    
    printf("Detected L1 Cache: %u KB, %u-way, %u byte line\n",
           l1_size_kb, l1_assoc, l1_line);
    printf("Detected L2 Cache: %u KB, %u-way, %u byte line\n",
           l2_size_kb, l2_assoc, l2_line);
    
    /* Cache-sensitive operations */
    cache_sensitive_benchmark();
    
    /* Additional cache-size-specific tests */
    volatile int checksum = 0;
    
    /* Test different array sizes matching uncovered cache sizes */
    if (has_sse2) {
        /* 256KB test - matches case 0x21 */
        char *array256k = malloc(256 * 1024);
        if (array256k) {
            for (int i = 0; i < 256 * 1024; i += 64) {
                array256k[i] = i & 0xFF;
                checksum += array256k[i];
            }
            free(array256k);
        }
    }
    
    if (has_avx) {
        /* 1024KB test - matches case 0x24 */
        char *array1mb = malloc(1024 * 1024);
        if (array1mb) {
            for (int i = 0; i < 1024 * 1024; i += 64) {
                array1mb[i] = i & 0xFF;
                checksum += array1mb[i];
            }
            free(array1mb);
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
