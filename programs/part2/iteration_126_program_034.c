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
    
    /* Process each valid descriptor byte (non-zero, bit 31 clear) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || (desc & 0x80)) continue;
        
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
                /* Skip if Xeon MP - we don't have that flag */
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
                /* Other descriptors we don't specifically map */
                break;
        }
    }
}

/* Architecture-specific functions with different mtune optimizations */
__attribute__((optimize("-mtune=generic")))
void benchmark_generic(void) {
    volatile int sum = 0;
    /* Use array sizes that match common cache sizes */
    char array8k[8 * 1024];      /* 8KB - matches case 0x0a */
    char array16k[16 * 1024];    /* 16KB - matches case 0x0c/0x0d */
    char array256k[256 * 1024];  /* 256KB - matches case 0x21 */
    
    /* Access patterns that depend on cache parameters */
    for (int i = 0; i < sizeof(array8k); i += 32)  /* 32-byte line */
        sum += array8k[i];
    for (int i = 0; i < sizeof(array16k); i += 64) /* 64-byte line */
        sum += array16k[i];
    for (int i = 0; i < sizeof(array256k); i += 64)
        sum += array256k[i];
    
    /* Prevent optimization */
    asm volatile ("" : "+r" (sum));
}

__attribute__((optimize("-mtune=core2")))
void benchmark_core2(void) {
    volatile int sum = 0;
    /* Different array sizes for Core2 cache topology */
    char array32k[32 * 1024];    /* 32KB L1 - matches case 0x2c */
    char array2m[2 * 1024 * 1024]; /* 2MB L2 - matches case 0x7d */
    
    for (int i = 0; i < sizeof(array32k); i += 64)
        sum += array32k[i];
    for (int i = 0; i < sizeof(array2m); i += 64)
        sum += array2m[i];
    
    asm volatile ("" : "+r" (sum));
}

__attribute__((optimize("-mtune=haswell")))
void benchmark_haswell(void) {
    volatile int sum = 0;
    /* Haswell typically has 32KB L1, 256KB L2 */
    char array32k[32 * 1024];    /* 32KB */
    char array256k[256 * 1024];  /* 256KB */
    char array8m[8 * 1024 * 1024]; /* 8MB L3 (not in switch but triggers detection) */
    
    for (int i = 0; i < sizeof(array32k); i += 64)
        sum += array32k[i];
    for (int i = 0; i < sizeof(array256k); i += 64)
        sum += array256k[i];
    for (int i = 0; i < sizeof(array8m); i += 64)
        sum += array8m[i];
    
    asm volatile ("" : "+r" (sum));
}

__attribute__((optimize("-mtune=pentium4")))
void benchmark_pentium4(void) {
    volatile int sum = 0;
    /* Pentium 4 cache characteristics */
    char array8k[8 * 1024];      /* 8KB L1 - matches case 0x0a */
    char array512k[512 * 1024];  /* 512KB L2 - matches several cases */
    
    for (int i = 0; i < sizeof(array8k); i += 64)
        sum += array8k[i];
    for (int i = 0; i < sizeof(array512k); i += 64)
        sum += array512k[i];
    
    asm volatile ("" : "+r" (sum));
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
    clock_t start, end;
    
    /* Decode CPUID cache descriptors directly */
    printf("Decoding CPUID leaf 0x2 cache descriptors...\n");
    decode_cpuid_cache_descriptors();
    
    printf("Detected cache parameters:\n");
    printf("  L1: %u KB, %u-way, %u-byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u-byte line\n", l2_size_kb, l2_assoc, l2_line);
    
    /* Complex control flow with goto based on CPU features */
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
        char array1m[1024 * 1024];
        for (int i = 0; i < sizeof(array1m); i += 64)
            sum += array1m[i];
        final_sum += sum & 0xFF;
    }
    goto merge_point;

sse2_only_path:
    /* SSE2-only optimizations */
    {
        volatile int sum = 0;
        char array128k[128 * 1024];  /* 128KB - matches case 0x39/0x3b */
        for (int i = 0; i < sizeof(array128k); i += 32)
            sum += array128k[i];
        final_sum += sum & 0xFF;
    }
    goto merge_point;

legacy_path:
    /* Legacy CPU optimizations */
    {
        volatile int sum = 0;
        char array64k[64 * 1024];
        for (int i = 0; i < sizeof(array64k); i += 32)
            sum += array64k[i];
        final_sum += sum & 0xFF;
    }

merge_point:
    /* Final benchmark using detected cache sizes */
    start = clock();
    
    /* Allocate arrays based on detected cache parameters */
    size_t l1_size = l1_size_kb ? l1_size_kb * 1024 : 32 * 1024;
    size_t l2_size = l2_size_kb ? l2_size_kb * 1024 : 256 * 1024;
    
    char *l1_array = malloc(l1_size);
    char *l2_array = malloc(l2_size);
    
    if (l1_array && l2_array) {
        /* Initialize with pattern */
        for (size_t i = 0; i < l1_size; i++)
            l1_array[i] = (i % 256);
        for (size_t i = 0; i < l2_size; i++)
            l2_array[i] = (i % 256);
        
        /* Cache-sensitive traversal */
        volatile int sum = 0;
        unsigned int line_size = l1_line ? l1_line : 64;
        
        /* L1-sized working set */
        for (int iter = 0; iter < 100; iter++) {
            for (size_t i = 0; i < l1_size; i += line_size)
                sum += l1_array[i];
        }
        
        /* L2-sized working set */
        for (int iter = 0; iter < 10; iter++) {
            for (size_t i = 0; i < l2_size; i += line_size)
                sum += l2_array[i];
        }
        
        final_sum += sum & 0xFFFF;
        
        free(l1_array);
        free(l2_array);
    }
    
    end = clock();
    
    printf("Benchmark completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Final checksum: %d\n", final_sum);
    
    return final_sum & 0xFF;
}
