/* 
 * cache_detection.c
 * Designed to trigger GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_detection.c -o cache_detection
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11 cache_detection.c -o cache_detection
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters matching the uncovered block */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* CPU feature flags - volatile to prevent optimization */
volatile int has_sse2 = 0;
volatile int has_avx = 0;
volatile int has_avx2 = 0;
volatile int is_intel = 0;
volatile int is_amd = 0;

/* Function to manually decode CPUID leaf 0x2 cache descriptors */
__attribute__((noinline, optimize("O0")))
void decode_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptors[16];
    int i, j;
    
    /* Execute CPUID leaf 0x2 to get cache descriptors */
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
    
    /* Process each descriptor byte - mirroring the uncovered switch cases */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        
        /* Skip invalid descriptors (0x00) and TLB/other descriptors */
        if (desc == 0x00 || (desc & 0x80)) continue;
        
        /* Direct mapping of descriptor values to cache parameters */
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
                /* Check for Xeon MP - simplified */
                if (0) {  /* xeon_mp check placeholder */
                    break;
                }
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

/* Architecture-specific functions with different optimization hints */
__attribute__((optimize("O3,-mtune=generic")))
void generic_optimized_loop(int* array, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
        array[i] = sum;
    }
}

__attribute__((optimize("O3,-mtune=core2")))
void core2_optimized_loop(int* array, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 4) {
        sum += array[i];
        array[i] = sum;
    }
}

__attribute__((optimize("O3,-mtune=nehalem")))
void nehalem_optimized_loop(int* array, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 8) {
        sum += array[i];
        array[i] = sum;
    }
}

__attribute__((optimize("O3,-mtune=haswell")))
void haswell_optimized_loop(int* array, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 16) {
        sum += array[i];
        array[i] = sum;
    }
}

/* Cache-sensitive benchmark function */
void cache_sensitive_benchmark(void) {
    /* Array sizes matching cache sizes from uncovered block */
    const int sizes[] = {
        8 * 1024 / sizeof(int),      /* 8KB */
        16 * 1024 / sizeof(int),     /* 16KB */
        32 * 1024 / sizeof(int),     /* 32KB */
        128 * 1024 / sizeof(int),    /* 128KB */
        256 * 1024 / sizeof(int),    /* 256KB */
        512 * 1024 / sizeof(int),    /* 512KB */
        1024 * 1024 / sizeof(int),   /* 1MB */
        2048 * 1024 / sizeof(int),   /* 2MB */
        4096 * 1024 / sizeof(int),   /* 4MB */
        6144 * 1024 / sizeof(int)    /* 6MB */
    };
    
    volatile long long total_sum = 0;
    
    for (int s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        int size = sizes[s];
        int* array = (int*)malloc(size * sizeof(int));
        
        if (!array) continue;
        
        /* Initialize array */
        for (int i = 0; i < size; i++) {
            array[i] = (i * 3) % 256;
        }
        
        /* Execute different loops based on CPU features */
        if (has_sse2) {
            generic_optimized_loop(array, size);
        }
        
        if (has_avx) {
            core2_optimized_loop(array, size);
        }
        
        if (has_avx2) {
            nehalem_optimized_loop(array, size);
        }
        
        if (is_intel) {
            haswell_optimized_loop(array, size);
        }
        
        /* Compute checksum */
        for (int i = 0; i < size; i += 64) {
            total_sum += array[i];
        }
        
        free(array);
    }
    
    /* Use total_sum to prevent dead code elimination */
    printf("Benchmark checksum: %lld\n", total_sum);
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Set CPU feature flags */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    
    /* Check CPU vendor */
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];
    
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );
    
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    
    is_intel = (strcmp(vendor, "GenuineIntel") == 0);
    is_amd = (strcmp(vendor, "AuthenticAMD") == 0);
    
    printf("CPU Vendor: %s\n", vendor);
    printf("SSE2: %s, AVX: %s, AVX2: %s\n",
           has_sse2 ? "yes" : "no",
           has_avx ? "yes" : "no",
           has_avx2 ? "yes" : "no");
    
    /* Decode cache descriptors */
    decode_cpuid_cache_descriptors();
    
    printf("Detected L1 Cache: %u KB, %u-way, %u byte line\n",
           l1_size_kb, l1_assoc, l1_line);
    printf("Detected L2 Cache: %u KB, %u-way, %u byte line\n",
           l2_size_kb, l2_assoc, l2_line);
    
    /* Complex control flow based on CPU features */
    volatile int cpu_type = 0;
    
    if (__builtin_cpu_is("intel")) {
        cpu_type = 1;
        goto intel_path;
    } else if (__builtin_cpu_is("amd")) {
        cpu_type = 2;
        goto amd_path;
    } else {
        cpu_type = 0;
        goto generic_path;
    }

intel_path:
    printf("Intel CPU detected\n");
    if (has_avx2) {
        printf("Using Haswell+ optimizations\n");
    }
    goto benchmark;

amd_path:
    printf("AMD CPU detected\n");
    if (has_avx) {
        printf("Using AMD AVX optimizations\n");
    }
    goto benchmark;

generic_path:
    printf("Generic CPU path\n");
    goto benchmark;

benchmark:
    /* Run cache-sensitive benchmarks */
    cache_sensitive_benchmark();
    
    /* Final checksum computation using all detected parameters */
    volatile unsigned int final_checksum = 0;
    final_checksum += l1_size_kb * l1_assoc + l1_line;
    final_checksum += l2_size_kb * l2_assoc + l2_line;
    final_checksum += has_sse2 * 1000 + has_avx * 2000 + has_avx2 * 3000;
    final_checksum += cpu_type * 10000;
    
    printf("Final checksum: %u\n", final_checksum);
    
    return 0;
}
