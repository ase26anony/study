/* 
 * GCC Cache Descriptor Trigger Program
 * Targets driver-i386.cc cache parameter decoding logic (lines 127-244)
 * Compile with: gcc -O2 -march=native -mtune=native -fno-omit-frame-pointer -std=gnu11 -o cache_detect cache_detect.c
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -flto -std=gnu11 -o cache_detect cache_detect.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to store cache parameters matching driver-i386.cc structure */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Volatile CPU feature flags to force runtime evaluation */
volatile int cpu_sse2 = 0;
volatile int cpu_sse4 = 0;
volatile int cpu_avx = 0;
volatile int cpu_avx2 = 0;
volatile int cpu_xeon_mp = 0; /* Simulated Xeon MP flag */

/* Function with architecture-specific optimization */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int *arr, int size) {
    /* Simple memory access pattern */
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int *arr, int size) {
    /* Different access pattern */
    for (int i = size - 1; i >= 0; i--) {
        arr[i] = arr[i] + 1;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int *arr, int size) {
    /* Strided access */
    for (int i = 0; i < size; i += 4) {
        arr[i] = arr[i] * 3;
    }
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cpuid_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t descriptor;
    int i, j;
    
    /* CPUID leaf 0x2 - Cache Descriptors */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Process descriptor bytes from registers */
    uint8_t *regs = (uint8_t*)&eax;
    
    for (i = 0; i < 4; i++) {
        descriptor = regs[i];
        if (descriptor == 0) continue;
        
        /* Switch statement mirroring driver-i386.cc uncovered block */
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
                if (cpu_xeon_mp)
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
                /* Unknown descriptor */
                break;
        }
    }
    
    /* Check other registers too */
    regs = (uint8_t*)&ebx;
    for (i = 0; i < 4; i++) {
        descriptor = regs[i];
        if (descriptor == 0) continue;
        
        /* Same switch logic would go here in full implementation */
        (void)descriptor; /* Prevent unused warning */
    }
}

/* Cache-size sensitive benchmark */
void cache_sensitive_benchmark(void) {
    volatile int use_small_array = 0;
    volatile int use_medium_array = 0;
    volatile int use_large_array = 0;
    
    /* Control flow based on CPU feature flags */
    if (cpu_sse2) {
        use_small_array = 1;
        goto small_array_test;
    } else if (cpu_sse4) {
        use_medium_array = 1;
        goto medium_array_test;
    } else if (cpu_avx) {
        use_large_array = 1;
        goto large_array_test;
    }
    
small_array_test:
    {
        /* 8KB array - matches case 0x0a L1 size */
        int small_array[2048]; /* 2048 * 4 bytes = 8192 bytes */
        generic_tuned_function(small_array, 2048);
    }
    
    if (!use_medium_array && !use_large_array) goto end_benchmark;
    
medium_array_test:
    {
        /* 256KB array - matches case 0x21 L2 size */
        int medium_array[65536]; /* 65536 * 4 bytes = 262144 bytes */
        core2_tuned_function(medium_array, 65536);
    }
    
    if (!use_large_array) goto end_benchmark;
    
large_array_test:
    {
        /* 1024KB array - matches case 0x24 L2 size */
        int large_array[262144]; /* 262144 * 4 bytes = 1048576 bytes */
        haswell_tuned_function(large_array, 262144);
    }
    
end_benchmark:
    return;
}

/* Additional CPUID-based checks */
void check_cpu_features(void) {
    /* These builtins trigger CPU detection in GCC's driver */
    __builtin_cpu_init();
    
    /* Volatile assignments force runtime evaluation */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    
    /* Simulate Xeon MP detection for case 0x49 */
    cpu_xeon_mp = 0; /* Change to 1 to test the break path in case 0x49 */
}

int main(void) {
    int checksum = 0;
    
    /* Initialize CPU detection - triggers driver-i386.cc cache detection */
    check_cpu_features();
    
    /* Read cache descriptors directly */
    read_cpuid_cache_descriptors();
    
    /* Perform cache-sensitive operations */
    cache_sensitive_benchmark();
    
    /* Call architecture-tuned functions to force mtune processing */
    int test_array[1024];
    generic_tuned_function(test_array, 1024);
    core2_tuned_function(test_array, 1024);
    haswell_tuned_function(test_array, 1024);
    
    /* Compute checksum from detected cache parameters */
    checksum = l1_size_kb + l1_assoc + l1_line +
               l2_size_kb + l2_assoc + l2_line +
               cpu_sse2 + cpu_sse4 + cpu_avx + cpu_avx2;
    
    /* Print results */
    printf("Cache Parameters (from CPUID descriptors):\n");
    printf("  L1: %u KB, %u-way, %u byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("  L2: %u KB, %u-way, %u byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    printf("CPU Features: SSE2=%d, SSE4.2=%d, AVX=%d, AVX2=%d\n",
           cpu_sse2, cpu_sse4, cpu_avx, cpu_avx2);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
