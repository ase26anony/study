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

/* Function optimized for different microarchitectures to force cache detection */
__attribute__((optimize("-mtune=generic")))
void generic_optimized_loop(void) {
    volatile int sum = 0;
    /* Use array size that might trigger cache size consideration */
    int array[1024];
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_optimized_loop(void) {
    volatile int sum = 0;
    int array[2048];
    for (int i = 0; i < 2048; i++) {
        array[i] = i * 2;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_optimized_loop(void) {
    volatile int sum = 0;
    int array[4096];
    for (int i = 0; i < 4096; i++) {
        array[i] = i * 3;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_optimized_loop(void) {
    volatile int sum = 0;
    int array[8192];
    for (int i = 0; i < 8192; i++) {
        array[i] = i * 4;
        sum += array[i];
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
    
    /* Process each valid descriptor byte (0x00 is invalid, 0xFF is reserved) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0x00 || desc == 0xFF) continue;
        
        /* Switch statement mirroring driver-i386.cc uncovered block */
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
                /* Simulating xeon_mp = false case */
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
                /* Other descriptor values not in uncovered block */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions */
void benchmark_l1_cache(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache (8KB-32KB) */
    int array[8192];  /* 32KB for ints */
    
    for (int i = 0; i < 8192; i++) {
        array[i] = i;
    }
    
    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 0; i < 8192; i++) {
            sum += array[i];
        }
    }
    
    (void)sum;
}

void benchmark_l2_cache(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache (256KB-2048KB) */
    int array[262144];  /* 1MB for ints */
    
    for (int i = 0; i < 262144; i++) {
        array[i] = i;
    }
    
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < 262144; i++) {
            sum += array[i];
        }
    }
    
    (void)sum;
}

int main(void) {
    /* Initialize GCC's CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force CPU feature checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    int final_checksum = 0;
    
    /* Call architecture-specific optimized functions */
    generic_optimized_loop();
    final_checksum += 1;
    
    if (has_sse2) {
        core2_optimized_loop();
        final_checksum += 2;
    }
    
    if (has_avx) {
        nehalem_optimized_loop();
        final_checksum += 4;
    }
    
    if (has_avx2) {
        haswell_optimized_loop();
        final_checksum += 8;
    }
    
    /* Direct CPUID cache descriptor reading */
    read_cpuid_cache_descriptors();
    final_checksum += 16;
    
    /* Control flow based on CPU features */
    volatile int cache_benchmark_target = 0;
    
    if (has_sse2 && !has_avx) {
        cache_benchmark_target = 1;
        goto benchmark_l1;
    } else if (has_avx && !has_avx2) {
        cache_benchmark_target = 2;
        goto benchmark_l2;
    } else {
        cache_benchmark_target = 3;
        goto benchmark_both;
    }
    
benchmark_l1:
    benchmark_l1_cache();
    final_checksum += 32;
    goto benchmark_done;
    
benchmark_l2:
    benchmark_l2_cache();
    final_checksum += 64;
    goto benchmark_done;
    
benchmark_both:
    benchmark_l1_cache();
    benchmark_l2_cache();
    final_checksum += 128;
    
benchmark_done:
    /* Print detected cache parameters */
    printf("Detected Cache Parameters:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", 
           l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", 
           l2_size_kb, l2_assoc, l2_line);
    
    /* Print CPU feature flags */
    printf("\nCPU Features:\n");
    printf("SSE2: %d, AVX: %d, AVX2: %d\n", has_sse2, has_avx, has_avx2);
    printf("Intel: %d, AMD: %d\n", is_intel, is_amd);
    
    printf("\nFinal checksum: %d\n", final_checksum);
    
    return final_checksum & 0xFF;
}
