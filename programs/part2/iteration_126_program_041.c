/* cache_detection.c - Trigger GCC driver-i386.cc cache parameter decoding */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
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

/* Architecture-specific functions with different tune attributes */
__attribute__((optimize("-mtune=generic")))
void generic_tuned_function(int *arr, int size) {
    /* Access pattern that depends on cache size */
    for (int i = 0; i < size; i += 16) {
        arr[i] = i * 2;
    }
}

__attribute__((optimize("-mtune=core2")))
void core2_tuned_function(int *arr, int size) {
    /* Different stride for Core2 */
    for (int i = 0; i < size; i += 32) {
        arr[i] = i * 3;
    }
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_tuned_function(int *arr, int size) {
    /* Nehalem-optimized pattern */
    for (int i = 0; i < size; i += 64) {
        arr[i] = i * 4;
    }
}

__attribute__((optimize("-mtune=haswell")))
void haswell_tuned_function(int *arr, int size) {
    /* Haswell-optimized pattern */
    for (int i = 0; i < size; i += 128) {
        arr[i] = i * 5;
    }
}

/* Direct CPUID leaf 0x2 reading - mirrors driver-i386.cc logic */
void read_cache_descriptors(void) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t *descriptors;
    int i, j;
    
    /* Execute CPUID leaf 0x2 */
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2)
    );
    
    /* Extract descriptor bytes from registers */
    uint8_t desc_bytes[16];
    desc_bytes[0] = (eax >> 0) & 0xFF;
    desc_bytes[1] = (eax >> 8) & 0xFF;
    desc_bytes[2] = (eax >> 16) & 0xFF;
    desc_bytes[3] = (eax >> 24) & 0xFF;
    desc_bytes[4] = (ebx >> 0) & 0xFF;
    desc_bytes[5] = (ebx >> 8) & 0xFF;
    desc_bytes[6] = (ebx >> 16) & 0xFF;
    desc_bytes[7] = (ebx >> 24) & 0xFF;
    desc_bytes[8] = (ecx >> 0) & 0xFF;
    desc_bytes[9] = (ecx >> 8) & 0xFF;
    desc_bytes[10] = (ecx >> 16) & 0xFF;
    desc_bytes[11] = (ecx >> 24) & 0xFF;
    desc_bytes[12] = (edx >> 0) & 0xFF;
    desc_bytes[13] = (edx >> 8) & 0xFF;
    desc_bytes[14] = (edx >> 16) & 0xFF;
    desc_bytes[15] = (edx >> 24) & 0xFF;
    
    /* Process descriptors - mirroring driver-i386.cc switch logic */
    for (i = 0; i < 16; i++) {
        uint8_t desc = desc_bytes[i];
        
        /* Skip invalid descriptors */
        if (desc == 0x00 || desc == 0xFF) continue;
        
        /* Map descriptor to cache parameters */
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
                /* Simulating xeon_mp = false */
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
                /* Unknown descriptor - continue */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions */
void benchmark_l1_cache(void) {
    /* Use sizes matching L1 cache cases */
    int sizes[] = {8, 16, 24, 32}; /* KB */
    long checksum = 0;
    
    for (int s = 0; s < 4; s++) {
        int elements = (sizes[s] * 1024) / sizeof(int);
        int *array = (int*)malloc(elements * sizeof(int));
        
        if (!array) continue;
        
        /* Fill array */
        for (int i = 0; i < elements; i++) {
            array[i] = i;
        }
        
        /* Access with different strides */
        for (int i = 0; i < elements; i += 16) {
            checksum += array[i];
        }
        
        free(array);
    }
    
    /* Use checksum to prevent optimization */
    asm volatile ("" : : "r"(checksum));
}

void benchmark_l2_cache(void) {
    /* Use sizes matching L2 cache cases */
    int sizes[] = {128, 256, 512, 1024, 2048, 3072, 4096, 6144}; /* KB */
    long checksum = 0;
    
    for (int s = 0; s < 8; s++) {
        int elements = (sizes[s] * 1024) / sizeof(int);
        int *array = (int*)malloc(elements * sizeof(int));
        
        if (!array) continue;
        
        /* Fill array */
        for (int i = 0; i < elements; i++) {
            array[i] = i * 2;
        }
        
        /* Access pattern */
        for (int i = 0; i < elements; i += 64) {
            checksum += array[i];
        }
        
        free(array);
    }
    
    /* Use checksum to prevent optimization */
    asm volatile ("" : : "r"(checksum));
}

int main(void) {
    /* Initialize CPU detection (triggers driver cache detection) */
    __builtin_cpu_init();
    
    /* Set volatile CPU feature flags */
    cpu_sse2 = __builtin_cpu_supports("sse2");
    cpu_sse4 = __builtin_cpu_supports("sse4.2");
    cpu_avx = __builtin_cpu_supports("avx");
    cpu_avx2 = __builtin_cpu_supports("avx2");
    
    long total_checksum = 0;
    
    /* Read cache descriptors directly */
    read_cache_descriptors();
    
    /* Allocate arrays of different sizes matching cache parameters */
    int *array_small = (int*)malloc(8 * 1024);      /* 8KB */
    int *array_medium = (int*)malloc(256 * 1024);   /* 256KB */
    int *array_large = (int*)malloc(1024 * 1024);   /* 1MB */
    
    if (array_small && array_medium && array_large) {
        /* Control flow based on volatile CPU flags */
        if (cpu_sse2) {
            generic_tuned_function(array_small, (8 * 1024) / sizeof(int));
            total_checksum += 1;
            goto benchmark_block1;
        } else {
            goto benchmark_block2;
        }
        
benchmark_block1:
        if (cpu_sse4) {
            core2_tuned_function(array_medium, (256 * 1024) / sizeof(int));
            total_checksum += 2;
            goto benchmark_block3;
        }
        
benchmark_block2:
        if (cpu_avx) {
            nehalem_tuned_function(array_large, (1024 * 1024) / sizeof(int));
            total_checksum += 3;
        }
        
benchmark_block3:
        if (cpu_avx2) {
            haswell_tuned_function(array_small, (8 * 1024) / sizeof(int));
            total_checksum += 4;
        }
        
        /* Use arrays to prevent optimization */
        for (int i = 0; i < 100; i++) {
            array_small[i % 2048] = i;
            array_medium[i % 65536] = i * 2;
            array_large[i % 262144] = i * 3;
            total_checksum += array_small[i % 2048] + 
                             array_medium[i % 65536] + 
                             array_large[i % 262144];
        }
    }
    
    /* Run cache benchmarks */
    benchmark_l1_cache();
    benchmark_l2_cache();
    
    /* Print results to prevent dead code elimination */
    printf("Cache Parameters: L1=%uKB, L2=%uKB\n", l1_size_kb, l2_size_kb);
    printf("Checksum: %ld\n", total_checksum);
    
    /* Cleanup */
    if (array_small) free(array_small);
    if (array_medium) free(array_medium);
    if (array_large) free(array_large);
    
    return 0;
}
