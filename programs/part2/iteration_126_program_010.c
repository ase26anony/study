/* 
 * cache_descriptor_trigger.c
 * Designed to exercise GCC's CPUID leaf 0x2 cache descriptor decoding
 * Compile with: gcc -O2 -march=native -mtune=native -std=gnu11 cache_descriptor_trigger.c -o cache_test
 * Or for specific architectures: gcc -O3 -march=core2 -mtune=core2 -std=gnu11 cache_descriptor_trigger.c -o cache_test
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* Global variables to store cache parameters (matching driver-i386.cc structure) */
volatile unsigned int l1_size_kb = 0;
volatile unsigned int l1_assoc = 0;
volatile unsigned int l1_line = 0;
volatile unsigned int l2_size_kb = 0;
volatile unsigned int l2_assoc = 0;
volatile unsigned int l2_line = 0;

/* Function to manually decode CPUID leaf 0x2 descriptors */
void __attribute__((noinline)) decode_cpuid_cache_descriptors() {
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
    
    /* Process each valid descriptor byte (non-zero, non-1) */
    for (i = 0; i < 16; i++) {
        uint8_t desc = descriptors[i];
        if (desc == 0 || desc == 1) continue;
        
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
                /* Simulating xeon_mp check */
                if (0) { /* xeon_mp would be determined elsewhere */
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

/* Architecture-specific functions with different tune attributes */
void __attribute__((optimize("-mtune=generic"), noinline)) 
generic_tuned_function(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        data[i] = sum;
    }
}

void __attribute__((optimize("-mtune=core2"), noinline)) 
core2_tuned_function(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 4) {
        sum += data[i];
        data[i] = sum;
    }
}

void __attribute__((optimize("-mtune=haswell"), noinline)) 
haswell_tuned_function(int* data, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i += 8) {
        sum += data[i];
        data[i] = sum;
    }
}

/* Cache-sensitive benchmark functions */
void __attribute__((noinline)) benchmark_l1_cache(int size_kb) {
    int elements = (size_kb * 1024) / sizeof(int);
    int* array = (int*)__builtin_alloca(elements * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < elements; i++) {
        array[i] = i & 0xFF;
    }
    
    /* Access pattern that exercises cache */
    volatile int sum = 0;
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < elements; i += 64) {
            sum += array[i];
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
}

void __attribute__((noinline)) benchmark_l2_cache(int size_kb) {
    int elements = (size_kb * 1024) / sizeof(int);
    int* array = (int*)malloc(elements * sizeof(int));
    
    if (!array) return;
    
    /* Initialize with stride pattern */
    for (int i = 0; i < elements; i++) {
        array[i] = (i * 7) & 0xFF;
    }
    
    /* Large stride to exceed L1 */
    volatile int sum = 0;
    for (int iter = 0; iter < 50; iter++) {
        for (int i = 0; i < elements; i += 128) {
            sum += array[i];
        }
    }
    
    free(array);
    asm volatile ("" : : "r"(sum));
}

int main() {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Volatile flags to force CPU feature checks */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    /* Decode cache descriptors */
    decode_cpuid_cache_descriptors();
    
    /* Control flow based on CPU features */
    int test_array[1024];
    for (int i = 0; i < 1024; i++) {
        test_array[i] = i;
    }
    
    volatile int checksum = 0;
    
    /* Branch to different optimization paths */
    if (has_sse2) {
        generic_tuned_function(test_array, 1024);
        checksum += 1;
        goto sse2_path;
    } else {
        goto legacy_path;
    }
    
sse2_path:
    if (has_avx) {
        haswell_tuned_function(test_array, 1024);
        checksum += 2;
        
        /* Test L1 cache sizes from uncovered cases */
        benchmark_l1_cache(8);   /* 0x0a */
        benchmark_l1_cache(16);  /* 0x0c, 0x0d */
        benchmark_l1_cache(24);  /* 0x0e */
        benchmark_l1_cache(32);  /* 0x2c, 0x68 */
    } else {
        core2_tuned_function(test_array, 1024);
        checksum += 3;
    }
    
    /* Test L2 cache sizes */
    benchmark_l2_cache(128);   /* 0x39, 0x3b, 0x41, 0x79 */
    benchmark_l2_cache(256);   /* 0x21, 0x3c, 0x42, 0x7a, 0x82 */
    benchmark_l2_cache(512);   /* 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    benchmark_l2_cache(1024);  /* 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
    benchmark_l2_cache(2048);  /* 0x45, 0x7d, 0x85 */
    benchmark_l2_cache(3072);  /* 0x48 */
    benchmark_l2_cache(4096);  /* 0x49 */
    benchmark_l2_cache(6144);  /* 0x4e */
    
    goto finish;
    
legacy_path:
    /* Legacy path for older CPUs */
    for (int i = 0; i < 1024; i++) {
        test_array[i] = test_array[i] * 3 + 1;
    }
    checksum += 4;
    
finish:
    /* Compute final checksum using decoded cache parameters */
    checksum += l1_size_kb + l1_assoc + l1_line;
    checksum += l2_size_kb + l2_assoc + l2_line;
    
    /* Print results */
    printf("Cache Parameters Decoded:\n");
    printf("L1: %u KB, %u-way, %u byte line\n", l1_size_kb, l1_assoc, l1_line);
    printf("L2: %u KB, %u-way, %u byte line\n", l2_size_kb, l2_assoc, l2_line);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
