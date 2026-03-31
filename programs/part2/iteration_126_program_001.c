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
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(2), "c"(0));
    
    /* Extract descriptor bytes (eax bit 31 indicates if registers contain descriptors) */
    if ((eax & 0x80000000) == 0) {
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
        
        /* Process descriptors (mirroring driver-i386.cc logic) */
        for (i = 0; i < 16; i++) {
            uint8_t desc = descriptors[i];
            if (desc == 0x00 || desc == 0xFF) continue;
            
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
}

/* Architecture-specific functions with different -mtune optimizations */
__attribute__((optimize("-mtune=generic"), noinline))
void benchmark_generic(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache */
    int array[2048]; /* ~8KB */
    for (int i = 0; i < 2048; i++) {
        array[i] = i;
        sum += array[i];
    }
    asm volatile ("" : : "r"(sum) : "memory");
}

__attribute__((optimize("-mtune=core2"), noinline))
void benchmark_core2(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache for Core 2 */
    int array[262144]; /* ~1MB */
    for (int i = 0; i < 262144; i++) {
        array[i] = i;
        sum += array[i];
    }
    asm volatile ("" : : "r"(sum) : "memory");
}

__attribute__((optimize("-mtune=haswell"), noinline))
void benchmark_haswell(void) {
    volatile int sum = 0;
    /* Array sized to larger L2 cache */
    int array[524288]; /* ~2MB */
    for (int i = 0; i < 524288; i++) {
        array[i] = i;
        sum += array[i];
    }
    asm volatile ("" : : "r"(sum) : "memory");
}

__attribute__((optimize("-mtune=pentium4"), noinline))
void benchmark_pentium4(void) {
    volatile int sum = 0;
    /* Pentium 4 had smaller L1 cache */
    int array[1024]; /* ~4KB */
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    asm volatile ("" : : "r"(sum) : "memory");
}

/* Cache-sensitive memory access pattern */
__attribute__((noinline))
unsigned long cache_sensitive_access(unsigned int size_kb) {
    unsigned int elements = (size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    volatile unsigned long sum = 0;
    
    if (!buffer) return 0;
    
    /* Initialize with pattern */
    for (unsigned int i = 0; i < elements; i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Access with stride to test cache effects */
    for (unsigned int i = 0; i < elements; i += 64) {
        sum += buffer[i];
    }
    
    free(buffer);
    return sum;
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to force evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    unsigned long total_checksum = 0;
    
    /* Control flow based on CPU features */
    if (has_sse2) {
        benchmark_generic();
        total_checksum += 1;
    }
    
    if (has_avx) {
        benchmark_haswell();
        total_checksum += 2;
    }
    
    if (has_avx2) {
        benchmark_core2();  /* Some AVX2 CPUs might be Core2-like */
        total_checksum += 4;
    }
    
    if (is_intel) {
        benchmark_pentium4();
        total_checksum += 8;
    }
    
    /* Decode CPUID cache descriptors directly */
    decode_cpuid_cache_descriptors();
    
    /* Perform cache-sensitive benchmarks based on detected parameters */
    if (l1_size_kb > 0) {
        total_checksum += cache_sensitive_access(l1_size_kb);
    }
    
    if (l2_size_kb > 0) {
        total_checksum += cache_sensitive_access(l2_size_kb);
    }
    
    /* Fallback to common cache sizes if detection failed */
    if (l1_size_kb == 0) {
        total_checksum += cache_sensitive_access(32);  /* Common L1 */
    }
    if (l2_size_kb == 0) {
        total_checksum += cache_sensitive_access(256); /* Common L2 */
    }
    
    /* Complex control flow with goto to prevent optimization */
    volatile int selector = has_sse2 + has_avx * 2 + has_avx2 * 4;
    
    switch (selector) {
        case 0: goto no_features;
        case 1: goto sse2_only;
        case 3: goto sse2_avx;
        case 7: goto all_features;
        default: goto mixed_features;
    }
    
sse2_only:
    total_checksum += 0x1000;
    goto end_switch;
    
sse2_avx:
    total_checksum += 0x2000;
    goto end_switch;
    
all_features:
    total_checksum += 0x3000;
    goto end_switch;
    
mixed_features:
    total_checksum += 0x4000;
    goto end_switch;
    
no_features:
    total_checksum += 0x5000;
    goto end_switch;
    
end_switch:
    
    /* Print results to prevent dead code elimination */
    printf("Cache Detection Results:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d\n", has_sse2, has_avx, has_avx2);
    printf("Total checksum: %lu\n", total_checksum);
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
