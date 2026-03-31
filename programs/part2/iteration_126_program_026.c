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

/* Function optimized for different microarchitectures */
__attribute__((optimize("-mtune=generic")))
void generic_cache_test(void) {
    volatile int sum = 0;
    int array[1024]; /* 4KB typical L1 */
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=core2")))
void core2_cache_test(void) {
    volatile int sum = 0;
    int array[32768]; /* 128KB typical L2 */
    for (int i = 0; i < 32768; i++) {
        array[i] = i;
        sum += array[i];
    }
    (void)sum;
}

__attribute__((optimize("-mtune=haswell")))
void haswell_cache_test(void) {
    volatile int sum = 0;
    int array[262144]; /* 1MB typical L3 */
    for (int i = 0; i < 262144; i++) {
        array[i] = i;
        sum += array[i];
    }
    (void)sum;
}

/* Direct CPUID leaf 0x2 reading */
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
    
    /* Extract descriptor bytes */
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
    
    /* Process each valid descriptor byte (0x01-0xFF) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || desc == 0xFF) continue;
        
        /* Mirror the uncovered switch logic */
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
                /* Simulate xeon_mp = false */
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

/* Cache-sensitive benchmark */
void cache_sensitive_benchmark(volatile int use_sse2, volatile int use_avx) {
    int i, j;
    volatile long long sum = 0;
    
    /* Array sizes matching uncovered cache sizes */
    int sizes[] = {2048, 8192, 32768, 131072, 524288, 2097152};
    const char* size_names[] = {"8KB", "32KB", "128KB", "512KB", "2MB", "8MB"};
    
    for (j = 0; j < 6; j++) {
        int* array = (int*)malloc(sizes[j] * sizeof(int));
        if (!array) continue;
        
        /* Initialize */
        for (i = 0; i < sizes[j]; i++) {
            array[i] = i & 0xFF;
        }
        
        /* Access pattern */
        clock_t start = clock();
        for (i = 0; i < sizes[j]; i += 64) { /* 64-byte stride */
            sum += array[i];
        }
        clock_t end = clock();
        
        double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Size %s: %f sec, sum=%lld\n", 
               size_names[j], time_used, sum);
        
        free(array);
        
        /* Control flow based on CPU features */
        if (use_sse2 && j == 1) {
            goto sse2_block;
        }
        if (use_avx && j == 3) {
            goto avx_block;
        }
    }
    
    return;
    
sse2_block:
    /* SSE2-specific path */
    asm volatile ("" : : : "memory");
    return;
    
avx_block:
    /* AVX-specific path */
    asm volatile ("" : : : "memory");
    return;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initialize CPU detection (triggers driver cache detection) */
    __builtin_cpu_init();
    
    /* Volatile flags to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    
    printf("CPU Features: SSE2=%d, AVX=%d\n", has_sse2, has_avx);
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
    
    printf("Detected L1: %uKB, %u-way, %uB line\n", 
           level1_sizekb, level1_assoc, level1_line);
    printf("Detected L2: %uKB, %u-way, %uB line\n", 
           level2_sizekb, level2_assoc, level2_line);
    
    /* Cache-sensitive benchmark with control flow */
    cache_sensitive_benchmark(has_sse2, has_avx);
    
    /* Complex if-else chain based on CPU features */
    if (has_sse2) {
        int* sse2_array = (int*)malloc(8192 * sizeof(int)); /* 32KB */
        for (int i = 0; i < 8192; i++) {
            sse2_array[i] = i;
            checksum += sse2_array[i];
        }
        free(sse2_array);
        goto next_block;
    } else if (has_avx) {
        int* avx_array = (int*)malloc(32768 * sizeof(int)); /* 128KB */
        for (int i = 0; i < 32768; i++) {
            avx_array[i] = i;
            checksum += avx_array[i];
        }
        free(avx_array);
        goto next_block;
    } else {
        int* generic_array = (int*)malloc(2048 * sizeof(int)); /* 8KB */
        for (int i = 0; i < 2048; i++) {
            generic_array[i] = i;
            checksum += generic_array[i];
        }
        free(generic_array);
    }
    
next_block:
    /* Additional architecture checks */
    if (__builtin_cpu_is("pentium4")) {
        checksum += 0x2c; /* Descriptor for P4 L1 cache */
    }
    if (__builtin_cpu_is("athlon")) {
        checksum += 0x0a; /* Descriptor for Athlon L1 cache */
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
