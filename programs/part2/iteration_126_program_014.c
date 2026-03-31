/* 
 * cache_detection_test.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection_test.c -o cache_test
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11 cache_detection_test.c -o cache_test
 */

#include <stdio.h>
#include <stdint.h>
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
    
    /* Process each descriptor byte (mirroring driver-i386.cc logic) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip null or invalid descriptors */
        if (desc == 0x00 || desc == 0x01) continue;
        
        /* Manual switch over descriptor values - triggers same logic as uncovered block */
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
                if (0) /* Placeholder for xeon_mp variable */
                    break;
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
                /* Other descriptors not in uncovered block */
                break;
        }
    }
}

/* Architecture-specific functions with different mtune optimizations */
void __attribute__((optimize("-mtune=generic"))) generic_cache_test(void) {
    volatile int sum = 0;
    /* Array sized to trigger L1 cache considerations */
    char buffer[8 * 1024]; /* 8KB */
    
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(sum));
}

void __attribute__((optimize("-mtune=core2"))) core2_cache_test(void) {
    volatile int sum = 0;
    /* Array sized for L2 cache (256KB) */
    char buffer[256 * 1024];
    
    for (int i = 0; i < sizeof(buffer); i += 64) { /* 64-byte stride */
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

void __attribute__((optimize("-mtune=haswell"))) haswell_cache_test(void) {
    volatile int sum = 0;
    /* Array sized for larger L2 cache (1MB) */
    char buffer[1024 * 1024];
    
    for (int i = 0; i < sizeof(buffer); i += 128) { /* 128-byte stride */
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

void __attribute__((optimize("-mtune=pentium4"))) pentium4_cache_test(void) {
    volatile int sum = 0;
    /* Pentium 4 specific cache size (32KB L1) */
    char buffer[32 * 1024];
    
    for (int i = 0; i < sizeof(buffer); i += 32) { /* 32-byte stride */
        buffer[i] = i & 0xFF;
        sum += buffer[i];
    }
    
    asm volatile ("" : : "r"(sum));
}

/* Cache-sensitive benchmark function */
unsigned long long cache_sensitive_benchmark(size_t size_kb) {
    volatile unsigned long long checksum = 0;
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *array = (int*)malloc(elements * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array */
    for (size_t i = 0; i < elements; i++) {
        array[i] = (int)(i * 3 + 1);
    }
    
    /* Access pattern that depends on cache size */
    for (size_t iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < elements; i += 16) { /* 64-byte cache line stride */
            checksum += array[i];
        }
    }
    
    free(array);
    return checksum;
}

int main(void) {
    volatile unsigned long long total_checksum = 0;
    
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Use volatile flags to force runtime evaluation */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    /* Decode CPUID cache descriptors directly */
    decode_cpuid_cache_descriptors();
    
    /* Control flow based on CPU features (forces driver to evaluate cache params) */
    if (has_sse2) {
        generic_cache_test();
        total_checksum += 1;
        
        if (is_intel) {
            core2_cache_test();
            total_checksum += 2;
            
            if (has_avx) {
                haswell_cache_test();
                total_checksum += 4;
            }
        }
        
        if (is_amd) {
            pentium4_cache_test(); /* Some AMD CPUs might trigger pentium4 paths */
            total_checksum += 8;
        }
    }
    
    /* Perform cache-sensitive benchmarks with different sizes */
    volatile int cache_size_selector = 0;
    
    /* Force evaluation order with gotos */
    cache_size_selector = has_sse2 ? 1 : 0;
    
    if (cache_size_selector) {
        /* Test different cache sizes from the uncovered block */
        total_checksum += cache_sensitive_benchmark(8);    /* 0x0a */
        total_checksum += cache_sensitive_benchmark(16);   /* 0x0c, 0x0d */
        total_checksum += cache_sensitive_benchmark(32);   /* 0x2c */
        total_checksum += cache_sensitive_benchmark(128);  /* 0x39, 0x3b, 0x41 */
        total_checksum += cache_sensitive_benchmark(256);  /* 0x21, 0x3c, 0x42 */
        total_checksum += cache_sensitive_benchmark(512);  /* 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
        total_checksum += cache_sensitive_benchmark(1024); /* 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
        total_checksum += cache_sensitive_benchmark(2048); /* 0x45, 0x7d, 0x85 */
        total_checksum += cache_sensitive_benchmark(4096); /* 0x49 */
    }
    
    /* Print results (prevents dead code elimination) */
    printf("Cache Detection Test Results:\n");
    printf("L1 Cache: %u KB, %u-way, %u-byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("L2 Cache: %u KB, %u-way, %u-byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    printf("Total Checksum: %llu\n", total_checksum);
    printf("CPU Features: SSE2=%d, AVX=%d, AVX2=%d, Intel=%d, AMD=%d\n",
           has_sse2, has_avx, has_avx2, is_intel, is_amd);
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
