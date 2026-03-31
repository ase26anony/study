/* cache_detection.c - Trigger GCC's CPUID cache descriptor decoding */
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

/* Architecture-specific functions with different tune attributes */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    int array[1024]; /* Small array */
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    int array[8192]; /* 32KB array */
    for (int i = 0; i < 8192; i++) {
        array[i] = i * 2;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=nehalem")))
void nehalem_cache_test(void) {
    volatile int sum = 0;
    int array[16384]; /* 64KB array */
    for (int i = 0; i < 16384; i++) {
        array[i] = i * 3;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    int array[32768]; /* 128KB array */
    for (int i = 0; i < 32768; i++) {
        array[i] = i * 4;
        sum += array[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading with inline assembly */
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
    
    /* Process each non-zero, non-1 descriptor byte */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || desc == 1) continue;
        
        /* Mirror the uncovered switch statement */
        switch (desc) {
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
                /* Simulating xeon_mp = false */
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
                /* Unknown descriptor */
                break;
        }
    }
}

/* Cache-sensitive benchmark functions */
void benchmark_l1_cache(void) {
    volatile int sum = 0;
    /* Array sized to typical L1 cache (8KB-32KB) */
    int array[8192]; /* 32KB for ints */
    
    for (int i = 0; i < 8192; i++) {
        array[i] = i;
    }
    
    /* Sequential access pattern */
    for (int i = 0; i < 8192; i++) {
        sum += array[i];
    }
    
    /* Random access pattern */
    for (int i = 0; i < 10000; i++) {
        sum += array[i % 8192];
    }
    
    (void)sum;
}

void benchmark_l2_cache(void) {
    volatile int sum = 0;
    /* Array sized to typical L2 cache (256KB-2MB) */
    int array[262144]; /* 1MB for ints */
    
    for (int i = 0; i < 262144; i++) {
        array[i] = i;
    }
    
    /* Strided access pattern */
    for (int i = 0; i < 262144; i += 64) {
        sum += array[i];
    }
    
    (void)sum;
}

int main(void) {
    volatile int checksum = 0;
    volatile int use_sse2 = 0;
    volatile int use_avx = 0;
    volatile int use_avx2 = 0;
    
    /* Initialize CPU detection (triggers GCC's internal cache detection) */
    __builtin_cpu_init();
    
    /* Set volatile flags based on CPU features */
    use_sse2 = __builtin_cpu_supports("sse2");
    use_avx = __builtin_cpu_supports("avx");
    use_avx2 = __builtin_cpu_supports("avx2");
    
    /* Call architecture-specific functions */
    generic_cache_test();
    checksum += 1;
    
    if (use_sse2) {
        core2_cache_test();
        checksum += 2;
        goto benchmark_block1;
    } else {
        goto benchmark_block2;
    }
    
benchmark_block1:
    if (use_avx) {
        nehalem_cache_test();
        checksum += 4;
        goto benchmark_block3;
    }
    
benchmark_block2:
    haswell_cache_test();
    checksum += 8;
    
benchmark_block3:
    /* Read CPUID cache descriptors directly */
    read_cpuid_cache_descriptors();
    checksum += 16;
    
    /* Perform cache-sensitive benchmarks based on detected features */
    if (use_sse2) {
        benchmark_l1_cache();
        checksum += 32;
    }
    
    if (use_avx) {
        benchmark_l2_cache();
        checksum += 64;
    }
    
    /* Additional CPU model checks */
    if (__builtin_cpu_is("intel")) {
        checksum += 128;
    }
    
    if (__builtin_cpu_is("amd")) {
        checksum += 256;
    }
    
    /* Create arrays sized to specific cache thresholds */
    volatile int* small_array = malloc(2048 * sizeof(int));  /* ~8KB */
    volatile int* medium_array = malloc(65536 * sizeof(int)); /* ~256KB */
    volatile int* large_array = malloc(262144 * sizeof(int)); /* ~1MB */
    
    if (small_array && medium_array && large_array) {
        /* Access patterns that depend on cache parameters */
        for (int i = 0; i < 2048; i++) {
            small_array[i] = i;
            checksum += small_array[i];
        }
        
        for (int i = 0; i < 65536; i += 16) {
            medium_array[i] = i;
            checksum += medium_array[i];
        }
        
        for (int i = 0; i < 262144; i += 64) {
            large_array[i] = i;
            checksum += large_array[i];
        }
    }
    
    free((void*)small_array);
    free((void*)medium_array);
    free((void*)large_array);
    
    /* Print results to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("L1 Cache: %uKB, %u-way, %uB line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("L2 Cache: %uKB, %u-way, %uB line\n", 
           level2_sizekb, level2_assoc, level2_line);
    
    return checksum & 0xFF;
}
